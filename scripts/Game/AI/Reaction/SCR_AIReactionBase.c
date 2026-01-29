//------------------------------------------------------------------------------------------------
// BASIC REACTIONS
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIReactionBase
{
	// TODO: Behavior overriding will break the behavior, it's forbidden for now
	//[Attribute(defvalue: "", uiwidget: UIWidgets.stringPicker, desc: "BT assigned to behavior", params: "bt")]
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "(TEMP) Relavive path to a behavior tree with forward slashes \nExample: 'AI/BehaviorTrees/Chimera/Soldier/Wait.bt'", params: "bt")]
	string m_OverrideBehaviorTree;
	
	void PerformReaction(notnull SCR_AIUtilityComponent utility) {}
	void PerformReaction(notnull SCR_AIGroupUtilityComponent utility) {}
	
	#ifdef AI_DEBUG
	protected void AddDebugMessage(SCR_AIUtilityComponent utility, string str)
	{
		SCR_AIInfoBaseComponent infoComp = SCR_AIInfoBaseComponent.Cast(utility.GetOwner().FindComponent(SCR_AIInfoBaseComponent));
		infoComp.AddDebugMessage(string.Format("%1: %2", this, str), msgType: EAIDebugMsgType.REACTION);
	}
	#endif
};

//------------------------------------------------------------------------------------------------
// GENERIC REACTIONS
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIWaitReaction : SCR_AIReactionBase
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility) 
	{
		auto behavior = new SCR_AIWaitBehavior(utility, null);
		//if (behavior.m_sBehaviorTree != string.Empty)
			//behavior.m_sBehaviorTree = m_OverrideBehaviorTree;
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIIdleReaction : SCR_AIReactionBase
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility) 
	{
		auto behavior = new SCR_AIIdleBehavior(utility, null);
		//if (behavior.m_sBehaviorTree != string.Empty)
			//behavior.m_sBehaviorTree = m_OverrideBehaviorTree;
		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility) 
	{
		auto activity = new SCR_AIIdleActivity(utility, null);
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIObserveThreatSystemReaction : SCR_AIReactionBase
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility)
	{
		auto behavior = new SCR_AIObserveThreatSystemBehavior(utility, null);
		utility.AddAction(behavior);
	}
}
