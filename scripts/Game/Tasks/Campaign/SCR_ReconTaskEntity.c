class SCR_ReconTaskEntityClass : SCR_TaskClass
{
}

class SCR_ReconTaskEntity : SCR_Task
{
	[Attribute("300", UIWidgets.EditBox, "Antiexploit cooldown [s], after the task is completed, a cooldown is set for the player", "0 inf")]
	protected float m_fRewardCooldown;

	protected RplComponent m_RplComponent;

	// hidden anti-exploit cooldown
	protected static ref map<int, WorldTimestamp> m_mRewardCooldowns = new map<int, WorldTimestamp>(); //! <playerId, cooldownTimestamp>

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
			// players can get a reward when the cooldown ends, this is an anti-exploit protection
			if (!HasPlayerRewardCooldown(playerID))
			{
				comp.AwardXP(playerID, SCR_EXPRewards.RECON_TASK_COMPLETED, 1.0, false);
				SetPlayerRewardCooldown(playerID);
			}
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
	//! \param[in] playerId
	//! returns true if player has active reward cooldown
	protected bool HasPlayerRewardCooldown(int playerId)
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return false;

		WorldTimestamp cooldownTimestamp;
		if (!m_mRewardCooldowns.Find(playerId, cooldownTimestamp))
			return false;

		return !world.GetServerTimestamp().Greater(cooldownTimestamp);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets to player reward cooldown, this is an anti-exploit protection
	//! \param[in] playerId
	protected void SetPlayerRewardCooldown(int playerId)
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		m_mRewardCooldowns.Set(playerId, world.GetServerTimestamp().PlusSeconds(m_fRewardCooldown));
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
	}
}
