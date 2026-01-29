class SCR_AttackTaskEntityClass : SCR_TaskClass
{
}

class SCR_AttackTaskEntity : SCR_Task
{
	protected const int PERIODICAL_CHECK_INTERVAL = 2000; // ms

	[Attribute("150", UIWidgets.EditBox, "Area radius [m]", "0 inf")]
	protected float m_fAreaRadius;

	[Attribute("60", UIWidgets.EditBox, "Reset timer to this time [s] when task is activated by player or by killing enemy", "0 inf")]
	protected float m_fEvaluationTime;

	[Attribute("120", UIWidgets.EditBox, "Time [s] since activation when a task can fail", "0 inf")]
	protected float m_fFailSafeTime;

	[Attribute("10", UIWidgets.EditBox, "Bonus xp for enemy killed when task is completed", "0 inf")]
	protected int m_iEnemyKillBonusXp_TaskCompleted;

	[Attribute("5", UIWidgets.EditBox, "Bonus xp for enemy killed when task is failed", "0 inf")]
	protected int m_iEnemyKillBonusXp_TaskFailed;

	protected WorldTimestamp m_EvaluationTimestamp;
	protected WorldTimestamp m_TaskActivatedTimestamp;
	protected float m_fAreaRadiusSq;
	protected int m_iKilledEnemies;
	protected int m_iKilledAssignedPlayers;
	protected ref array<int> m_aCombatPlayerList = {}; //! array of playerIDs
	protected ref array<int> m_aRewardPlayerList = {}; //! array of playerIDs
	protected bool m_bWasActivated;
	protected RplComponent m_RplComponent;

