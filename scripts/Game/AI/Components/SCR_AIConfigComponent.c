[ComponentEditorProps(category: "GameScripted/AI", description: "Component for utility AI system calculations")]
class SCR_AIConfigComponentClass : ScriptComponentClass
{
}

class SCR_AIConfigComponent : ScriptComponent
{
	// @TODO: Implement into the rest of the AI
	[Attribute( defvalue: "0.5", uiwidget: UIWidgets.Slider, desc: "Unit skill", params: "0 1 0.01" )]
	float m_Skill;
	
	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Alow movement" )]
	bool m_EnableMovement;

	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow reacting on danger events" )]
	bool m_EnableDangerEvents;

	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow reacting on perceived targets" )]
	bool m_EnablePerception;

	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow shooting and attacking in general" )]
	bool m_EnableAttack;

	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow finding and taking cover" )]
	bool m_EnableTakeCover;

	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow aiming and gestures in general" )]
	bool m_EnableLooking;

	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow sending AI messages" )]
	bool m_EnableCommunication;

	[Attribute( defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Allow leader to stop when formation is deformed" )]
	bool m_EnableLeaderStop;

	[Attribute( defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allow artificial aiming error for AI." )]
	bool m_EnableAimingError;
	
	[Attribute("", UIWidgets.Object)]
	ref array<ref SCR_AIReactionBase> m_aDefaultReactions;

	[Attribute("", UIWidgets.Object)]
	ref array<ref SCR_AIDangerReaction> m_aDangerReactions;	
	ref map<EAIDangerEventType, ref SCR_AIDangerReaction> m_mDangerReactions = new map<EAIDangerEventType, ref SCR_AIDangerReaction>();
	
	[Attribute("", UIWidgets.Object)]
	ref array<ref SCR_AIGoalReaction> m_aGoalReactions;	

	ref array<ref SCR_AIGoalReaction> m_aGoalReactionsPacked = {};
	
	[Attribute("", UIWidgets.Object)]
	ref array<ref SCR_AIInfoReaction> m_aInfoReactions;	

	ref array<ref SCR_AIInfoReaction> m_aInfoReactionsPacked = {};
	
	[Attribute("", UIWidgets.Object)]
    ref SCR_AITargetReactionBase m_Reaction_UnknownTarget;
	
	[Attribute("", UIWidgets.Object)]
	ref SCR_AITargetReactionBase m_Reaction_RetreatFromTarget;
	
	[Attribute("", UIWidgets.Object)]
	ref SCR_AITargetReaction_SelectedTargetChangedBase m_Reaction_SelectedTargetChanged;
	
	[Attribute("", UIWidgets.Auto, "Specifies which behavior tree is used for specific weapon type", category: "Weapon Handling")]
	ref array<ref SCR_AIWeaponTypeSelectionConfig> m_aWeaponTypeSelectionConfig;
	
	[Attribute("", UIWidgets.Object, "Handling configs for weapon types", category: "Weapon Handling")]
	ref array<ref SCR_AIWeaponTypeHandlingConfig> m_aWeaponTypeHandlingConfig;
	ref map<EWeaponType, ref SCR_AIWeaponTypeHandlingConfig> m_mWeaponTypeHandlingConfig = new map<EWeaponType, ref SCR_AIWeaponTypeHandlingConfig>();
	ref array<int> m_aMinSuppressiveMagCountSpec = {}; // This array is passed to AIWeaponTargetSelector
	
	[Attribute("", UIWidgets.Object, "Default weapon handling config", category: "Weapon Handling")]
	ref SCR_AIWeaponTypeHandlingConfig m_DefaultWeaponTypeHandlingConfig;
	
	[Attribute("{3EA0ED1A7C3B8FE5}AI/BehaviorTrees/Chimera/Soldier/HandleWeapon_Default.bt", UIWidgets.Auto, "Fallback BT when nothing found in WeaponTypeSelectionConfig", category: "Weapon Handling")]
	ResourceName m_sDefaultWeaponBehaviorTree;
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SCR_AISettingsComponent settings = SCR_AISettingsComponent.GetInstance();
		if (!settings)
			return;

		// If the settings component is enabled, overwrite current settings by global ones
		m_EnableMovement = settings.m_EnableMovement;
		m_EnableDangerEvents = settings.m_EnableDangerEvents;
		m_EnablePerception = settings.m_EnablePerception;
		m_EnableAttack = settings.m_EnableAttack;
		m_EnableTakeCover = settings.m_EnableTakeCover;
		m_EnableLooking = settings.m_EnableLooking;
		m_EnableCommunication = settings.m_EnableCommunication;
		m_EnableLeaderStop = settings.m_EnableLeaderStop;
		m_EnableAimingError = settings.m_EnableAimError;
		
		typename type_EMessageType_Goal = EMessageType_Goal;
		typename type_EMessageType_Info = EMessageType_Info;
		
		// Map weapon handling configs based on weapon type
		// Initialize min magazine specification array
		m_aMinSuppressiveMagCountSpec.Clear();
		m_aMinSuppressiveMagCountSpec.Insert(SCR_AIWeaponTypeHandlingConfig.DEFAULT_LOW_MAG_THRESHOLD);
		if (m_DefaultWeaponTypeHandlingConfig)
			m_aMinSuppressiveMagCountSpec[0] = m_DefaultWeaponTypeHandlingConfig.m_iMinSuppressiveMagCountThreshold;
		foreach (SCR_AIWeaponTypeHandlingConfig config : m_aWeaponTypeHandlingConfig)
		{
			m_mWeaponTypeHandlingConfig[config.m_eWeaponType] = config;
			
			m_aMinSuppressiveMagCountSpec.Insert(config.m_eWeaponType);
			m_aMinSuppressiveMagCountSpec.Insert(config.m_iMinSuppressiveMagCountThreshold);
		}
			
		foreach (SCR_AIDangerReaction reaction : m_aDangerReactions)
			m_mDangerReactions[reaction.m_eType] = reaction;
		
		m_aGoalReactionsPacked.Resize(type_EMessageType_Goal.GetVariableCount());
		foreach (SCR_AIGoalReaction reaction : m_aGoalReactions)
		{
			if (reaction.m_eType != EMessageType_Goal.NONE)
				m_aGoalReactionsPacked[reaction.m_eType] = reaction;
		}
		
		m_aInfoReactionsPacked.Resize(type_EMessageType_Info.GetVariableCount());
		foreach (SCR_AIInfoReaction reaction : m_aInfoReactions)
		{
			if (reaction.m_eType != EMessageType_Info.NONE)
				m_aInfoReactionsPacked[reaction.m_eType] = reaction;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AIWeaponTypeHandlingConfig GetWeaponTypeHandlingConfig(EWeaponType weaponType)
	{
		SCR_AIWeaponTypeHandlingConfig config = m_mWeaponTypeHandlingConfig[weaponType];
		if (!config)
			return m_DefaultWeaponTypeHandlingConfig;
			
		return config;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] utility
	//! \param[in] message
	void PerformGoalReaction(SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessageGoal goalMessage = SCR_AIMessageGoal.Cast(message);
		if (!goalMessage)
		{
			Debug.Error("Message mismatch");
			return;
		}
		SCR_AIGoalReaction reaction = m_aGoalReactionsPacked[goalMessage.m_MessageType];
		if (reaction)
		{
			reaction.PerformReaction(utility, message);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] utility
	//! \param[in] message
	void PerformGoalReaction(SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessageGoal goalMessage = SCR_AIMessageGoal.Cast(message);
		if (!goalMessage)
		{
			Debug.Error("Message mismatch");
			return;
		}
		SCR_AIGoalReaction reaction = m_aGoalReactionsPacked[goalMessage.m_MessageType];
		if (reaction)
		{
			reaction.PerformReaction(utility, message);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] utility
	//! \param[in] message
	void PerformInfoReaction(SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessageInfo infoMessage = SCR_AIMessageInfo.Cast(message);
		if (!infoMessage)
		{
			Debug.Error("Message mismatch");
			return;
		}
		SCR_AIInfoReaction reaction = m_aInfoReactionsPacked[infoMessage.m_MessageType];
		if (reaction)
		{
			reaction.PerformReaction(utility, message);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] utility
	//! \param[in] message
	void PerformInfoReaction(SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessageInfo infoMessage = SCR_AIMessageInfo.Cast(message);
		if (!infoMessage)
		{
			Debug.Error("Message mismatch");
			return;
		}
		SCR_AIInfoReaction reaction = m_aInfoReactionsPacked[infoMessage.m_MessageType];
		if (reaction)
		{
			reaction.PerformReaction(utility, message);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] utility
	//! \param[in] dangerEvent
	//! \return
	bool PerformDangerReaction(SCR_AIUtilityComponent utility, AIDangerEvent dangerEvent, int dangerEventCount)
	{
		SCR_AIDangerReaction reaction = m_mDangerReactions[dangerEvent.GetDangerType()];
		if (reaction)
		{
			return reaction.PerformReaction(utility, utility.m_ThreatSystem, dangerEvent, dangerEventCount);
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] utility
	void AddDefaultBehaviors(SCR_AIUtilityComponent utility)
	{
		foreach (SCR_AIReactionBase reaction : m_aDefaultReactions)
		{
			reaction.PerformReaction(utility); // @TODO: This has to work for all reactions, now it would work only on default ones
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] utility
	void AddDefaultActivities(SCR_AIGroupUtilityComponent utility)
	{
		foreach (SCR_AIReactionBase reaction : m_aDefaultReactions)
		{
			reaction.PerformReaction(utility); // @TODO: This has to work for all reactions, now it would work only on default ones
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns resource name set for specific WeaponType and MuzzleType as sub-filter
	ResourceName GetTreeNameForWeaponType(EWeaponType weaponType, EMuzzleType muzzleType)
	{
		foreach (SCR_AIWeaponTypeSelectionConfig configItem : m_aWeaponTypeSelectionConfig)
		{
			if (configItem.m_eWeaponType == EWeaponType.WT_NONE || configItem.m_eWeaponType == weaponType) // NONE will work as ANY, first filter = weapon type
			{
				if (configItem.m_eMuzzleType == muzzleType) // second filter = muzzle type
				{
					return configItem.m_sBehaviorTree;
				}
			}
		}
		return m_sDefaultWeaponBehaviorTree;
	}
}
