[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_TransportUnitComponentClass : ScriptComponentClass
{
}

void OnTransportUnitVehicleChangedDelegate(Vehicle currentVehicle, Vehicle previousVehicle);
typedef func OnTransportUnitVehicleChangedDelegate;
typedef ScriptInvokerBase<OnTransportUnitVehicleChangedDelegate> OnTransportUnitVehicleChangedInvoker;

class SCR_TransportUnitComponent : ScriptComponent
{
	[Attribute(desc: "Name of the source base entity used to set source base. If empty, the base is not set.")]
	protected string m_sSourceBaseEntityName;

	[Attribute("100", desc: "Radius in meters within which the unit is searching for the vehicle.", params: "0 inf")]
	protected float m_fSearchTruckRadius;

	[Attribute("0", desc: "Should the transport group be teleported into the vehicle just before the task solving?")]
	protected bool m_bTeleportGroupToVehicle;

	[Attribute("#AR-SuppliesTransportSystem_TransportGroup_NoDestination", desc: "Localization key for cases where the transport group has no destination.")]
	protected LocalizedString m_sNoDestinationLocalizationKey;

	[Attribute("{349C8E58C5ACAF4D}Prefabs/Tasks/TransportUnitHelpTaskEntity.et", desc: "Task to be created if the transport unit fails to deliver supplies.", params: "et")]
	protected ResourceName m_sTransportUnitHelpTask;

	[RplProp()]
	protected SCR_ETransportUnitState m_eState;

	[RplProp(onRplName: "OnSourceBaseCallsignSet")]
	protected int m_iSourceBaseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;

	[RplProp(onRplName: "OnDestinationCallsignSet")]
	protected int m_iSuppliedBaseCallsign = SCR_CampaignMilitaryBaseComponent.INVALID_BASE_CALLSIGN;

	[RplProp()]
	protected RplId m_VehicleRplId = RplId.Invalid();

	[RplProp(onRplName: "OnPreferableResupplyTaskSet")]
	protected RplId m_PreferableResupplyTaskRplId = RplId.Invalid();

	[RplProp(onRplName: "OnVehicleAggregatedResourceValueSet")]
	protected float m_fVehicleAggregatedResourceValue = -1;

	[RplProp(onRplName: "OnIsVehicleUsableSet")]
	protected bool m_bIsVehicleUsable;

	[RplProp()]
	protected SCR_EVehicleConditionCheckType m_eVehicleFailedConditionType;

	//! World position of the group for the logistics map. Set by the server in Update (every few seconds),
	//! so clients know it even when the group members are not streamed in. vector.Zero until the first update.
	[RplProp()]
	protected vector m_vMapPosition;

	protected SCR_AIGroup m_AIGroup;

	protected SCR_ResupplyCampaignMilitaryBaseTaskEntity m_PreferableResupplyTask;

	protected ref set<Vehicle> m_NearbySupplyTrucks = new set<Vehicle>();

	protected SCR_SuppliesTransportSystem m_SuppliesTransportSystem;

	protected SCR_TaskSolverManagerComponent m_TaskSolverManager;

	protected SCR_CampaignMilitaryBaseComponent m_SourceBase;

	protected SCR_CampaignMilitaryBaseComponent m_SuppliedBase;

	protected Vehicle m_Vehicle;

	protected SCR_AIVehicleUsageComponent m_AiVehicleUsage;

	protected SCR_FuelManagerComponent m_FuelManager;

	protected SCR_ResupplyTaskSolver m_TaskSolver;

	protected SCR_VehicleConditionManager m_VehicleConditionManager;

	protected SCR_AIGroupConditionManager m_AiGroupConditionManager;

	protected SCR_TaskSystem m_TaskSystem;
	protected SCR_TransportUnitHelpTaskEntity m_TransportUnitHelpTaskEntity;

	protected SCR_BoardingEntityWaypoint m_BoardingWaypoint;
	protected SCR_AIWaypoint m_MoveToSourceBaseWaypoint;

	protected ref ScriptInvokerInt m_OnStateChanged;
	protected ref OnTransportUnitVehicleChangedInvoker m_OnVehicleChanged;
	protected ref ScriptInvokerInt m_OnResupplyTaskSolverStateChanged;
	protected ref ScriptInvokerFloat m_OnVehicleResourcesValueChanged;

	protected bool m_bGroupLifecyclePinned;
	protected int m_iImportanceBeforePin = -1;

	protected static int s_iLastGeneratedTaskId;

	protected static const string TRANSPORT_UNIT_HELP_TASK_ID = "%1_TransportUnitHelpTask_%2";

	// Fuel fraction (0-1) set in the truck when a task starts, and when the crew drives with low fuel
	protected static const float TRUCK_FUEL_PERCENTAGE = 0.75;
	// When the crew drives the truck and its fuel fraction (0-1) drops below this value, the fuel is set back to TRUCK_FUEL_PERCENTAGE
	protected static const float TRUCK_FUEL_TOP_UP_THRESHOLD = 0.1;
	// The map position is replicated again only when the group moved more than 10 m (value is squared, m^2)
	protected static const float MAP_POSITION_UPDATE_DISTANCE_SQ = 100;

	//------------------------------------------------------------------------------------------------
	ScriptInvokerFloat GetOnVehicleResourcesValueChanged()
	{
		if (!m_OnVehicleResourcesValueChanged)
			m_OnVehicleResourcesValueChanged = new ScriptInvokerFloat();

		return m_OnVehicleResourcesValueChanged;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnResupplyTaskSolverStateChanged()
	{
		if (!m_OnResupplyTaskSolverStateChanged)
			m_OnResupplyTaskSolverStateChanged = new ScriptInvokerInt();

		return m_OnResupplyTaskSolverStateChanged;
	}

	//------------------------------------------------------------------------------------------------
	OnTransportUnitVehicleChangedInvoker GetOnVehicleChanged()
	{
		if (!m_OnVehicleChanged)
			m_OnVehicleChanged = new OnTransportUnitVehicleChangedInvoker();

		return m_OnVehicleChanged;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnStateChanged()
	{
		if (!m_OnStateChanged)
			m_OnStateChanged = new ScriptInvokerInt();

		return m_OnStateChanged;
	}

	//------------------------------------------------------------------------------------------------
	Faction GetFaction()
	{
		if (!m_AIGroup)
			return null;

		return m_AIGroup.GetFaction();
	}

	//------------------------------------------------------------------------------------------------
	SCR_ResupplyTaskSolver GetSolver()
	{
		return m_TaskSolver;
	}

	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_AIGroup.GetCustomNameWithOriginal();
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent GetSourceBase()
	{
		return m_SourceBase;
	}

	//------------------------------------------------------------------------------------------------
	string GetDestinationName()
	{
		if (m_SourceBase && m_TaskSolver.GetResupplyTaskSolverState() == SCR_EResupplyTaskSolverState.MOVING_TO_SOURCE_BASE)
			return m_SourceBase.GetCallsignDisplayName();

		if (m_SuppliedBase && m_TaskSolver.GetResupplyTaskSolverState() == SCR_EResupplyTaskSolverState.MOVING_TO_SUPPLIED_BASE)
			return m_SuppliedBase.GetCallsignDisplayName();

		return m_sNoDestinationLocalizationKey;
	}

	//------------------------------------------------------------------------------------------------
	void SetSourceBase(SCR_CampaignMilitaryBaseComponent sourceBase)
	{
		if (m_SourceBase == sourceBase)
			return;

		m_SourceBase = sourceBase;
		if (m_SourceBase)
			m_iSourceBaseCallsign = m_SourceBase.GetCallsign();
		else
			m_iSourceBaseCallsign = SCR_CampaignMilitaryBaseComponent.INVALID_BASE_CALLSIGN;

		// On a task, the solver moves the group to the new source base itself
		m_TaskSolver.SetSourceBase(m_SourceBase);

		CancelReturnToOldSourceBase();

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Stops the return trip (not on a task) to the previous source base.
	//! The next Update calls ReturnToSourceBaseIfNeeded, which sends the group to the new source base.
	protected void CancelReturnToOldSourceBase()
	{
		if (m_eState != SCR_ETransportUnitState.INACTIVE || !m_MoveToSourceBaseWaypoint)
			return;

		if (m_TaskSolver.GetResupplyTaskSolverState() != SCR_EResupplyTaskSolverState.MOVING_TO_SOURCE_BASE)
			return;

		// Remove the handler first, so completing the old waypoint does not run it
		m_AIGroup.GetOnWaypointCompleted().Remove(OnMoveToSourceBaseWaypointCompleted);
		m_MoveToSourceBaseWaypoint = null;

		m_AIGroup.CompleteAllWaypoints();
		m_TaskSolver.ResetToInitialState();
	}

	//------------------------------------------------------------------------------------------------
	void SetSuppliedBase(SCR_CampaignMilitaryBaseComponent suppliedBase)
	{
		if (m_SuppliedBase == suppliedBase)
			return;

		m_SuppliedBase = suppliedBase;
		if (m_SuppliedBase)
			m_iSuppliedBaseCallsign = m_SuppliedBase.GetCallsign();
		else
			m_iSuppliedBaseCallsign = SCR_CampaignMilitaryBaseComponent.INVALID_BASE_CALLSIGN;

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	void SelectForTaskSolving(SCR_ResupplyCampaignMilitaryBaseTaskEntity task)
	{
		SCR_ResupplyTaskSolverContext resupplyTaskContext = new SCR_ResupplyTaskSolverContext(m_Vehicle, m_SourceBase);
		m_TaskSolverManager.StartSolverTask(m_TaskSolver, task, resupplyTaskContext);

		SetTruckFuel(TRUCK_FUEL_PERCENTAGE);

		SetState(SCR_ETransportUnitState.ON_TASK);

		if (m_bTeleportGroupToVehicle)
		{
			TeleportGroupToVehicle();
		}
	}

	//------------------------------------------------------------------------------------------------
	bool HasVehicle()
	{
		return m_VehicleRplId.IsValid();
	}

	//------------------------------------------------------------------------------------------------
	SCR_EVehicleConditionCheckType GetVehicleFailedConditionType()
	{
		return m_eVehicleFailedConditionType;
	}

	//------------------------------------------------------------------------------------------------
	Vehicle GetVehicle()
	{
		return m_Vehicle;
	}

	//------------------------------------------------------------------------------------------------
	bool IsVehicleUsable()
	{
		return m_bIsVehicleUsable;
	}

	//------------------------------------------------------------------------------------------------
	float GetVehicleAggregatedResourceValue()
	{
		return m_fVehicleAggregatedResourceValue;
	}

	//------------------------------------------------------------------------------------------------
	SCR_AIGroup GetAIGroup()
	{
		return m_AIGroup;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ETransportUnitState GetState()
	{
		return m_eState;
	}

	//------------------------------------------------------------------------------------------------
	SCR_EResupplyTaskSolverState GetResupplyTaskSolverState()
	{
		return m_TaskSolver.GetResupplyTaskSolverState();
	}

	//------------------------------------------------------------------------------------------------
	bool IsReadyForTaskSolving()
	{
		if (!m_SourceBase)
			return false;

		if (!m_PreferableResupplyTask)
			return false;

		if (!m_VehicleConditionManager.IsVehicleUsable(m_Vehicle))
			return false;

		if (GetState() == SCR_ETransportUnitState.ON_TASK)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Update(float timeSlice)
	{
		m_TaskSolver.Update(timeSlice);

		TopUpTruckFuelIfLow();

		if (GetState() != SCR_ETransportUnitState.ON_TASK)
		{
			SetPreferableResupplyTask(SelectPreferableResupplyTask());
			if (m_PreferableResupplyTask)
					SetSuppliedBase(m_PreferableResupplyTask.GetMilitaryBase());

			// Also give up the truck when another unit has started a task with it
			if (!m_Vehicle || !m_VehicleConditionManager.IsVehicleUsable(m_Vehicle) || m_SuppliesTransportSystem.IsVehicleUsed(m_Vehicle, this))
			{
				SetVehicle(FindVehicle());
			}
		}

		SCR_ETransportUnitState previousState = GetState();
		UpdateState();

		// The vehicle can become unusable without a state change (destroyed, sunk)
		UpdateGroupLifecyclePin();

		UpdateMapPosition();
	}

	//------------------------------------------------------------------------------------------------
	void ReturnToSourceBaseIfNeeded()
	{
		if (m_eState != SCR_ETransportUnitState.INACTIVE || m_TaskSolver.GetResupplyTaskSolverState() != SCR_EResupplyTaskSolverState.INACTIVE)
			return;

		if (!m_SourceBase || !m_Vehicle || !m_AiVehicleUsage)
			return;

		if (!m_VehicleConditionManager.IsVehicleUsable(m_Vehicle))
			return;

		if (m_SuppliesTransportSystem.IsVehicleBoarded(m_Vehicle, this))
			return;

		float sourceBaseRadius = m_SourceBase.GetRadius();
		bool isCloseToSourceBase = vector.DistanceSq(m_SourceBase.GetOwner().GetOrigin(), m_Vehicle.GetOrigin()) <= (sourceBaseRadius * sourceBaseRadius);
		if (isCloseToSourceBase)
			return;

		m_TaskSolver.SetVehicle(m_Vehicle);
		m_TaskSolver.SetSourceBase(m_SourceBase);

		m_AIGroup.CompleteAllWaypoints();

		if (m_AiVehicleUsage.IsOccupiedByGroup(m_AIGroup))
		{
			CreateMoveWaypointToSourceBase();
		}
		else
		{
			m_BoardingWaypoint = CreateBoardingVehicleWaypoint();
			if (!m_BoardingWaypoint)
				return;

			m_AIGroup.GetOnWaypointCompleted().Insert(OnBoardingCompleted);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBoardingCompleted(AIWaypoint waypoint)
	{
		if (!waypoint || waypoint != m_BoardingWaypoint)
			return;

		m_AIGroup.GetOnWaypointCompleted().Remove(OnBoardingCompleted);

		if (!m_Vehicle || !m_AiVehicleUsage)
			return;

		if (m_AiVehicleUsage.IsOccupiedByGroup(m_AIGroup))
			CreateMoveWaypointToSourceBase();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMoveToSourceBaseWaypointCompleted(AIWaypoint waypoint)
	{
		if (!waypoint || waypoint != m_MoveToSourceBaseWaypoint)
			return;

		m_AIGroup.GetOnWaypointCompleted().Remove(OnMoveToSourceBaseWaypointCompleted);

		m_TaskSolver.ResetToInitialState();
	}

	//------------------------------------------------------------------------------------------------
	protected static int GenerateTaskID()
	{
		return s_iLastGeneratedTaskId++;
	}

	//------------------------------------------------------------------------------------------------
	protected void TeleportGroupToVehicle()
	{
		if (!m_Vehicle)
			return;

		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		SCR_CompartmentAccessComponent compartmentAccess;
		foreach (AIAgent agent : agents)
		{
			compartmentAccess = SCR_CompartmentAccessComponent.Cast(agent.GetControlledEntity().FindComponent(SCR_CompartmentAccessComponent));
			if (!compartmentAccess)
				continue;

			compartmentAccess.MoveInVehicleAny(m_Vehicle);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called upon the m_PreferableResupplyTaskRplId replication
	protected void OnPreferableResupplyTaskSet()
	{
		m_PreferableResupplyTask = SCR_ResupplyCampaignMilitaryBaseTaskEntity.Cast(Replication.FindItem(m_PreferableResupplyTaskRplId));
	}

	//------------------------------------------------------------------------------------------------
	protected void SetPreferableResupplyTask(SCR_ResupplyCampaignMilitaryBaseTaskEntity preferableTask)
	{
		if (m_PreferableResupplyTask == preferableTask)
			return;

		m_PreferableResupplyTask = preferableTask;

		m_PreferableResupplyTaskRplId = Replication.FindItemId(m_PreferableResupplyTask);
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetTruckFuel(float percentage)
	{
		if (!m_FuelManager)
			return;

		m_FuelManager.SetTotalFuelPercentage(percentage);
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps the truck from running out of fuel while a member of this group drives it. With no fuel the
	//! AI driver reports a stuck move, which orders the crew out of the truck and fails the task.
	//! Not done when somebody else drives the truck, for example a player.
	protected void TopUpTruckFuelIfLow()
	{
		if (!m_Vehicle || !m_FuelManager)
			return;

		PilotCompartmentSlot pilotSlot = m_Vehicle.GetPilotCompartmentSlot();
		if (!pilotSlot)
			return;

		IEntity driver = pilotSlot.GetOccupant();
		if (!driver)
			return;

		AIAgent driverAgent = SCR_AIUtils.GetAIAgent(driver);
		if (!driverAgent || driverAgent.GetParentGroup() != m_AIGroup)
			return;

		float totalFuel, totalMaxFuel, totalFuelPercentage;
		m_FuelManager.GetTotalValuesOfFuelNodes(totalFuel, totalMaxFuel, totalFuelPercentage);
		if (totalMaxFuel <= 0 || totalFuelPercentage >= TRUCK_FUEL_TOP_UP_THRESHOLD)
			return;

		SetTruckFuel(TRUCK_FUEL_PERCENTAGE);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVehicleDeleted(SCR_AIVehicleUsageComponent vehicleUsageComponent)
	{
		SetVehicle(null);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVehicleAggregatedResourceValueSet()
	{
		if (m_OnVehicleResourcesValueChanged)
			m_OnVehicleResourcesValueChanged.Invoke(m_fVehicleAggregatedResourceValue);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVehicleResourceValueChanged(SCR_ResourceInteractor interactor, float previousValue)
	{
		if (!interactor)
			return;

		m_fVehicleAggregatedResourceValue = interactor.GetAggregatedResourceValue();
		Replication.BumpMe();

		if (m_OnVehicleResourcesValueChanged)
			m_OnVehicleResourcesValueChanged.Invoke(m_fVehicleAggregatedResourceValue);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceConsumer GetVehicleSuppliesConsumer(Vehicle vehicle)
	{
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(vehicle);
		if (!resourceComponent)
			return null;

		return resourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, EResourceType.SUPPLIES);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetVehicle(Vehicle vehicle)
	{
		if (vehicle == m_Vehicle)
			return;

		if (m_Vehicle)
		{
			UpdateGarbageBlacklist(m_Vehicle, false);

			if (m_AiVehicleUsage)
				m_AiVehicleUsage.GetOnDeleted().Remove(OnVehicleDeleted);

			SCR_ResourceConsumer consumer = GetVehicleSuppliesConsumer(m_Vehicle);
			if (consumer)
				consumer.GetOnResourcesChanged().Remove(OnVehicleResourceValueChanged);
		}

		Vehicle previousVehicle = m_Vehicle;
		m_Vehicle = vehicle;
		m_VehicleRplId = Replication.FindItemId(m_Vehicle);
		m_bIsVehicleUsable = m_VehicleConditionManager.IsVehicleUsable(m_Vehicle);

		if (m_Vehicle)
		{
			m_FuelManager = SCR_FuelManagerComponent.Cast(m_Vehicle.FindComponent(SCR_FuelManagerComponent));
			UpdateGarbageBlacklist(m_Vehicle, true);
			m_AiVehicleUsage = SCR_AIVehicleUsageComponent.Cast(m_Vehicle.FindComponent(SCR_AIVehicleUsageComponent));
			if (m_AiVehicleUsage)
				m_AiVehicleUsage.GetOnDeleted().Insert(OnVehicleDeleted);

			SCR_ResourceConsumer consumer = GetVehicleSuppliesConsumer(vehicle);
			if (consumer)
			{
				m_fVehicleAggregatedResourceValue = consumer.GetAggregatedResourceValue();
				consumer.GetOnResourcesChanged().Insert(OnVehicleResourceValueChanged);
			}
		}
		else
		{
			m_fVehicleAggregatedResourceValue = -1;
			m_FuelManager = null;
			m_AiVehicleUsage = null;
		}

		Replication.BumpMe();

		UpdateGroupLifecyclePin();

		if (m_OnVehicleChanged)
			m_OnVehicleChanged.Invoke(vehicle, previousVehicle);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateGarbageBlacklist(notnull Vehicle vehicle, bool blacklisted)
	{
		SCR_GameModeCampaign gameMode = SCR_GameModeCampaign.GetInstance();
		if (gameMode && gameMode.IsProxy())
			return;

		if (m_SuppliesTransportSystem.IsVehicleSelected(vehicle, this))
			return;

		SCR_GarbageSystem garbageSystem = SCR_GarbageSystem.GetByEntityWorld(vehicle);
		if (!garbageSystem)
			return;

		garbageSystem.UpdateBlacklist(vehicle, blacklisted);
	}

	//------------------------------------------------------------------------------------------------
	//! While the group has a usable truck, hold its members materialised (see UpdateGroupLifecyclePin).
	//! The proximity lifecycle in SCR_AIGroup deletes distant members without saving their vehicle
	//! seats, so a despawned crew would later respawn on foot while the blacklisted truck keeps
	//! driving on its own. A permanent LOD pin makes SCR_AIGroup.LifecycleTick hold the group
	//! (HasPermanentLodMember), and CRITICAL importance keeps ChimeraAIWorld eviction from
	//! picking the group when the active-AI budget is full.
	protected void SetGroupLifecyclePinned(bool pinned)
	{
		if (m_bGroupLifecyclePinned == pinned)
			return;

		SCR_GameModeCampaign gameMode = SCR_GameModeCampaign.GetInstance();
		if (gameMode && gameMode.IsProxy())
			return;

		if (!m_AIGroup)
			return;

		m_bGroupLifecyclePinned = pinned;

		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);

		if (pinned)
		{
			m_iImportanceBeforePin = m_AIGroup.GetImportance();
			m_AIGroup.SetImportance(SCR_EAISpawnImportance.CRITICAL);

			foreach (AIAgent agent : agents)
			{
				if (agent)
					agent.SetPermanentLOD(0);
			}

			// Members that materialise later (group was dormant when the task was assigned)
			// must be pinned as well.
			m_AIGroup.GetOnAgentAdded().Insert(OnPinnedAgentAdded);
		}
		else
		{
			m_AIGroup.GetOnAgentAdded().Remove(OnPinnedAgentAdded);

			if (m_iImportanceBeforePin != -1)
			{
				m_AIGroup.SetImportance(m_iImportanceBeforePin);
				m_iImportanceBeforePin = -1;
			}

			foreach (AIAgent agent : agents)
			{
				if (agent)
					agent.SetPermanentLOD(-1);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The group never goes dormant while it has a usable truck, in any state (on a task, returning, or idle).
	//! A crew despawned from a driving truck leaves the truck without a driver, and a dormant idle crew
	//! cannot start a task without an observer nearby. Without a usable truck the crew is on foot and can go dormant.
	protected void UpdateGroupLifecyclePin()
	{
		// Checked on the own vehicle, m_bIsVehicleUsable can come from checks of other nearby trucks (FindVehicle)
		bool hasUsableVehicle = m_Vehicle && m_VehicleConditionManager && m_VehicleConditionManager.IsVehicleUsable(m_Vehicle);
		SetGroupLifecyclePinned(m_eState == SCR_ETransportUnitState.ON_TASK || hasUsableVehicle);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPinnedAgentAdded(AIAgent agent)
	{
		if (agent)
			agent.SetPermanentLOD(0);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetState(SCR_ETransportUnitState state)
	{
		if (m_eState == state)
			return;

		m_eState = state;
		Replication.BumpMe();

		UpdateGroupLifecyclePin();

		if (m_OnStateChanged)
			m_OnStateChanged.Invoke(m_eState);
	}

	//------------------------------------------------------------------------------------------------
	//! \return World position of the group for the map, vector.Zero when not known yet
	vector GetMapPosition()
	{
		return m_vMapPosition;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateMapPosition()
	{
		if (!m_AIGroup)
			return;

		// The group entity origin follows the leader only while the group has a move order,
		// so use the leader entity. It is the vehicle position when the leader sits in the truck.
		vector position;
		IEntity leaderEntity = m_AIGroup.GetLeaderEntity();
		if (leaderEntity)
			position = leaderEntity.GetOrigin();
		else if (m_vMapPosition == vector.Zero)
			position = m_AIGroup.GetOrigin(); // No members (dormant), use the group position until a leader exists
		else
			return; // No members (dormant), keep the last known position

		if (vector.DistanceSq(position, m_vMapPosition) < MAP_POSITION_UPDATE_DISTANCE_SQ)
			return;

		m_vMapPosition = position;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	SCR_ResupplyCampaignMilitaryBaseTaskEntity GetPreferableResupplyTask()
	{
		return m_PreferableResupplyTask;
	}

	//------------------------------------------------------------------------------------------------
	//! Called upon the m_iSourceBaseCallsign replication and set the correct base from the server.
	protected void OnSourceBaseCallsignSet()
	{
		SCR_CampaignMilitaryBaseManager baseManager = SCR_GameModeCampaign.GetInstance().GetBaseManager();
		SCR_CampaignMilitaryBaseComponent base = baseManager.FindBaseByCallsign(m_iSourceBaseCallsign);
		if (base == m_SourceBase)
			return;

		m_SourceBase = base;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDestinationCallsignSet()
	{
		SCR_CampaignMilitaryBaseManager baseManager = SCR_GameModeCampaign.GetInstance().GetBaseManager();
		SCR_CampaignMilitaryBaseComponent suppliedBase = baseManager.FindBaseByCallsign(m_iSuppliedBaseCallsign);
		if (suppliedBase == m_SuppliedBase)
			return;

		m_SuppliedBase = suppliedBase;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateState()
	{
		if (!m_AiGroupConditionManager.IsAIGroupAvailable(m_AIGroup))
		{
			SetState(SCR_ETransportUnitState.UNAVAILABLE);
			return;
		}

		if (!m_SourceBase || !m_PreferableResupplyTask || !m_Vehicle || !m_VehicleConditionManager.IsVehicleUsable(m_Vehicle))
		{
			SetState(SCR_ETransportUnitState.INACTIVE);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnResupplyTaskSolverStateChanged(int solverId, SCR_EResupplyTaskSolverState resupplyTaskSolverState)
	{
		if (m_OnResupplyTaskSolverStateChanged)
			m_OnResupplyTaskSolverStateChanged.Invoke(resupplyTaskSolverState);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskSolverStateChanged(SCR_TaskSolverBase taskSolver, SCR_ETaskSolverState taskSolverState)
	{
		if (taskSolverState == SCR_ETaskSolverState.FAILED || taskSolverState == SCR_ETaskSolverState.COMPLETED)
		{
			SetPreferableResupplyTask(null);
			UpdateState();

			if (taskSolverState == SCR_ETaskSolverState.FAILED)
			{
				m_TransportUnitHelpTaskEntity = CreateTransportUnitHelpTask();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void CreateMoveWaypointToSourceBase()
	{
		if (m_eState != SCR_ETransportUnitState.INACTIVE)
			return;

		m_MoveToSourceBaseWaypoint = m_TaskSolver.CreateMoveWaypointToSourceBase();
		if (!m_MoveToSourceBaseWaypoint)
			return;

		m_AIGroup.GetOnWaypointCompleted().Insert(OnMoveToSourceBaseWaypointCompleted);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_BoardingEntityWaypoint CreateBoardingVehicleWaypoint()
	{
		if (m_eState != SCR_ETransportUnitState.INACTIVE)
			return null;

		return m_TaskSolver.CreateBoardingVehicleWaypoint();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_TransportUnitHelpTaskEntity CreateTransportUnitHelpTask()
	{
		if (!m_TaskSystem)
			return null;

		SCR_Faction faction = SCR_Faction.Cast(GetFaction());
		if (!faction || !faction.IsPlayable())
			return null;

		SCR_GroupTaskManagerComponent groupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();
		if (!groupTaskManager || !groupTaskManager.CanCreateNewTaskWithResourceName(m_sTransportUnitHelpTask, faction))
			return null;

		if (HasTransportUnitHelpTask(faction))
			return null;

		string taskId = string.Format(TRANSPORT_UNIT_HELP_TASK_ID, faction.GetFactionKey(), GenerateTaskID());
		SCR_Task task = m_TaskSystem.CreateTask(m_sTransportUnitHelpTask, taskId, "", "",	m_AIGroup.GetOrigin());
		if (!task)
		{
			Print("Task was not created", LogLevel.ERROR);
			return null;
		}

		m_TaskSystem.SetTaskOwnership(task, SCR_ETaskOwnership.EXECUTOR);
		m_TaskSystem.SetTaskVisibility(task, SCR_ETaskVisibility.GROUP);
		m_TaskSystem.AddTaskFaction(task, faction.GetFactionKey());

		groupTaskManager.SetGroupTask(task, 0);

		SCR_TransportUnitHelpTaskEntity transportUnitHelpTask = SCR_TransportUnitHelpTaskEntity.Cast(task);
		if (!transportUnitHelpTask)
			return null;

		transportUnitHelpTask.SetTransportUnit(this);

		return transportUnitHelpTask;
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasTransportUnitHelpTask(notnull SCR_Faction faction)
	{
		array<SCR_Task> tasks = {};
		SCR_TaskSystem.GetInstance().GetTasksByStateFiltered(
			tasks,
			SCR_ETaskState.CREATED | SCR_ETaskState.ASSIGNED,
			faction.GetFactionKey(),
			-1,
			SCR_TransportUnitHelpTaskEntity
		);

		foreach (SCR_Task theTask : tasks)
		{
			SCR_TransportUnitHelpTaskEntity transportUnitHelpTask = SCR_TransportUnitHelpTaskEntity.Cast(theTask);
			if (!transportUnitHelpTask)
				continue;

			if (transportUnitHelpTask.GetTransportUnit() == this)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected Vehicle FindVehicle()
	{
		m_NearbySupplyTrucks.Clear();
		GetGame().GetWorld().QueryEntitiesBySphere(GetOwner().GetOrigin(), m_fSearchTruckRadius, QuerySupplyTrucks, FilterVehicles, EQueryEntitiesFlags.DYNAMIC);

		return GetClosestVehicle();
	}

	//------------------------------------------------------------------------------------------------
	protected Vehicle GetClosestVehicle()
	{
		float minSqDistanceAny = float.MAX;
		float minSqDistanceOperable = float.MAX;

		Vehicle closestAny, closestOperable;
		foreach (Vehicle vehicle : m_NearbySupplyTrucks)
		{
			float sqDistance = vector.DistanceSq(vehicle.GetOrigin(), m_AIGroup.GetOrigin());

			// Track closest overall
			if (sqDistance < minSqDistanceAny)
			{
				minSqDistanceAny = sqDistance;
				closestAny = vehicle;
			}

			// Track closest operable
			if (m_VehicleConditionManager.IsVehicleUsable(vehicle) && sqDistance < minSqDistanceOperable)
			{
				minSqDistanceOperable = sqDistance;
				closestOperable = vehicle;
			}
		}

		if (closestOperable)
			return closestOperable;

		return closestAny;
	}

	//------------------------------------------------------------------------------------------------
	protected bool FilterVehicles(IEntity entity)
	{
		return entity.IsInherited(Vehicle);
	}

	//------------------------------------------------------------------------------------------------
	protected bool QuerySupplyTrucks(IEntity entity)
	{
		Vehicle vehicle = Vehicle.Cast(entity);
		if (!vehicle)
			return true;

		if (vehicle.m_eVehicleType != EVehicleType.SUPPLY_TRUCK)
			return true;

		if (m_SuppliesTransportSystem && m_SuppliesTransportSystem.IsVehicleUsed(vehicle, this))
			return true;

		m_NearbySupplyTrucks.Insert(vehicle);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ResupplyCampaignMilitaryBaseTaskEntity SelectPreferableResupplyTask()
	{
		array<SCR_Task> resupplyTasks = {};
		SCR_TaskSystem.GetInstance().GetTasksByStateFiltered(resupplyTasks, SCR_ETaskState.CREATED | SCR_ETaskState.ASSIGNED, GetFaction().GetFactionKey(), -1, SCR_ResupplyCampaignMilitaryBaseTaskEntity);

		float maxScarcityLevel = -float.MAX;
		int maxPriorityLevel = int.MAX;
		SCR_ResupplyCampaignMilitaryBaseTaskEntity resupplyTask;
		SCR_ResupplyCampaignMilitaryBaseTaskEntity preferableResupplyTask;
		SCR_CampaignMilitaryBaseComponent militaryBase;
		foreach (SCR_Task task : resupplyTasks)
		{
			if (task.GetTaskState() != SCR_ETaskState.CREATED)
				continue;

			if (m_SuppliesTransportSystem && m_SuppliesTransportSystem.IsTaskSolved(task, this))
				continue;

			resupplyTask = SCR_ResupplyCampaignMilitaryBaseTaskEntity.Cast(task);
			if (!resupplyTask)
				continue;

			if (m_SourceBase == resupplyTask.GetMilitaryBase())
				continue;

			// Tasks with the highest priority are preferred
			// If more tasks have the highest priority, tasks with the highest scarcity are preferred
			militaryBase = resupplyTask.GetMilitaryBase();
			if (!militaryBase)
				continue;

			int baseResupplyPriority = resupplyTask.GetMilitaryBase().GetSupplyRequestExecutionPriority();
			if (baseResupplyPriority < maxPriorityLevel)
			{
				maxPriorityLevel = baseResupplyPriority;
				preferableResupplyTask = resupplyTask;
				maxScarcityLevel = resupplyTask.GetMilitaryBase().GetScarcityLevel();
			}
			else if (baseResupplyPriority == maxPriorityLevel)
			{
				float baseScarcityLevel = resupplyTask.GetMilitaryBase().GetScarcityLevel();
				if (baseScarcityLevel > maxScarcityLevel)
				{
					maxScarcityLevel = baseScarcityLevel;
					preferableResupplyTask = resupplyTask;
				}
			}
		}

		return preferableResupplyTask;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetSourceBaseFromEntityName()
	{
		if (m_sSourceBaseEntityName.IsEmpty())
			return;

		IEntity sourceBaseEntity = GetGame().GetWorld().FindEntityByName(m_sSourceBaseEntityName);
		if (!sourceBaseEntity)
			return;

		m_SourceBase = SCR_CampaignMilitaryBaseComponent.Cast(sourceBaseEntity.FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (!m_SourceBase)
			return;

		m_iSourceBaseCallsign = m_SourceBase.GetCallsign();
	}

	//------------------------------------------------------------------------------------------------
	protected void SendTransportGroupLostNotificationToCommander()
	{
		if (!m_AIGroup)
			return;

		SCR_Faction faction = SCR_Faction.Cast(m_AIGroup.GetFaction());
		if (!faction)
			return;

		if (SCR_PlayerController.GetLocalPlayerId() == faction.GetCommanderId())
			SCR_NotificationsComponent.SendLocal(ENotification.CAMPAIGN_TRANSPORT_GROUP_LOST, Replication.FindItemId(GetOwner()));
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		SetSourceBaseFromEntityName();

		m_SuppliesTransportSystem = SCR_SuppliesTransportSystem.GetInstance();
		if (!m_SuppliesTransportSystem)
			return;

		m_TaskSolverManager = SCR_TaskSolverManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_TaskSolverManagerComponent));
		if (!m_TaskSolverManager)
			return;

		m_TaskSolver = SCR_ResupplyTaskSolver.Cast(m_TaskSolverManager.CreateSolver(SCR_ResupplyTaskSolver));
		if (!m_TaskSolver)
			return;

		m_TaskSolver.SetAiGroup(m_AIGroup);

		m_TaskSolver.GetOnStateChanged().Insert(OnTaskSolverStateChanged);
		m_TaskSolver.GetOnResupplyTaskSolverStateChanged().Insert(OnResupplyTaskSolverStateChanged);

		m_VehicleConditionManager = m_TaskSolver.GetVehicleConditionManager();
		m_VehicleConditionManager.GetOnVehicleUsabilityChanged().Insert(OnVehicleUsabilityChanged);
		m_AiGroupConditionManager = m_TaskSolver.GetAiGroupConditionManager();

		m_TaskSystem = SCR_TaskSystem.GetInstance();

		m_SuppliesTransportSystem.Register(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnIsVehicleUsableSet()
	{
		if (m_OnVehicleChanged)
			m_OnVehicleChanged.Invoke(m_Vehicle, null);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVehicleUsabilityChanged(bool isUsable)
	{
		if (m_bIsVehicleUsable == isUsable)
			return;

		m_bIsVehicleUsable = isUsable;
		m_eVehicleFailedConditionType = SCR_EVehicleConditionCheckType.MISSING_VEHICLE;

		if (!m_bIsVehicleUsable)
		{
			SCR_VehicleConditionCheck failedCondition = m_VehicleConditionManager.GetFirstFailedCheck(m_Vehicle);
			if (failedCondition)
				m_eVehicleFailedConditionType = failedCondition.GetConditionType();
		}

		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;

		super.OnPostInit(owner);

		SetEventMask(owner, EntityEvent.INIT);

		SetState(SCR_ETransportUnitState.INACTIVE);

		m_AIGroup = SCR_AIGroup.Cast(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		SetGroupLifecyclePinned(false);

		if (m_TaskSolver)
		{
			m_TaskSolver.GetOnStateChanged().Remove(OnTaskSolverStateChanged);
			m_TaskSolver.GetOnResupplyTaskSolverStateChanged().Remove(OnResupplyTaskSolverStateChanged);
		}

		if (m_TaskSolverManager)
			m_TaskSolverManager.RemoveSolver(m_TaskSolver);

		SCR_ResourceConsumer consumer = GetVehicleSuppliesConsumer(m_Vehicle);
		if (consumer)
			consumer.GetOnResourcesChanged().Remove(OnVehicleResourceValueChanged);

		if (m_VehicleConditionManager)
			m_VehicleConditionManager.GetOnVehicleUsabilityChanged().Remove(OnVehicleUsabilityChanged);

		if (m_AiVehicleUsage)
			m_AiVehicleUsage.GetOnDeleted().Remove(OnVehicleDeleted);

		if (m_SuppliesTransportSystem)
			m_SuppliesTransportSystem.Unregister(this);

		SCR_GameModeCampaign gameMode = SCR_GameModeCampaign.GetInstance();
		if (gameMode && !gameMode.IsProxy())
		{
			if (m_TaskSystem && m_TransportUnitHelpTaskEntity)
				m_TaskSystem.DeleteTask(m_TransportUnitHelpTaskEntity);

			if (m_Vehicle)
				UpdateGarbageBlacklist(m_Vehicle, false);
		}

		SendTransportGroupLostNotificationToCommander();

		super.OnDelete(owner);
	}
}
