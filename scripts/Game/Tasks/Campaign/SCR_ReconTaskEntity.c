class SCR_ReconTaskEntityClass : SCR_TaskClass
{
}

class SCR_ReconTaskEntity : SCR_Task
{
	protected RplComponent m_RplComponent;

	//------------------------------------------------------------------------------------------------
	protected void AddXPReward()
	{
		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
		if (!comp || !m_RplComponent || m_RplComponent.IsProxy())
			return;

		// add XP to all players in the assigned group
		array<int> assigneePlayerIDs = GetTaskAssigneePlayerIDs();
		if (!assigneePlayerIDs)
			return;

		foreach (int playerID : assigneePlayerIDs)
		{
			comp.AwardXP(playerID, SCR_EXPRewards.RECON_TASK_COMPLETED, 1.0, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void SetTaskState(SCR_ETaskState state)
	{
		if (state == SCR_ETaskState.COMPLETED)
			AddXPReward();

		super.SetTaskState(state);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
	}
}
