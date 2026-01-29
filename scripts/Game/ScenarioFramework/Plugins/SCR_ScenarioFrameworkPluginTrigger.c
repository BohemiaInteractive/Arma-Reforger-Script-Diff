[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkPluginTrigger : SCR_ScenarioFrameworkPlugin
{
	[Attribute(defvalue: "5.0", params: "0 inf", desc: "Radius of the trigger if selected", category: "Trigger")]
	float m_fAreaRadius;

	[Attribute("0", UIWidgets.ComboBox, "By whom the trigger is activated", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkTriggerActivation), category: "Trigger Activation")]
	SCR_EScenarioFrameworkTriggerActivation m_eActivationPresence;
	
	[Attribute(desc: "Fill the entity names here for detection. Is combined with other filters using OR.", category: "Trigger")]
	ref array<string> m_aSpecificEntityNames;

	[Attribute(desc: "If SPECIFIC_CLASS is selected, fill the class name here.", category: "Trigger")]
	ref array<string> m_aSpecificClassNames;

	[Attribute(desc: "Which Prefabs and if their children will be detected by the trigger. Is combined with other filters using OR.", category: "Trigger")]
	ref array<ref SCR_ScenarioFrameworkPrefabFilter> m_aPrefabFilter;

	[Attribute("", category: "Trigger Activation")]
	FactionKey m_sActivatedByThisFaction;

	[Attribute(desc: "Here you can input custom trigger conditions that you can create by extending the SCR_CustomTriggerConditions", uiwidget: UIWidgets.Object)]
	ref array<ref SCR_ScenarioFrameworkActivationConditionBase> m_aCustomTriggerConditions;
	
	[Attribute(defvalue: SCR_EScenarioFrameworkLogicOperators.AND.ToString(), UIWidgets.ComboBox, "Which Boolean Logic will be used for Custom Trigger Conditions.", "", enums: SCR_EScenarioFrameworkLogicOperatorHelper.GetParamInfo(), category: "Trigger")]
	SCR_EScenarioFrameworkLogicOperators m_eCustomTriggerConditionLogic;
	
	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "If you set some vehicle to be detected by the trigger, it will also search the inventory for vehicle prefabs/classes that are set", category: "Trigger")]
	bool m_bSearchVehicleInventory;

	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "Trigger")]
	bool m_bOnce;
	
	[Attribute(defvalue: "0", desc: "Activate the trigger once it is empty", category: "Trigger")]
	bool m_bActivateOnEmpty;

	[Attribute(defvalue: "1", UIWidgets.Slider, desc: "How frequently is the trigger updated and performing calculations. Lower numbers will decrease performance.", params: "0 86400 1", category: "Trigger")]
	float m_fUpdateRate;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Minimum players needed to activate this trigger when PLAYER Activation presence is selected", params: "0 1 0.01", precision: 2, category: "Trigger")]
	float m_fMinimumPlayersNeededPercentage;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "For how long the trigger conditions must be true in order for the trigger to activate. If conditions become false, timer resets", params: "0 86400 1", category: "Trigger")]
	float m_fActivationCountdownTimer;

	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Whether or not the notification is allowed to be displayed", category: "Trigger")]
	bool m_bNotificationEnabled;

	[Attribute(desc: "Notification title text that will be displayed when the PLAYER Activation presence is selected", category: "Trigger")]
	string m_sPlayerActivationNotificationTitle;

	[Attribute(defvalue: "0", UIWidgets.CheckBox, desc: "Whether or not the audio sound is played and affected by the trigger", category: "Trigger")]
	bool m_bEnableAudio;

	[Attribute(desc: "Audio sound that will be playing when countdown is active.", category: "Trigger")]
	string m_sCountdownAudio;
	
	[Attribute(desc: "Actions that will be activated when entity that went through the filter entered the trigger and is inside (Be carefull as Framework Triggers activate this periodically if you don't disable the Once attribute)", category: "Trigger")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aEntityEnteredActions;
	
	[Attribute(desc: "Actions that will be activated when entity that went through the filter left the trigger", category: "Trigger")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aEntityLeftActions;
	
	[Attribute(desc: "Actions that will be activated when all conditions are met and Trigger finishes", category: "Trigger")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aFinishedActions;

	//------------------------------------------------------------------------------------------------
	//! Initializes scenario framework trigger entity with provided parameters.
	//! \param[in] object of layer base from which entity is further retrieved
	override void Init(SCR_ScenarioFrameworkLayerBase object)
	{
		if (!object)
			return;

		super.Init(object);
		SCR_ScenarioFrameworkTriggerEntity trigger;
		IEntity entity = object.GetSpawnedEntity();

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(object);
		if (area)
		{
			trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(area.GetTrigger());
		}
		else
		{
			if (!BaseGameTriggerEntity.Cast(entity))
			{
				Print("ScenarioFramework: SlotTrigger - The selected prefab is not trigger!", LogLevel.ERROR);
				return;
			}
			trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(entity);
		}
		
		// Resolve Alias
		SCR_FactionAliasComponent factionAliasComponent = SCR_FactionAliasComponent.Cast(GetGame().GetFactionManager().FindComponent(SCR_FactionAliasComponent));
		if (factionAliasComponent) 
			m_sActivatedByThisFaction = factionAliasComponent.ResolveFactionAlias(m_sActivatedByThisFaction);

		if (trigger)
		{
			trigger.SetSphereRadius(m_fAreaRadius);
			trigger.SetActivationPresence(m_eActivationPresence);
			trigger.SetOwnerFaction(m_sActivatedByThisFaction);
			trigger.SetSpecificClassName(m_aSpecificClassNames);
			trigger.AddSpecificEntityNameFilter(m_aSpecificEntityNames);
			trigger.SetPrefabFilters(m_aPrefabFilter);
			trigger.SetCustomTriggerConditions(m_aCustomTriggerConditions);
			trigger.SetCustomTriggerConditionLogic(m_eCustomTriggerConditionLogic);
			trigger.SetSearchVehicleInventory(m_bSearchVehicleInventory);
			trigger.SetOnce(m_bOnce);
			trigger.SetActivateOnEmpty(m_bActivateOnEmpty);
			trigger.SetUpdateRate(m_fUpdateRate);
			trigger.SetNotificationEnabled(m_bNotificationEnabled);
			trigger.SetEnableAudio(m_bEnableAudio);
			trigger.SetMinimumPlayersNeeded(m_fMinimumPlayersNeededPercentage);
			trigger.SetPlayerActivationNotificationTitle(m_sPlayerActivationNotificationTitle);
			trigger.SetActivationCountdownTimer(m_fActivationCountdownTimer);
			trigger.SetCountdownAudio(m_sCountdownAudio);
			trigger.SetEntityEnteredActions(m_aEntityEnteredActions);
			trigger.SetEntityLefActions(m_aEntityLeftActions);
			trigger.SetFinishedActions(m_aFinishedActions);

			return;
		}

		SCR_BaseFactionTriggerEntity factionTrigger = SCR_BaseFactionTriggerEntity.Cast(entity);
		if (factionTrigger)
		{
			factionTrigger.SetSphereRadius(m_fAreaRadius);
			FactionManager factionManager = GetGame().GetFactionManager();
			if (factionManager)
				factionTrigger.AddOwnerFaction(m_sActivatedByThisFaction);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] object of layer base from which entity is further retrieved
	override void OnWBKeyChanged(SCR_ScenarioFrameworkLayerBase object)
	{
		super.OnWBKeyChanged(object);
		object.SetDebugShapeSize(m_fAreaRadius);
		//src.Set("m_sAreaName", m_fAreaRadius);
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetActions()
	{
		array<ref SCR_ScenarioFrameworkActionBase>	combinedActions = {};
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aEntityEnteredActions)
	    {
	        combinedActions.Insert(action);
   		}
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aEntityLeftActions)
	    {
	        combinedActions.Insert(action);
   		}
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aFinishedActions)
	    {
	        combinedActions.Insert(action);
   		}
		
		return combinedActions;
	}
}