	//------------------------------------------------------------------------------------------------
	protected void PeriodicalCheck()
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		SCR_ChimeraCharacter character;
		CharacterControllerComponent charControl;
		array<int> assigneePlayerIDs = GetTaskAssigneePlayerIDs();
		if (!assigneePlayerIDs)
			return;

		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		bool isAnyAssignedPlayerInZone = false;
		foreach (int playerId : assigneePlayerIDs)
		{
			character = SCR_ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerId));
			if (!character)
				continue;

			charControl = character.GetCharacterController();
			if (!charControl || charControl.IsDead())
				continue;

			if (vector.DistanceSqXZ(character.GetOrigin(), GetTaskPosition()) > m_fAreaRadiusSq)
				continue;

			RegisterPlayer(playerId);

			if (!m_bWasActivated)
			{
				m_TaskActivatedTimestamp = world.GetServerTimestamp();
				m_EvaluationTimestamp = world.GetServerTimestamp().PlusSeconds(m_fEvaluationTime);
				m_bWasActivated = true;
				#ifdef ATTACK_TASK_DEBUG
				Print("Attack task, task activated", LogLevel.DEBUG);
				#endif
			}

			isAnyAssignedPlayerInZone = true;
		}

		if (!m_bWasActivated)
			return;

		// no one was killed and a player from the assigned group left the zone => reset evaluation timer
		if (m_iKilledEnemies == 0 && m_iKilledAssignedPlayers == 0 && !isAnyAssignedPlayerInZone)
			m_EvaluationTimestamp = world.GetServerTimestamp().PlusSeconds(m_fEvaluationTime);

		WorldTimestamp currentTime = world.GetServerTimestamp();

		#ifdef ATTACK_TASK_DEBUG
		PrintFormat("Attack task, finish remaining time:%1, timeFromStart:%2", m_EvaluationTimestamp.DiffSeconds(currentTime), m_TaskActivatedTimestamp.DiffSeconds(currentTime), level: LogLevel.DEBUG);
		#endif

		if (currentTime.Less(m_EvaluationTimestamp))
			return;

		if (!m_aCombatPlayerList.IsEmpty())
		{
			#ifdef ATTACK_TASK_DEBUG
			Print("Attack task, completed", LogLevel.DEBUG);
			#endif

			m_TaskSystem.SetTaskState(this, SCR_ETaskState.COMPLETED);
			DeleteTask();
		}
		else if (currentTime.GreaterEqual(m_TaskActivatedTimestamp.PlusSeconds(m_fFailSafeTime)))
		{
			#ifdef ATTACK_TASK_DEBUG
			Print("Attack task, failed", LogLevel.DEBUG);
			#endif

			m_TaskSystem.SetTaskState(this, SCR_ETaskState.FAILED);
			DeleteTask();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		bool wasAssignedPlayerKilled;

		// remove a player from the combat list if he dies
		if (m_aCombatPlayerList.Contains(instigatorContextData.GetVictimPlayerID()))
		{
			m_aCombatPlayerList.RemoveItem(instigatorContextData.GetVictimPlayerID());
			wasAssignedPlayerKilled = true;

			#ifdef ATTACK_TASK_DEBUG
			PrintFormat("Attack task, playerID:%1 was killed and removed from combat list", instigatorContextData.GetVictimPlayerID(), level: LogLevel.DEBUG);
			#endif
		}

		// check if was killed by enemy
		if (!instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_ENEMY_PLAYER | SCR_ECharacterDeathStatusRelations.KILLED_BY_ENEMY_AI))
			return;

		IEntity victimEntity = instigatorContextData.GetVictimEntity();
		if (!victimEntity)
			return;

		IEntity killerEntity = instigatorContextData.GetKillerEntity();
		if (!killerEntity)
			return;

		// player from assigned group was killed by enemy
		if (wasAssignedPlayerKilled)
		{
			m_iKilledAssignedPlayers++;

			#ifdef ATTACK_TASK_DEBUG
			PrintFormat("Attack task, playerID:%1 was killed by enemy", instigatorContextData.GetVictimPlayerID(), level: LogLevel.DEBUG);
			#endif
		}

		// check distance from task, if killer or victim is in radius, it's count as kill
		if (vector.DistanceSq(GetTaskPosition(), victimEntity.GetOrigin()) > m_fAreaRadiusSq &&
			vector.DistanceSq(GetTaskPosition(), killerEntity.GetOrigin()) > m_fAreaRadiusSq)
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
		int killerPlayerGroup = group.GetGroupID();
		if (!IsTaskAssignedTo(SCR_TaskExecutor.FromGroup(killerPlayerGroup)))
			return;

		// when AI group kill enemy, leader will get into combat list
		killerId = group.GetLeaderID();

		#ifdef ATTACK_TASK_DEBUG
		Print("Attack task, somebody was killed in the zone, timer was reset", LogLevel.DEBUG);
		#endif

		// somebody was killed(player, AI) in the zone => reset combat timer
		ChimeraWorld world = GetGame().GetWorld();
		if (world)
		{
			m_EvaluationTimestamp = world.GetServerTimestamp().PlusSeconds(m_fEvaluationTime);
			if (!m_bWasActivated)
				m_TaskActivatedTimestamp = world.GetServerTimestamp();
		}

		m_bWasActivated = true;
		m_iKilledEnemies++;

		if (killerId > 0)
			RegisterPlayer(killerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterPlayer(int playerId)
	{
		if (!m_aCombatPlayerList.Contains(playerId))
		{
			m_aCombatPlayerList.Insert(playerId);

			#ifdef ATTACK_TASK_DEBUG
			Print("Attack task, playerId"+playerId+" is registered to combat list", LogLevel.DEBUG);
			#endif
		}

		if (!m_aRewardPlayerList.Contains(playerId))
			m_aRewardPlayerList.Insert(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddXPReward(SCR_ETaskState state)
	{
		if (state != SCR_ETaskState.COMPLETED && state != SCR_ETaskState.FAILED)
			return;

		SCR_XPHandlerComponent xpHandler = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
		if (!xpHandler || !m_RplComponent || m_RplComponent.IsProxy())
			return;

		// add XP to all players in the assigned group
		array<int> assigneePlayerIDs = GetTaskAssigneePlayerIDs();
		if (!assigneePlayerIDs)
			return;

		int bonusXp = m_iKilledEnemies;
		SCR_EXPRewards xpRewardType;

		if (state == SCR_ETaskState.COMPLETED)
		{
			xpRewardType = SCR_EXPRewards.ATTACK_TASK_COMPLETED;
			bonusXp *= m_iEnemyKillBonusXp_TaskCompleted;
		}
		else if (state == SCR_ETaskState.FAILED)
		{
			xpRewardType = SCR_EXPRewards.ATTACK_TASK_FAILED;
			bonusXp *= m_iEnemyKillBonusXp_TaskFailed;
		}

		int taskXp = xpHandler.GetXPRewardAmount(xpRewardType);
		if (taskXp <= 0)
			return;

		float bonusMultiplier = 1 + (bonusXp / taskXp); //! computed multiplier with bonus for enemies killed

		foreach (int playerID : assigneePlayerIDs)
		{
			if (m_aRewardPlayerList.Contains(playerID))
			{
				xpHandler.AwardXP(playerID, xpRewardType, bonusMultiplier, false);

				#ifdef ATTACK_TASK_DEBUG
				PrintFormat("Attack task, player:%1 is rewarded, taskXp:%2, bonusXp:%3 for killed enemies:%4", playerID, taskXp, bonusXp, m_iKilledEnemies, level: LogLevel.DEBUG);
				#endif
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void SetTaskState(SCR_ETaskState state)
	{
		AddXPReward(state);

		super.SetTaskState(state);
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteTask()
	{
		if (!m_TaskSystem)
			return;

		m_TaskSystem.DeleteTask(this);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode(this))
			return;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		m_fAreaRadiusSq = m_fAreaRadius * m_fAreaRadius;
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		GetGame().GetCallqueue().CallLater(PeriodicalCheck, PERIODICAL_CHECK_INTERVAL, true);
		gameMode.GetOnControllableDestroyed().Insert(OnControllableDestroyed);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_AttackTaskEntity()
	{
		GetGame().GetCallqueue().Remove(PeriodicalCheck);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		gameMode.GetOnControllableDestroyed().Remove(OnControllableDestroyed);
	}
}
