class SCR_ReinforceRequestedTaskEntityClass : SCR_BaseRequestedTaskEntityClass
{
}

class SCR_ReinforceRequestedTaskEntity : SCR_BaseRequestedTaskEntity
{
	protected const int PERIODICAL_CHECK_INTERVAL = 2000; // ms

	[Attribute("150", UIWidgets.EditBox, "Area radius [m]", "0 inf")]
	protected float m_fAreaRadius;

	[Attribute("300", UIWidgets.EditBox, "[s] time since activation when the task fail", "0 inf")]
	protected float m_fFailTime;

	protected ref array<int> m_aArrivedPlayers = {}; //! array of playerIDs
	protected ref array<int> m_aDeadPlayers = {}; //! array of playerIDs
	protected WorldTimestamp m_TaskActivatedTimestamp;
	protected float m_fAreaRadiusSq;
	protected RplComponent m_RplComponent;
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected bool m_bWasAnyPlayerInArea;
	protected bool m_bIsAreaCheckRunning;
	protected bool m_bWasLocalPlayerInArea;

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
			comp.AwardXP(playerID, SCR_EXPRewards.REINFORCE_TASK_COMPLETED, 1.0, false);
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
	protected void StartCheckingArea()
	{
		if (m_bIsAreaCheckRunning || m_bWasLocalPlayerInArea)
			return;

		if (!IsTaskAssignedTo(SCR_TaskExecutorPlayer.FromPlayerID(SCR_PlayerController.GetLocalPlayerId())))
			return;

		m_bIsAreaCheckRunning = true;
		GetGame().GetCallqueue().CallLater(CheckReinforcementArea, SCR_GameModeCampaign.DEFAULT_DELAY, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void StopCheckingArea()
	{
		if (m_bIsAreaCheckRunning && IsTaskAssignedTo(SCR_TaskExecutorPlayer.FromPlayerID(SCR_PlayerController.GetLocalPlayerId())))
			return;

		m_bIsAreaCheckRunning = false;
		GetGame().GetCallqueue().Remove(CheckReinforcementArea);
	}

	//------------------------------------------------------------------------------------------------
	protected void PeriodicalCheck()
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		PlayerManager pManager = GetGame().GetPlayerManager();
		SCR_ChimeraCharacter character;
		CharacterControllerComponent charControl;
		array<int> assigneePlayerIDs = GetTaskAssigneePlayerIDs();
		if (!assigneePlayerIDs)
			return;

		foreach (int playerId : assigneePlayerIDs)
		{
			character = SCR_ChimeraCharacter.Cast(pManager.GetPlayerControlledEntity(playerId));
			if (!character)
				continue;

			charControl = character.GetCharacterController();
			if (!charControl || charControl.IsDead())
				continue;

			if (vector.DistanceSqXZ(character.GetOrigin(), GetTaskPosition()) > m_fAreaRadiusSq)
				continue;

			if (!m_bWasAnyPlayerInArea)
			{
				m_TaskActivatedTimestamp = world.GetServerTimestamp();
				m_bWasAnyPlayerInArea = true;
			}

			if (!m_aArrivedPlayers.Contains(playerId))
			{
				m_aArrivedPlayers.Insert(playerId);

				#ifdef REINFORCE_TASK_DEBUG
				Print("Reinforce requested task, playerId "+playerId+" is registered to evaluation player list", LogLevel.DEBUG);
				#endif
			}
		}

		int halfAssignedPlayers = assigneePlayerIDs.Count() * 0.5;

		#ifdef REINFORCE_TASK_DEBUG
		if (m_bWasAnyPlayerInArea)
			PrintFormat("Reinforce requested task, fail timer remaining time:%1", world.GetServerTimestamp().DiffSeconds(m_TaskActivatedTimestamp.PlusSeconds(m_fFailTime)), level: LogLevel.DEBUG);
		#endif

		if (m_aArrivedPlayers.Count() > (halfAssignedPlayers))
		{
			#ifdef REINFORCE_TASK_DEBUG
			Print("Reinforce requested task, completed", LogLevel.DEBUG);
			#endif

			m_TaskSystem.SetTaskState(this, SCR_ETaskState.COMPLETED);
			DeleteTask();
			return;
		}

		if (m_aDeadPlayers.Count() > halfAssignedPlayers ||
			m_bWasAnyPlayerInArea && world.GetServerTimestamp().GreaterEqual(m_TaskActivatedTimestamp.PlusSeconds(m_fFailTime)))
		{
			#ifdef REINFORCE_TASK_DEBUG
			Print("Reinforce requested task, failed", LogLevel.DEBUG);
			#endif

			m_TaskSystem.SetTaskState(this, SCR_ETaskState.FAILED);
			DeleteTask();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckReinforcementArea()
	{
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
			return;

		#ifdef REINFORCE_TASK_DEBUG
		Print("Reinforce requested task, CheckReinforcementArea", level: LogLevel.DEBUG);
		#endif

		if (!m_bWasLocalPlayerInArea && vector.DistanceSqXZ(player.GetOrigin(), GetTaskPosition()) <= m_fAreaRadiusSq)
		{
			m_bWasLocalPlayerInArea = true;
			SCR_NotificationsComponent.SendLocal(ENotification.GROUP_TASK_REINFORCE_ENTERED_AREA);
			StopCheckingArea();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		int victimPlayerID = instigatorContextData.GetVictimPlayerID();

		if (victimPlayerID == 0)
			return;

		if (!IsTaskAssignedTo(SCR_TaskExecutorPlayer.FromPlayerID(victimPlayerID)))
			return;

		// add a player to the dead player list if he dies
		if (!m_aDeadPlayers.Contains(victimPlayerID))
		{
			m_aDeadPlayers.Insert(victimPlayerID);

			#ifdef REINFORCE_TASK_DEBUG
			PrintFormat("Reinforce requested task, playerID:%1 was killed and added to dead player list", victimPlayerID, level: LogLevel.DEBUG);
			#endif
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskAssigneeAdded(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		if (task != this)
			return;

		StartCheckingArea();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskAssigneeRemoved(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		if (task != this)
			return;

		StopCheckingArea();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDataLoaded()
	{
		StartCheckingArea();
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		bool loaded = super.RplLoad(reader);
		if (loaded)
			OnDataLoaded();

		return loaded;
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
		m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();

		if (!System.IsConsoleApp())
		{
			GetOnTaskAssigneeAdded().Insert(OnTaskAssigneeAdded);
			GetOnTaskAssigneeRemoved().Insert(OnTaskAssigneeRemoved);
		}

		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		GetGame().GetCallqueue().CallLater(PeriodicalCheck, PERIODICAL_CHECK_INTERVAL, true);
		gameMode.GetOnControllableDestroyed().Insert(OnControllableDestroyed);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_ReinforceRequestedTaskEntity()
	{
		GetOnTaskAssigneeAdded().Remove(OnTaskAssigneeAdded);
		GetOnTaskAssigneeRemoved().Remove(OnTaskAssigneeRemoved);
		GetGame().GetCallqueue().Remove(PeriodicalCheck);
		GetGame().GetCallqueue().Remove(CheckReinforcementArea);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		gameMode.GetOnControllableDestroyed().Remove(OnControllableDestroyed);
	}
}
