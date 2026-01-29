class SCR_RepairRequestedTaskEntityClass : SCR_BaseRequestedTaskEntityClass
{
}

class SCR_RepairRequestedTaskEntity : SCR_BaseRequestedTaskEntity
{
	protected const int PERIODICAL_CHECK_INTERVAL = 5000; // ms
	protected const int DISTANCE_CHECK_INTERVAL = 2000; // ms

	[Attribute("150", UIWidgets.EditBox, "Area radius [m]", "0 inf")]
	protected float m_fAreaRadius;

	[Attribute("900", UIWidgets.EditBox, "After this time [s], task is cancelled", "0 inf")]
	protected float m_fAutoCancelTime;

	protected RplComponent m_RplComponent;
	protected float m_fAreaRadiusSq;
	protected WorldTimestamp m_AutoCancelTimestamp;
	protected bool m_bIsAreaCheckRunning;
	protected bool m_bWasLocalPlayerInArea;

	//------------------------------------------------------------------------------------------------
	protected void StartCheckingDistanceToArea()
	{
		if (m_bIsAreaCheckRunning || !IsTaskAssignedTo(SCR_TaskExecutorPlayer.FromPlayerID(SCR_PlayerController.GetLocalPlayerId())))
			return;

		m_bIsAreaCheckRunning = true;
		GetGame().GetCallqueue().CallLater(CheckDistanceToArea, DISTANCE_CHECK_INTERVAL, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void StopCheckingDistanceToArea()
	{
		if (!m_bIsAreaCheckRunning && IsTaskAssignedTo(SCR_TaskExecutorPlayer.FromPlayerID(SCR_PlayerController.GetLocalPlayerId())))
			return;

		m_bIsAreaCheckRunning = false;
		GetGame().GetCallqueue().Remove(CheckDistanceToArea);
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckDistanceToArea()
	{
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
			return;

		float distance = vector.DistanceSqXZ(player.GetOrigin(), GetTaskPosition());

		if (!m_bWasLocalPlayerInArea && distance <= m_fAreaRadiusSq)
		{
			m_bWasLocalPlayerInArea = true;
			SCR_NotificationsComponent.SendLocal(ENotification.GROUP_TASK_REPAIR_ENTERED_AREA);
		}
		else if (m_bWasLocalPlayerInArea && distance > m_fAreaRadiusSq)
		{
			m_bWasLocalPlayerInArea = false;
			SCR_NotificationsComponent.SendLocal(ENotification.GROUP_TASK_REPAIR_LEAVED_AREA);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskAssigneeAdded(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		if (task != this)
			return;

		StartCheckingDistanceToArea();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskAssigneeRemoved(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		if (task != this)
			return;

		StopCheckingDistanceToArea();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDataLoaded()
	{
		StartCheckingDistanceToArea();
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
	protected void PeriodicalCheck()
	{
		PlayerManager pManager = GetGame().GetPlayerManager();
		SCR_ChimeraCharacter character;
		CharacterControllerComponent charControl;
		array<int> assigneePlayerIDs = GetTaskAssigneePlayerIDs();
		if (!assigneePlayerIDs)
			return;

		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
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

			// assigned player is in zone -> reset auto cancel timer
			m_AutoCancelTimestamp = world.GetServerTimestamp().PlusSeconds(m_fAutoCancelTime);

			return;
		}

		// when auto cancel timer ends, the task is cancelled
		if (world.GetServerTimestamp().Greater(m_AutoCancelTimestamp))
		{
			SetTaskState(SCR_ETaskState.CANCELLED);
			DeleteTask();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVehicleRepaired(SCR_BaseSupportStationComponent supportStation, ESupportStationType supportStationType, IEntity actionTarget, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		if (!m_TaskSystem)
			return;

		// Support station is not a repair station
		if (supportStationType != ESupportStationType.REPAIR)
			return;

		if (!action || !supportStation)
			return;

		IEntity repairedVehicle = actionTarget.GetRootParent();
		if (!repairedVehicle)
			return;

		// User is not a player
		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(actionUser);
		if (playerId == 0)
			return;

		if (vector.DistanceSqXZ(repairedVehicle.GetOrigin(), GetTaskPosition()) > m_fAreaRadiusSq)
			return;

		if (!IsTaskAssignedTo(SCR_TaskExecutorPlayer.FromPlayerID(playerId)))
			return;

		Vehicle vehicleWithRepairBox = Vehicle.Cast(supportStation.GetOwner().GetParent());
		if (!vehicleWithRepairBox)
		{
			SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.GROUP_TASK_REPAIR_TRUCK_IS_NOT_IN_RANGE);
			return;
		}

		SCR_EditableVehicleComponent editableVehicleWithRepairBox = SCR_EditableVehicleComponent.Cast(vehicleWithRepairBox.FindComponent(SCR_EditableVehicleComponent));
		if (!editableVehicleWithRepairBox)
		{
			SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.GROUP_TASK_REPAIR_TRUCK_IS_NOT_IN_RANGE);
			return;
		}

		// check if it is repair truck
		SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(editableVehicleWithRepairBox.GetInfo());
		if (!info || !info.HasEntityLabel(EEditableEntityLabel.TRAIT_REPAIRING))
		{
			SCR_NotificationsComponent.SendToPlayer(playerId, ENotification.GROUP_TASK_REPAIR_TRUCK_IS_NOT_IN_RANGE);
			return;
		}

		if (!IsVehiclePartFullyRepaired(actionTarget))
			return;

		m_TaskSystem.SetTaskState(this, SCR_ETaskState.COMPLETED);
		DeleteTask();
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
			comp.AwardXP(playerID, SCR_EXPRewards.REPAIR_TASK_COMPLETED, 1.0, false);
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
	protected bool IsVehiclePartFullyRepaired(IEntity actionTarget)
	{
		SCR_DamageManagerComponent damageComponent = SCR_DamageManagerComponent.GetDamageManager(actionTarget);
		if (!damageComponent)
			return false;

		HitZone hitZone = damageComponent.GetDefaultHitZone();
		if (!hitZone)
			return false;

		return hitZone.GetHealthScaled() == 1;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsVehicleFullyRepaired(IEntity vehicle)
	{
		SCR_DamageManagerComponent damageComponent = SCR_DamageManagerComponent.GetDamageManager(vehicle);
		if (!damageComponent)
			return false;

		// Check if any hit zone is damaged
		array<HitZone> hitZones = {};
		damageComponent.GetAllHitZonesInHierarchy(hitZones);
		SCR_FlammableHitZone flammableHitZone;
		foreach (HitZone hitZone : hitZones)
		{
			if (hitZone && hitZone.GetDamageState() != EDamageState.UNDAMAGED)
				return false;

			// Flammable hit zone may be smoking
			flammableHitZone = SCR_FlammableHitZone.Cast(hitZone);
			if (flammableHitZone && damageComponent.IsOnFire(flammableHitZone))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode(this))
			return;

		m_fAreaRadiusSq = m_fAreaRadius * m_fAreaRadius;

		if (!System.IsConsoleApp())
		{
			GetOnTaskAssigneeAdded().Insert(OnTaskAssigneeAdded);
			GetOnTaskAssigneeRemoved().Insert(OnTaskAssigneeRemoved);
		}

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		ChimeraWorld world = GetGame().GetWorld();
		if (world)
			m_AutoCancelTimestamp = world.GetServerTimestamp().PlusSeconds(m_fAutoCancelTime);

		SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
		if (supportStationManager)
			supportStationManager.GetOnSupportStationExecutedSuccessfully().Insert(OnVehicleRepaired);

		GetGame().GetCallqueue().CallLater(PeriodicalCheck, PERIODICAL_CHECK_INTERVAL, true);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_RepairRequestedTaskEntity()
	{
		GetOnTaskAssigneeAdded().Remove(OnTaskAssigneeAdded);
		GetOnTaskAssigneeRemoved().Remove(OnTaskAssigneeRemoved);
		GetGame().GetCallqueue().Remove(CheckDistanceToArea);

		SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
		if (supportStationManager)
			supportStationManager.GetOnSupportStationExecutedSuccessfully().Remove(OnVehicleRepaired);

		GetGame().GetCallqueue().Remove(PeriodicalCheck);
	}
}
