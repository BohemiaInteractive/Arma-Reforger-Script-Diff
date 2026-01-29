class SCR_RearmRequestedTaskEntityClass : SCR_BaseRequestedTaskEntityClass
{
}

class SCR_RearmRequestedTaskEntity : SCR_BaseRequestedTaskEntity
{
	protected const int PERIODICAL_CHECK_INTERVAL = 2000; //ms

	[Attribute("150", UIWidgets.EditBox, "Area radius [m]", "0 inf")]
	protected float m_fAreaRadius;

	[Attribute("180", UIWidgets.EditBox, "The time [s] it takes to finish a task when arsenal truck is in the zone", "0 inf")]
	protected float m_fFinishTime;

	protected float m_fAreaRadiusSq;
	protected int m_iSpentSupplies;
	protected SCR_GroupsManagerComponent m_GroupsManagerComponent;
	protected RplComponent m_RplComponent;

	//! map for tracking arsenal trucks which got in the zone, <vehicle, serverTimeStamp>
	protected ref map<IEntity, WorldTimestamp> m_mArsenalTrucksInZone = new map<IEntity, WorldTimestamp>();

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
			comp.AwardXP(playerID, SCR_EXPRewards.REARM_TASK_COMPLETED, 1.0, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterVehicle(IEntity vehicle)
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		// set vehicle start timestamp in the zone
		m_mArsenalTrucksInZone.Set(vehicle, world.GetServerTimestamp());

		#ifdef REARM_TASK_DEBUG
		PrintFormat("Rearm task, arsenal truck registered to the zone");
		#endif
	}

	//------------------------------------------------------------------------------------------------
	protected void UnregisterVehicle(IEntity vehicle)
	{
		m_mArsenalTrucksInZone.Remove(vehicle);

		#ifdef REARM_TASK_DEBUG
		PrintFormat("Rearm task, arsenal truck unregistered from the zone");
		#endif
	}

	//------------------------------------------------------------------------------------------------
	protected void PeriodicalCheck()
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;

		WorldTimestamp currentTime = world.GetServerTimestamp();

		array<int> assigneePlayerIDs = GetTaskAssigneePlayerIDs();
		if (!assigneePlayerIDs)
			return; // TODO check can be there return

		foreach (int playerID : assigneePlayerIDs)
		{
			IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			if (!playerEntity)
				continue;

			SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(playerEntity.FindComponent(SCR_CompartmentAccessComponent));
			if (!compartmentAccess)
				continue;

			IEntity vehicle = compartmentAccess.GetVehicle();
			if (!vehicle)
				continue;

			SCR_EditableVehicleComponent editableVehicle = SCR_EditableVehicleComponent.Cast(vehicle.FindComponent(SCR_EditableVehicleComponent));
			if (!editableVehicle)
				continue;

			// check if it is arsenal truck
			SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(editableVehicle.GetInfo());
			if (!info || !info.HasEntityLabel(EEditableEntityLabel.TRAIT_REARMING))
				continue;

			if (vector.DistanceSqXZ(vehicle.GetOrigin(), GetOrigin()) > m_fAreaRadiusSq)
			{
				// player could leave the zone
				UnregisterVehicle(vehicle);
				continue;
			}

			if (m_mArsenalTrucksInZone.Contains(vehicle))
				continue;

			// the vehicle has entered the zone by a player who is in the assigned group

			SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(vehicle);
			if (!resourceComponent)
				continue;

			SCR_ResourceContainer container = resourceComponent.GetContainer(EResourceType.SUPPLIES);
			if (!container)
				continue;

			// check resourses
			float neededResources = container.GetMaxResourceValue() * 0.5;
			if (container.GetResourceValue() < neededResources)
				continue;

			RegisterVehicle(vehicle);
		}

