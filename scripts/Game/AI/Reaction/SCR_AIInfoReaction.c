//------------------------------------------------------------------------------------------------
// INFO REACTION BASE
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIInfoReaction
{
	// Don't use, it's here for backwards compatibility, not guaranteed to work in all classes
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Don't use, it's here for backwards compatibility, not guaranteed to work in all classes", params: "bt")]
	string m_OverrideBehaviorTree;
	
	[Attribute("0", UIWidgets.ComboBox, "Type of event activating the reaction", "", ParamEnumArray.FromEnum(EMessageType_Info) )]
	EMessageType_Info m_eType;
	
	void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message) {}
	void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message) {}
};


//------------------------------------------------------------------------------------------------
// INFO REACTIONS - Reactions on different info inputs from children (groups or agents)
//------------------------------------------------------------------------------------------------


// Scheduled for deletion
[BaseContainerProps()]
class SCR_AIInfoReaction_NoAmmo : SCR_AIInfoReaction
{
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_NoAmmo.Cast(message);
		if(!msg)
			return;
		
		auto activity = new SCR_AIResupplyActivity(utility, null, msg.m_entityToSupply, msg.m_MagazineWell, priorityLevel: utility.GetCurrentAction().EvaluatePriorityLevel());
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIInfoReaction_UnderFire : SCR_AIInfoReaction
{
	// TODO: retreat to last safe position
};

[BaseContainerProps()]
class SCR_AIInfoReaction_Wounded : SCR_AIInfoReaction
{
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Wounded.Cast(message);
		if(!msg)
			return;
		AIAgent aiAgent = msg.GetSender();
		AIGroup aiGroup = utility.m_Owner;
		SCR_AIActionBase currentAction = SCR_AIActionBase.Cast(utility.GetCurrentAction());
		if (!currentAction || !aiAgent)
			return;
		
		float priorityLevelClamped = currentAction.GetRestrictedPriorityLevel();
		
		IEntity woundedEntity = msg.m_WoundedEntity;
		
		// Ignore message if we already have an activity to heal this soldier
		array<ref AIActionBase> actions = {};
		utility.GetActions(actions);
		foreach (AIActionBase action : actions)
		{
			SCR_AIHealActivity healActivity = SCR_AIHealActivity.Cast(action);
			
			if (!healActivity)
				continue;
			
			// Return if we are already healing this soldier
			if (healActivity.m_EntityToHeal.m_Value == woundedEntity)
			{
				healActivity.SetPriorityLevel(priorityLevelClamped);
				return;
			}
		}
		
		auto activity = new SCR_AIHealActivity(utility, null, woundedEntity, priorityLevel: priorityLevelClamped);
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIInfoReaction_FoundCorpse : SCR_AIInfoReaction
{
	// TODO: planning move to corpse location?
};