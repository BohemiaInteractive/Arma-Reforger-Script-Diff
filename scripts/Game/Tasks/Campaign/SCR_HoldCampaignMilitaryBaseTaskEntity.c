class SCR_HoldCampaignMilitaryBaseTaskEntityClass : SCR_CampaignMilitaryBaseTaskEntityClass
{
}

class SCR_HoldCampaignMilitaryBaseTaskEntity : SCR_CampaignMilitaryBaseTaskEntity
{
	protected const int PERIODICAL_CHECK_INTERVAL = 5000; // ms

	[Attribute("200", UIWidgets.EditBox, "Area radius [m] around base", "0 inf")]
	protected float m_fAreaRadiusAroundBase;

	[Attribute("300", UIWidgets.EditBox, "The time [s] it takes to finish a task when no combat is going on in the zone and at least one enemy character was killed", "0 inf")]
	protected float m_fNoCombatFinishTime;

	[Attribute("600", UIWidgets.EditBox, "The time [s] it takes to automatically cancel a task when no enemy has been killed in the zone", "0 inf")]
	protected float m_fAutoCancelTime;

	protected RplComponent m_RplComponent;
	protected float m_fAreaRadiusAroundBaseSq;
	protected bool m_bWasEnemyKilledInZone;
	protected WorldTimestamp m_LastCombatTimestamp; // is used for autocancel and win condition
	protected WorldTimestamp m_TaskStartedTimestamp; // is used for autocancel

	//------------------------------------------------------------------------------------------------
	protected void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		// check if was killed by enemy
		if (!instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_ENEMY_PLAYER | SCR_ECharacterDeathStatusRelations.KILLED_BY_ENEMY_AI))
			return;

		IEntity victimEntity = instigatorContextData.GetVictimEntity();
		if (!victimEntity)
			return;

		vector basePositon = GetMilitaryBase().GetOwner().GetOrigin();

		// check distance from base
		if (vector.DistanceSq(basePositon, victimEntity.GetOrigin()) > m_fAreaRadiusAroundBaseSq)
			return;

		// somebody was killed(player, AI) in the zone => reset combat timer
		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			m_LastCombatTimestamp = world.GetServerTimestamp();

		#ifdef HOLD_TASK_DEBUG
		Print("Hold task, somebody was killed in the zone - reset combat timer");
		#endif

		// if an enemy has been killed by someone in the assigned group, we do not need to check again.
		if (m_bWasEnemyKilledInZone)
			return;

		IEntity killerEntity = instigatorContextData.GetKillerEntity();
		if (!killerEntity)
			return;

		// get faction from entity(player, AI)
		Faction killerFaction = SCR_Faction.GetEntityFaction(killerEntity);
		array<string> ownerFactionKeys = GetOwnerFactionKeys();
		if (!ownerFactionKeys || !ownerFactionKeys.Contains(killerFaction.GetFactionKey()))
			return;

		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup group;
		int killerId = instigatorContextData.GetKillerPlayerID();
		if (killerId <= 0)
		{
			// killer is AI
			AIControlComponent aiControlComponent = AIControlComponent.Cast(killerEntity.FindComponent(AIControlComponent));
			if (!aiControlComponent)
				return;

			AIAgent agent = aiControlComponent.GetAIAgent();
			if (!agent)
				return;

			group = SCR_AIGroup.Cast(agent);
			if (!group)
				group = SCR_AIGroup.Cast(agent.GetParentGroup());

			if (!group || !group.IsSlave() || !group.GetMaster())
				return;

			group = group.GetMaster();
		}
		else
		{
			// killer is player
			group = groupManager.GetPlayerGroup(killerId);
		}

		if (!group)
			return;

		// check if killer's group(could be master) is assigned in the task
		if (!IsTaskAssignedTo(SCR_TaskExecutor.FromGroup(group.GetGroupID())))
			return;

		m_bWasEnemyKilledInZone = true;

		#ifdef HOLD_TASK_DEBUG
		Print("Hold task, enemy was killed in the zone");
		#endif
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTargetBaseCaptured(SCR_MilitaryBaseComponent base, Faction faction)
	{
		if (!m_MilitaryBase || base != m_MilitaryBase)
			return;

		#ifdef HOLD_TASK_DEBUG
		PrintFormat("Hold task, base captured, task canceled");
		#endif

		SetTaskState(SCR_ETaskState.CANCELLED);
		DeleteTask();
	}

	//------------------------------------------------------------------------------------------------
	protected void PeriodicalCheck()
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		WorldTimestamp currentTime = world.GetServerTimestamp();

		if (m_bWasEnemyKilledInZone)
		{
			// check finish
			WorldTimestamp finishTimestamp = m_LastCombatTimestamp.PlusSeconds(m_fNoCombatFinishTime);

			#ifdef HOLD_TASK_DEBUG
			PrintFormat("Hold task, finish remaining time %1", finishTimestamp.DiffSeconds(currentTime));
			#endif

			if (currentTime.Less(finishTimestamp))
				return;

			#ifdef HOLD_TASK_DEBUG
			PrintFormat("Hold task, completed");
			#endif

			m_TaskSystem.SetTaskState(this, SCR_ETaskState.COMPLETED);
			DeleteTask();
		}
		else
		{
			// check auto cancel
			WorldTimestamp cancelTimestamp = m_TaskStartedTimestamp.PlusSeconds(m_fAutoCancelTime);

			#ifdef HOLD_TASK_DEBUG
			PrintFormat("Hold task, cancel remaining time %1", cancelTimestamp.DiffSeconds(currentTime));
			#endif

			if (currentTime.Less(cancelTimestamp))
				return;

			#ifdef HOLD_TASK_DEBUG
			PrintFormat("Hold task, auto cancel");
			#endif

			m_TaskSystem.SetTaskState(this, SCR_ETaskState.CANCELLED);
			DeleteTask();
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
			comp.AwardXP(playerID, SCR_EXPRewards.HOLD_TASK_COMPLETED, 1.0, false);
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

		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
			return;

		m_fAreaRadiusAroundBaseSq = m_fAreaRadiusAroundBase * m_fAreaRadiusAroundBase;

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		GetGame().GetCallqueue().CallLater(PeriodicalCheck, PERIODICAL_CHECK_INTERVAL, true);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		gameMode.GetOnControllableDestroyed().Insert(OnControllableDestroyed);

		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		m_TaskStartedTimestamp = world.GetServerTimestamp();
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_HoldCampaignMilitaryBaseTaskEntity()
	{
		GetGame().GetCallqueue().Remove(PeriodicalCheck);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		gameMode.GetOnControllableDestroyed().Remove(OnControllableDestroyed);
	}
}