		foreach (IEntity vehicle, WorldTimestamp timestamp : m_mArsenalTrucksInZone)
		{
			if (!vehicle)
				continue; // probably destroyed

			SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.Cast(vehicle.FindComponent(SCR_DamageManagerComponent));
			if (!damageManager || damageManager.GetState() == EDamageState.DESTROYED)
				continue;

			// check finish by time
			WorldTimestamp finishTimestamp = timestamp.PlusSeconds(m_fFinishTime);

			#ifdef REARM_TASK_DEBUG
			PrintFormat("Rearm task, finish remaining time %1", finishTimestamp.DiffSeconds(currentTime));
			#endif

			if (currentTime.Less(finishTimestamp))
				continue;

			m_TaskSystem.SetTaskState(this, SCR_ETaskState.COMPLETED);
			DeleteTask();

			break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnArsenalItemRequested(SCR_ResourceComponent resourceComponent, ResourceName resourceName, IEntity requesterEntity, BaseInventoryStorageComponent inventoryStorageComponent, EResourceType resourceType, int resourceValue)
	{
		if (!resourceComponent || !inventoryStorageComponent)
			return;

		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(resourceComponent.GetOwner());
		if (!arsenalComponent || resourceType != EResourceType.SUPPLIES)
			return;

		IEntity parent = resourceComponent.GetOwner().GetRootParent(); // vehicle
		if (!parent)
			return;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(inventoryStorageComponent.GetOwner());
		if (!character && inventoryStorageComponent.GetOwner())
			character = SCR_ChimeraCharacter.Cast(inventoryStorageComponent.GetOwner().GetParent());

		int requesterPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character);

		#ifdef REARM_TASK_DEBUG
		PrintFormat("Rearm task, OnArsenalItemRequested, playerId:%1 character:%2", requesterPlayerId, character);
		#endif

		RegisterSpentSupplies(parent, requesterPlayerId, resourceValue);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSupportStationExecuted(SCR_BaseSupportStationComponent supportStation, ESupportStationType supportStationType, IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		if (!supportStation)
			return;

		IEntity parent = supportStation.GetOwner().GetRootParent();
		if (!parent || !action || !actionUser)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(actionUser);

		RegisterSpentSupplies(parent, playerId, action.GetSupportStationSuppliesOnUse());
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterSpentSupplies(IEntity vehicle, int playerId, int supplyCost)
	{
		if (supplyCost <= 0 || !vehicle || !m_mArsenalTrucksInZone.Contains(vehicle))
			return;

		array<int> assigneePlayerIDs = GetTaskAssigneePlayerIDs();
		if (!assigneePlayerIDs)
			return;

		// ignore players with same task
		SCR_TaskExecutor taskExecutor = SCR_TaskExecutor.FromPlayerID(playerId);
		if (assigneePlayerIDs.Contains(playerId))
			return;

		m_iSpentSupplies += supplyCost;

		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(vehicle);
		if (!resourceComponent)
			return;

		SCR_ResourceContainer container = resourceComponent.GetContainer(EResourceType.SUPPLIES);
		if (!container)
			return;

		float neededSupplies = container.GetMaxResourceValue() * 0.5;

		#ifdef REARM_TASK_DEBUG
		PrintFormat("Rearm task, SpentSupplies:%1 neededSupplies:%2", m_iSpentSupplies, neededSupplies);
		#endif

		// check spent resourses from arsenal truck for finish task sooner
		if (m_iSpentSupplies < neededSupplies)
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

			#ifdef REARM_TASK_DEBUG
			PrintFormat("Rearm task, finish task");
			#endif
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (SCR_Global.IsEditMode(this))
			return;

		m_fAreaRadiusSq = m_fAreaRadius * m_fAreaRadius;

		GetOnTaskStateChanged().Insert(OnTaskStateChanged);

		m_GroupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;

		GetGame().GetCallqueue().CallLater(PeriodicalCheck, PERIODICAL_CHECK_INTERVAL, true);

		SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
		if (supportStationManager)
			supportStationManager.GetOnSupportStationExecutedSuccessfully().Insert(OnSupportStationExecuted);

		SCR_ResourcePlayerControllerInventoryComponent.GetOnArsenalItemRequested().Insert(OnArsenalItemRequested);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_RearmRequestedTaskEntity()
	{
		GetOnTaskStateChanged().Remove(OnTaskStateChanged);
		GetGame().GetCallqueue().Remove(PeriodicalCheck);

		SCR_SupportStationManagerComponent supportStationManager = SCR_SupportStationManagerComponent.GetInstance();
		if (supportStationManager)
			supportStationManager.GetOnSupportStationExecutedSuccessfully().Remove(OnSupportStationExecuted);

		m_mArsenalTrucksInZone.Clear();

		SCR_ResourcePlayerControllerInventoryComponent.GetOnArsenalItemRequested().Remove(OnArsenalItemRequested);
	}
}
