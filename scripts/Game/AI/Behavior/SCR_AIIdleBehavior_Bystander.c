//---------------------------------------------------------------------------------------------------------------------------------
//! Idle behavior of rally bystanders, runs the gesture idle tree. Used only by the bystander agent prefab.
class SCR_AIIdleBehavior_Bystander : SCR_AIIdleBehavior
{
	void SCR_AIIdleBehavior_Bystander(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity)
	{
		if (!utility)
			return;
		m_sBehaviorTree = "{D026C0F1C0207A6C}AI/BehaviorTrees/Chimera/Bystander/Bystander_Idle.bt";
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionSelected()
	{
		super.OnActionSelected();

		SCR_ChimeraAIBystander bystander = SCR_ChimeraAIBystander.Cast(m_Utility.GetOwner());
		if (!bystander)
			return;
		bystander.ToggleAnimationInvoker(false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();

		SCR_ChimeraAIBystander bystander = SCR_ChimeraAIBystander.Cast(m_Utility.GetOwner());
		if (!bystander)
			return;

		ScriptInvokerBase<SCR_AIOnAnimationBehaviorAction> gestureInvoker = bystander.GetOnAnimationBehaviorAction();
		if (!gestureInvoker)
			return;

		gestureInvoker.Invoke(bystander, false, true, 0);
		bystander.ToggleAnimationInvoker(true);
	}
};

//---------------------------------------------------------------------------------------------------------------------------------
//! Default idle reaction for rally bystanders, creates the bystander idle behavior.
[BaseContainerProps()]
class SCR_AIIdleReaction_Bystander : SCR_AIIdleReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility)
	{
		utility.AddAction(new SCR_AIIdleBehavior_Bystander(utility, null));
	}
};
