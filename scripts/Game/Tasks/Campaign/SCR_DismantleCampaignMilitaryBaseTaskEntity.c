class SCR_DismantleCampaignMilitaryBaseTaskEntityClass : SCR_CampaignMilitaryBaseTaskEntityClass
{
}

class SCR_DismantleCampaignMilitaryBaseTaskEntity : SCR_CampaignMilitaryBaseTaskEntity
{
	protected RplComponent m_RplComponent;

	//------------------------------------------------------------------------------------------------
	override protected void OnTargetBaseCaptured(SCR_MilitaryBaseComponent base, Faction faction)
	{
		if (!m_MilitaryBase || base != m_MilitaryBase)
			return;

		m_TaskSystem.SetTaskState(this, SCR_ETaskState.CANCELLED);
		DeleteTask();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnBaseUnregistered(SCR_MilitaryBaseComponent base)
	{
		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
			return;

		// base is removed - cancel task
		SCR_MilitaryBaseComponent targetBase = GetMilitaryBase();
		if (!base || base != targetBase)
			return;

		m_TaskSystem.SetTaskState(this, SCR_ETaskState.COMPLETED);
		DeleteTask();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskStateChanged(SCR_Task task, SCR_ETaskState newState)
	{
		if (task != this)
			return;

		if (newState == SCR_ETaskState.COMPLETED)
		{
			AddXPReward();
		}
	}

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
			comp.AwardXP(playerID, SCR_EXPRewards.DISMANTLE_BASE_COMPLETED, 1.0, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		GetOnTaskStateChanged().Insert(OnTaskStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_DismantleCampaignMilitaryBaseTaskEntity()
	{
		GetOnTaskStateChanged().Remove(OnTaskStateChanged);
	}
}
