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

	[RplProp(onRplName: "OnVehicleRplIdSet")]
	protected RplId m_VehicleRplId = RplId.Invalid();

	[RplProp(onRplName: "OnPreferableResupplyTaskSet")]
	protected RplId m_PreferableResupplyTaskRplId = RplId.Invalid();

	protected SCR_AIGroup m_AIGroup;

	protected SCR_ResupplyCampaignMilitaryBaseTaskEntity m_PreferableResupplyTask;

	protected ref set<Vehicle> m_NearbySupplyTrucks = new set<Vehicle>();

	protected SCR_SuppliesTransportSystem m_SuppliesTransportSystem;

	protected SCR_TaskSolverManagerComponent m_TaskSolverManager;

	protected SCR_CampaignMilitaryBaseComponent m_SourceBase;

	protected SCR_CampaignMilitaryBaseComponent m_SuppliedBase;

	protected Vehicle m_Vehicle;

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

	protected static int s_iLastGeneratedTaskId;

	protected static const string TRANSPORT_UNIT_HELP_TASK_ID = "%1_TransportUnitHelpTask_%2";

	//------------------------------------------------------------------------------------------------
	SCR_VehicleConditionManager GetVehicleConditionManager()
	{
		return m_VehicleConditionManager;
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
	EntityID GetID()
	{
		if (m_AIGroup)
			return m_AIGroup.GetID();

		return EntityID.INVALID;
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

		Replication.BumpMe();
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

		SetTruckFuel(0.75);

		SetState(SCR_ETransportUnitState.ON_TASK);

		if (m_bTeleportGroupToVehicle)
		{
			TeleportGroupToVehicle();
		}
	}

	//------------------------------------------------------------------------------------------------
	Vehicle GetVehicle()
	{
		if (!m_Vehicle && m_VehicleRplId.IsValid())
			OnVehicleRplIdSet();

		return m_Vehicle;
	}

	bool IsVehicleUsable()
	{
		if (!m_VehicleConditionManager)
			return true;

		return m_VehicleConditionManager.IsVehicleUsable(m_Vehicle);
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

		if (GetState() != SCR_ETransportUnitState.ON_TASK)
		{
			SetPreferableResupplyTask(SelectPreferableResupplyTask());
			if (m_PreferableResupplyTask)
					SetSuppliedBase(m_PreferableResupplyTask.GetMilitaryBase());

			if (!m_Vehicle || !m_VehicleConditionManager.IsVehicleUsable(m_Vehicle))
			{
				SetVehicle(FindVehicle());
			}
		}

		SCR_ETransportUnitState previousState = GetState();
		UpdateState();
	}

	//------------------------------------------------------------------------------------------------
	void ReturnToSourceBaseIfNeeded()
	{
		if (m_eState != SCR_ETransportUnitState.INACTIVE || m_TaskSolver.GetResupplyTaskSolverState() != SCR_EResupplyTaskSolverState.INACTIVE)
			return;

		if (!m_SourceBase || !m_Vehicle)
			return;

		if (!m_VehicleConditionManager.IsVehicleUsable(m_Vehicle))
			return;

		SCR_AIVehicleUsageComponent aiVehicleUSage = SCR_AIVehicleUsageComponent.Cast(m_Vehicle.FindComponent(SCR_AIVehicleUsageComponent));
		if (!aiVehicleUSage)
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

		if (aiVehicleUSage.IsOccupiedByGroup(m_AIGroup))
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

		if (!m_Vehicle)
			return;

		SCR_AIVehicleUsageComponent aiVehicleUSage = SCR_AIVehicleUsageComponent.Cast(m_Vehicle.FindComponent(SCR_AIVehicleUsageComponent));
		if (!aiVehicleUSage)
			return;

		if (aiVehicleUSage.IsOccupiedByGroup(m_AIGroup))
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

		m_PreferableResupplyTaskRplId = Replication.FindId(m_PreferableResupplyTask);
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
	//! Called upon the m_VehicleRplId replication and set the vehicle from the server.
	protected void OnVehicleRplIdSet()
	{
		if (!m_VehicleRplId.IsValid())
			return;

		Vehicle vehicle = Vehicle.Cast(Replication.FindItem(m_VehicleRplId));
		SetVehicle(vehicle);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetVehicle(Vehicle vehicle)
	{
		if (vehicle == m_Vehicle)
			return;

		if (m_Vehicle)
			UpdateGarbageBlacklist(m_Vehicle, false);

		Vehicle previousVehicle = m_Vehicle;
		m_Vehicle = vehicle;
		m_VehicleRplId = Replication.FindId(m_Vehicle);
		Replication.BumpMe();

		if (m_Vehicle)
		{
			m_FuelManager = SCR_FuelManagerComponent.Cast(m_Vehicle.FindComponent(SCR_FuelManagerComponent));

			UpdateGarbageBlacklist(m_Vehicle, true);
		}
		else
		{
			m_FuelManager = null;
		}

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
	protected void SetState(SCR_ETransportUnitState state)
	{
		if (m_eState == state)
			return;

		m_eState = state;
		Replication.BumpMe();

		if (m_OnStateChanged)
			m_OnStateChanged.Invoke(m_eState);
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
			SCR_NotificationsComponent.SendLocal(ENotification.CAMPAIGN_TRANSPORT_GROUP_LOST, Replication.FindId(GetOwner()));
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
		m_AiGroupConditionManager = m_TaskSolver.GetAiGroupConditionManager();

		m_TaskSystem = SCR_TaskSystem.GetInstance();

		m_SuppliesTransportSystem.Register(this);
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
		if (m_TaskSolver)
		{
			m_TaskSolver.GetOnStateChanged().Remove(OnTaskSolverStateChanged);
			m_TaskSolver.GetOnResupplyTaskSolverStateChanged().Remove(OnResupplyTaskSolverStateChanged);
		}

		if (m_TaskSolverManager)
			m_TaskSolverManager.RemoveSolver(m_TaskSolver);

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
