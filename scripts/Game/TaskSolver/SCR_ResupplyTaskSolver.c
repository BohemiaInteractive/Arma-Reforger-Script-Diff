class SCR_ResupplyTaskSolver : SCR_TaskSolverBase
{
	protected SCR_EResupplyTaskSolverState m_eResupplySolverState = SCR_EResupplyTaskSolverState.INACTIVE;
	protected SCR_AIGroupUtilityComponent m_TransportGroupUtility;
	protected Vehicle m_Vehicle;
	protected SCR_AIVehicleUsageComponent m_AiVehicleUsage;
	protected EAIGroupCombatMode m_ePreviousGroupCombatMode;
	protected SCR_BoardingEntityWaypoint m_BoardingWaypoint;
	protected SCR_AIWaypoint m_MoveToSuppliedBaseWaypoint;
	protected SCR_AIWaypoint m_MoveToSourceBaseWaypoint;
	protected SCR_LoadSuppliesWaypoint m_LoadSuppliesWaypoint;
	protected SCR_UnloadSuppliesWaypoint m_UnloadSuppliesWaypoint;
	protected SCR_ResupplyCampaignMilitaryBaseTaskEntity m_ResupplyTask;
	protected SCR_CampaignMilitaryBaseComponent m_SourceBase;

	protected ref SCR_VehicleConditionManager m_VehicleConditionManager;
	protected ref SCR_AIGroupConditionManager m_AiGroupConditionManager;
	protected ref SCR_ResupplyTaskSolverEntry m_ResupplyTaskSolverEntry;
	protected ref ScriptInvokerInt2 m_OnResupplyTaskSolverStateChanged;

	//------------------------------------------------------------------------------------------------
	override void SetAiGroup(SCR_AIGroup aiGroup)
	{
		super.SetAiGroup(aiGroup);

		SCR_VehiclePilotVacancyCheck pilotVacancyCheck = SCR_VehiclePilotVacancyCheck.Cast(m_VehicleConditionManager.GetCheck(SCR_VehiclePilotVacancyCheck));
		if (pilotVacancyCheck)
			pilotVacancyCheck.SetIgnoredAiGroup(m_AiGroup);

		if (m_TransportGroupUtility)
			m_TransportGroupUtility.GetOnMoveFailed().Remove(OnWaypointFailed);

		m_TransportGroupUtility = SCR_AIGroupUtilityComponent.Cast(m_AiGroup.FindComponent(SCR_AIGroupUtilityComponent));
		m_TransportGroupUtility.GetOnMoveFailed().Insert(OnWaypointFailed);
	}

	//------------------------------------------------------------------------------------------------
	//! Configures this solver with a resupply solver entry.
	void SetResupplyTaskSolverEntry(SCR_ResupplyTaskSolverEntry entry)
	{
		m_ResupplyTaskSolverEntry = entry;
		if (!m_ResupplyTaskSolverEntry)
			return;

		array<ref SCR_ConditionCheckUIEntry> conditionCheckUIEntries = m_ResupplyTaskSolverEntry.GetConditionCheckUIEntries();
		foreach (SCR_ConditionCheckUIEntry uiEntry : conditionCheckUIEntries)
		{
			SCR_VehicleConditionCheck vehicleCheck = m_VehicleConditionManager.GetCheck(uiEntry.GetConditionCheckTypename().ToType());
			if (!vehicleCheck)
				continue;

			vehicleCheck.SetFailedConditionUIInfo(uiEntry.GetFailedConditionUIInfo());
		}
	}

	//------------------------------------------------------------------------------------------------
	SCR_VehicleConditionManager GetVehicleConditionManager()
	{
		return m_VehicleConditionManager;
	}

	//------------------------------------------------------------------------------------------------
	SCR_AIGroupConditionManager GetAiGroupConditionManager()
	{
		return m_AiGroupConditionManager;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt2 GetOnResupplyTaskSolverStateChanged()
	{
		if (!m_OnResupplyTaskSolverStateChanged)
			m_OnResupplyTaskSolverStateChanged = new ScriptInvokerInt2();

		return m_OnResupplyTaskSolverStateChanged;
	}

	//------------------------------------------------------------------------------------------------
	SCR_EResupplyTaskSolverState GetResupplyTaskSolverState()
	{
		return m_eResupplySolverState;
	}

	//------------------------------------------------------------------------------------------------
	//! Reset solver state to initial (inactive)
	void ResetToInitialState()
	{
		SetResupplyTaskSolverState(SCR_EResupplyTaskSolverState.INACTIVE);
	}

	//------------------------------------------------------------------------------------------------
	void SetResupplyTaskSolverState(SCR_EResupplyTaskSolverState state)
	{
		if (state == m_eResupplySolverState)
			return;

		string prevState = SCR_Enum.GetEnumName(SCR_EResupplyTaskSolverState, m_eResupplySolverState);
		string curState = SCR_Enum.GetEnumName(SCR_EResupplyTaskSolverState, state);

		PrintFormat("SCR_ResupplyTaskSolver: Transport group %1 changed state from %2 to %3.", GetTransportGroupName(), prevState, curState, level: LogLevel.DEBUG);

		m_eResupplySolverState = state;

		if (m_OnResupplyTaskSolverStateChanged)
			m_OnResupplyTaskSolverStateChanged.Invoke(m_iId, m_eResupplySolverState);
	}

	//------------------------------------------------------------------------------------------------
	//! Initialize solver with task and context. Registers events.
	override void Init(SCR_Task task, SCR_TaskSolverContext context)
	{
		super.Init(task, context);

		m_ResupplyTask = SCR_ResupplyCampaignMilitaryBaseTaskEntity.Cast(task);
		SCR_ResupplyTaskSolverContext resupplyTaskContext = SCR_ResupplyTaskSolverContext.Cast(context);
		SetVehicle(resupplyTaskContext.GetVehicle());

		if (!m_Vehicle)
			return;

		m_AiVehicleUsage = SCR_AIVehicleUsageComponent.Cast(m_Vehicle.FindComponent(SCR_AIVehicleUsageComponent));

		m_SourceBase = resupplyTaskContext.GetSourceBase();
	}

	//------------------------------------------------------------------------------------------------
	//! Start the task-solving process, branching to appropriate initial action.
	override void StartTaskSolving()
	{
		super.StartTaskSolving();

		if (!m_VehicleConditionManager.IsVehicleUsable(m_Vehicle))
		{
			FailResupplyTaskSolving(SCR_EResupplyTaskSolverErrorState.VEHICLE_NOT_OPERABLE);
			return;
		}

		if (!m_AiGroupConditionManager.IsAIGroupAvailable(m_AiGroup))
		{
			FailResupplyTaskSolving(SCR_EResupplyTaskSolverErrorState.GROUP_UNAVAILABLE);
			return;
		}

		CompleteAllWaypoints();
		
		m_ePreviousGroupCombatMode = m_TransportGroupUtility.GetCombatModeExternal();
		m_TransportGroupUtility.SetCombatMode(m_ResupplyTaskSolverEntry.GetGroupCombatMode());

		if (m_AiVehicleUsage.IsOccupiedByGroup(m_AiGroup))
		{
			CreateMoveWaypointToSourceOrSuppliedBase();
		}
		else
		{
			m_BoardingWaypoint = CreateBoardingVehicleWaypoint();
			AddOnBoardingCompletedHandler();
		}
	}

	//------------------------------------------------------------------------------------------------
	SCR_BoardingEntityWaypoint CreateBoardingVehicleWaypoint()
	{
		SCR_BoardingEntityWaypoint boardingWaypoint = SpawnBoardingWaypoint();
		if (!boardingWaypoint)
		{
			FailResupplyTaskSolving();
			return null;
		}

		boardingWaypoint.SetEntity(m_Vehicle);
		SetResupplyTaskSolverState(SCR_EResupplyTaskSolverState.BOARDING_VEHICLE);
		PreventMaxLOD(m_AiGroup);
		m_AiGroup.AddWaypoint(boardingWaypoint);
		m_BoardingWaypoint = boardingWaypoint;

		return boardingWaypoint;
	}

	//------------------------------------------------------------------------------------------------
	SCR_AIWaypoint CreateMoveWaypointToSourceBase()
	{
		if (!m_SourceBase)
			return null;

		return CreateMoveWaypoint(SCR_EResupplyTaskSolverState.MOVING_TO_SOURCE_BASE, m_SourceBase.GetOwner().GetOrigin(), m_SourceBase.GetRadius());
	}

	//------------------------------------------------------------------------------------------------
	void SetSourceBase(SCR_CampaignMilitaryBaseComponent sourceBase)
	{
		if (m_SourceBase == sourceBase)
			return;

		m_SourceBase = sourceBase;
	}

	//------------------------------------------------------------------------------------------------
	void SetVehicle(Vehicle vehicle)
	{
		if (m_Vehicle == vehicle)
			return;

		m_Vehicle = vehicle;
	}

	//------------------------------------------------------------------------------------------------
	//! Periodic update for resource/availability checks.
	override void Update(float timeSlice)
	{
		if (m_eResupplySolverState == SCR_EResupplyTaskSolverState.INACTIVE)
			return;

		if (!m_VehicleConditionManager.IsVehicleUsable(m_Vehicle))
			FailResupplyTaskSolving(SCR_EResupplyTaskSolverErrorState.VEHICLE_NOT_OPERABLE);

		if (!m_AiGroupConditionManager.IsAIGroupAvailable(m_AiGroup))
			FailResupplyTaskSolving(SCR_EResupplyTaskSolverErrorState.GROUP_UNAVAILABLE);
	}

	//------------------------------------------------------------------------------------------------
	//! Mark solver as failed, clean up, and provide error reporting.
	protected void FailResupplyTaskSolving(SCR_EResupplyTaskSolverErrorState errorState = SCR_EResupplyTaskSolverErrorState.DEFAULT)
	{
		if (m_AiGroup)
		{
			AllowMaxLOD(m_AiGroup);
			m_AiGroup.GetOnWaypointCompleted().Clear();
			m_TransportGroupUtility.SetCombatMode(m_ePreviousGroupCombatMode);
		}

		CompleteAllWaypoints();

		string errorStateName = SCR_Enum.GetEnumName(SCR_EResupplyTaskSolverErrorState, errorState);
		PrintFormat("SCR_ResupplyTaskSolver: Group %1 failed task solving: %2.", GetTransportGroupName(), errorStateName, level: LogLevel.DEBUG);

		if (m_Vehicle && m_Vehicle.IsOccupied() && errorState == SCR_EResupplyTaskSolverErrorState.VEHICLE_NOT_OPERABLE)
		{
			SCR_BoardingWaypoint getOutWaypoint = SpawnGetOutWaypoint();
			m_AiGroup.AddWaypoint(getOutWaypoint);
		}

		SetResupplyTaskSolverState(SCR_EResupplyTaskSolverState.INACTIVE);

		super.FailTaskSolving();
	}

	//------------------------------------------------------------------------------------------------
	//! Mark solver as completed with cleanup.
	override protected void CompleteTaskSolving()
	{
		if (m_AiGroup)
		{
			AllowMaxLOD(m_AiGroup);
			m_AiGroup.GetOnWaypointCompleted().Clear();
			m_TransportGroupUtility.SetCombatMode(m_ePreviousGroupCombatMode);
		}

		SetResupplyTaskSolverState(SCR_EResupplyTaskSolverState.INACTIVE);

		super.CompleteTaskSolving();
	}

	//------------------------------------------------------------------------------------------------
	// Helper for debug or log output for the AI group
	protected string GetTransportGroupName()
	{
		if (!m_AiGroup)
			return string.Empty;

		string company, platoon, squad, character, format;
		m_AiGroup.GetCallsigns(company, platoon, squad, character, format);
		return string.Format(format, company, platoon, squad, character);
	}

	//------------------------------------------------------------------------------------------------
	protected void PreventMaxLOD(SCR_AIGroup aiGroup)
	{
		if (!aiGroup)
			return;

		array<AIAgent> aiAgents = {};
		aiGroup.GetAgents(aiAgents);

		int nextToLastLOD = AIAgent.GetMaxLOD() - 1;

		foreach (AIAgent aiAgent : aiAgents)
		{
			if (aiAgent.GetLOD() == AIAgent.GetMaxLOD())
				aiAgent.SetLOD(nextToLastLOD);

			aiAgent.PreventMaxLOD();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AllowMaxLOD(SCR_AIGroup aiGroup)
	{
		if (!aiGroup)
			return;

		array<AIAgent> aiAgents = {};
		aiGroup.GetAgents(aiAgents);

		foreach (AIAgent aiAgent : aiAgents)
		{
			aiAgent.AllowMaxLOD();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddOnBoardingCompletedHandler()
	{
		if (m_AiGroup && m_BoardingWaypoint)
			m_AiGroup.GetOnWaypointCompleted().Insert(OnBoardingCompleted);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveOnBoardingCompletedHandler()
	{
		if (m_AiGroup && m_BoardingWaypoint)
			m_AiGroup.GetOnWaypointCompleted().Remove(OnBoardingCompleted);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddOnLoadSuppliesCompletedHandler()
	{
		if (m_AiGroup && m_LoadSuppliesWaypoint)
			m_AiGroup.GetOnWaypointCompleted().Insert(OnLoadSuppliesCompleted);
	}

	protected void RemoveOnLoadSuppliesCompletedHandler()
	{
		if (m_AiGroup && m_LoadSuppliesWaypoint)
			m_AiGroup.GetOnWaypointCompleted().Remove(OnLoadSuppliesCompleted);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddOnMoveToSuppliedBaseCompletedHandler()
	{
		if (m_AiGroup && m_MoveToSuppliedBaseWaypoint)
			m_AiGroup.GetOnWaypointCompleted().Insert(OnMoveToSuppliedBaseCompleted);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveOnMoveToSuppliedBaseCompletedHandler()
	{
		if (m_AiGroup && m_MoveToSuppliedBaseWaypoint)
			m_AiGroup.GetOnWaypointCompleted().Remove(OnMoveToSuppliedBaseCompleted);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddOnMoveToSourceBaseCompletedHandler()
	{
		if (m_AiGroup && m_MoveToSourceBaseWaypoint)
			m_AiGroup.GetOnWaypointCompleted().Insert(OnMoveToSourceBaseCompleted);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveOnMoveToSourceBaseCompletedHandler()
	{
		if (m_AiGroup && m_MoveToSourceBaseWaypoint)
			m_AiGroup.GetOnWaypointCompleted().Remove(OnMoveToSourceBaseCompleted);
	}

	protected void AddOnUnloadSuppliesCompletedHandler()
	{
		if (m_AiGroup && m_UnloadSuppliesWaypoint)
			m_AiGroup.GetOnWaypointCompleted().Insert(OnUnloadSuppliesCompleted);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveOnUnloadSuppliesCompletedHandler()
	{
		if (m_AiGroup && m_UnloadSuppliesWaypoint)
			m_AiGroup.GetOnWaypointCompleted().Remove(OnUnloadSuppliesCompleted);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceContainer GetVehicleSuppliesContainer()
	{
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(m_Vehicle);
		if (!resourceComponent)
			return null;

		return resourceComponent.GetContainer(EResourceType.SUPPLIES);
	}

	//------------------------------------------------------------------------------------------------
	protected void CompleteTransportGroupCurrentWaypoint()
	{
		if (!m_AiGroup)
			return;

		AIWaypoint currentWaypoint = m_AiGroup.GetCurrentWaypoint();

		if (currentWaypoint)
			m_AiGroup.CompleteWaypoint(currentWaypoint);
	}

	//------------------------------------------------------------------------------------------------
	protected void CompleteAllWaypoints()
	{
		if (!m_AiGroup)
			return;

		m_AiGroup.CompleteAllWaypoints();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWaypointFailed(int moveResult, IEntity vehicleUsed, bool isWaypointRelated, vector moveLocation)
	{
		if (moveResult == EMoveError.OK)
			return;

		FailResupplyTaskSolving(SCR_EResupplyTaskSolverErrorState.MOVE_WAYPOINT_FAILED);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_AIWaypoint CreateMoveWaypoint(SCR_EResupplyTaskSolverState resupplySolverState, vector waypointPosition, float waypointCompletionRadius)
	{
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld)
		{
			FailResupplyTaskSolving();
			return null;
		}

		RoadNetworkManager roadNetworkManager = aiWorld.GetRoadNetworkManager();
		if (!roadNetworkManager)
		{
			FailResupplyTaskSolving();
			return null;
		}

		vector reachableWaypointPosition;

		if (!roadNetworkManager.GetReachableWaypointInRoad(m_AiGroup.GetOrigin(), waypointPosition, waypointCompletionRadius, reachableWaypointPosition))
			reachableWaypointPosition = waypointPosition;

		SCR_AIWaypoint waypoint = SpawnMoveWaypoint(reachableWaypointPosition);
		if (!waypoint)
		{
			FailResupplyTaskSolving();
			return null;
		}

		waypoint.SetCompletionRadius(waypointCompletionRadius - vector.Distance(reachableWaypointPosition, waypointPosition));
		SetResupplyTaskSolverState(resupplySolverState);
		m_AiGroup.AddWaypoint(waypoint);

		return waypoint;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_LoadSuppliesWaypoint CreateLoadSuppliesWaypoint()
	{
		SCR_LoadSuppliesWaypoint waypoint = SCR_LoadSuppliesWaypoint.Cast(SpawnLoadSupplyWaypoint());
		if (!waypoint)
		{
			FailResupplyTaskSolving();
			return null;
		}

		waypoint.SetBase(m_SourceBase);
		SetResupplyTaskSolverState(SCR_EResupplyTaskSolverState.LOADING_SUPPLIES);
		m_AiGroup.AddWaypoint(waypoint);
		m_LoadSuppliesWaypoint = waypoint;
		AddOnLoadSuppliesCompletedHandler();

		return waypoint;
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateMoveWaypointToSourceOrSuppliedBase()
	{
		SCR_ResourceContainer container = GetVehicleSuppliesContainer();
		if (!container || !m_SourceBase)
		{
			FailResupplyTaskSolving();
			return;
		}

		float sourceBaseRadius = m_SourceBase.GetRadius();
		bool isCloseToSourceBase = vector.DistanceSq(m_SourceBase.GetOwner().GetOrigin(), m_AiGroup.GetOrigin()) < (sourceBaseRadius * sourceBaseRadius);
		if (isCloseToSourceBase)
		{
			CreateLoadSuppliesWaypoint();
			return;
		}

		float normalizedResource = container.GetResourceValue() / container.GetMaxResourceValue();
		if (normalizedResource > m_ResupplyTaskSolverEntry.GetResourceValueThreshold())
		{
			m_MoveToSuppliedBaseWaypoint = CreateMoveWaypoint(
				SCR_EResupplyTaskSolverState.MOVING_TO_SUPPLIED_BASE,
				m_ResupplyTask.GetOrigin(),
				m_ResupplyTask.GetMilitaryBase().GetRadius()
			);

			AddOnMoveToSuppliedBaseCompletedHandler();
			return;
		}
		else
		{
			m_MoveToSourceBaseWaypoint = CreateMoveWaypoint(
				SCR_EResupplyTaskSolverState.MOVING_TO_SOURCE_BASE,
				m_SourceBase.GetOwner().GetOrigin(),
				m_SourceBase.GetRadius()
			);

			AddOnMoveToSourceBaseCompletedHandler();
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBoardingCompleted(AIWaypoint waypoint)
	{
		if (waypoint != m_BoardingWaypoint)
			return;

		RemoveOnBoardingCompletedHandler();
		CreateMoveWaypointToSourceOrSuppliedBase();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMoveToSourceBaseCompleted(SCR_AIWaypoint waypoint)
	{
		if (waypoint != m_MoveToSourceBaseWaypoint)
			return;

		RemoveOnMoveToSourceBaseCompletedHandler();
		CreateLoadSuppliesWaypoint();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLoadSuppliesCompleted(SCR_AIWaypoint waypoint)
	{
		if (waypoint != m_LoadSuppliesWaypoint)
			return;

		RemoveOnLoadSuppliesCompletedHandler();

		m_MoveToSuppliedBaseWaypoint = CreateMoveWaypoint(
			SCR_EResupplyTaskSolverState.MOVING_TO_SUPPLIED_BASE,
			m_ResupplyTask.GetOrigin(),
			m_ResupplyTask.GetMilitaryBase().GetRadius()
		);

		AddOnMoveToSuppliedBaseCompletedHandler();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMoveToSuppliedBaseCompleted(SCR_AIWaypoint waypoint)
	{
		if (waypoint != m_MoveToSuppliedBaseWaypoint)
			return;

		RemoveOnMoveToSuppliedBaseCompletedHandler();

		m_UnloadSuppliesWaypoint = SpawnUnloadSupplyWaypoint();
		if (!m_UnloadSuppliesWaypoint)
		{
			FailResupplyTaskSolving();
			return;
		}

		m_UnloadSuppliesWaypoint.SetBase(m_ResupplyTask.GetMilitaryBase());
		SetResupplyTaskSolverState(SCR_EResupplyTaskSolverState.UNLOADING_SUPPLIES);
		m_AiGroup.AddWaypoint(m_UnloadSuppliesWaypoint);
		AddOnUnloadSuppliesCompletedHandler();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUnloadSuppliesCompleted(SCR_AIWaypoint waypoint)
	{
		if (waypoint != m_UnloadSuppliesWaypoint)
			return;

		RemoveOnUnloadSuppliesCompletedHandler();

		m_MoveToSourceBaseWaypoint = CreateMoveWaypoint(
			SCR_EResupplyTaskSolverState.MOVING_TO_SOURCE_BASE,
			m_SourceBase.GetOwner().GetOrigin(),
			m_SourceBase.GetRadius()
		);

		AddOnMoveToSourceBaseCompletedHandler();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_AIWaypoint SpawnWaypoint(Resource waypointPrefab, vector origin)
	{
		if (!waypointPrefab.IsValid())
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;

		float surfaceY = GetGame().GetWorld().GetSurfaceY(origin[0], origin[2]);
		origin[1] = surfaceY;
		spawnParams.Transform[3] = origin;

		return SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefab(waypointPrefab, GetGame().GetWorld(), spawnParams));
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_BoardingWaypoint SpawnGetOutWaypoint()
	{
		if (!m_ResupplyTaskSolverEntry)
			return null;

		Resource prefab = Resource.Load(m_ResupplyTaskSolverEntry.GetGetOutWaypointPrefabResourceName());
		return SCR_BoardingWaypoint.Cast(SpawnWaypoint(prefab, m_Vehicle.GetOrigin()));
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_BoardingEntityWaypoint SpawnBoardingWaypoint()
	{
		if (!m_ResupplyTaskSolverEntry)
			return null;

		Resource prefab = Resource.Load(m_ResupplyTaskSolverEntry.GetGetInWaypointPrefabResourceName());
		return SCR_BoardingEntityWaypoint.Cast(SpawnWaypoint(prefab, m_Vehicle.GetOrigin()));
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_AIWaypoint SpawnMoveWaypoint(vector position)
	{
		if (!m_ResupplyTaskSolverEntry)
			return null;

		Resource prefab = Resource.Load(m_ResupplyTaskSolverEntry.GetMoveWaypointPrefabResourceName());
		return SpawnWaypoint(prefab, position);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_AIWaypoint SpawnLoadSupplyWaypoint()
	{
		if (!m_ResupplyTaskSolverEntry)
			return null;

		Resource prefab = Resource.Load(m_ResupplyTaskSolverEntry.GetLoadSuppliesWaypointPrefabResourceName());
		return SpawnWaypoint(prefab, m_AiGroup.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_UnloadSuppliesWaypoint SpawnUnloadSupplyWaypoint()
	{
		if (!m_ResupplyTaskSolverEntry)
			return null;

		Resource prefab = Resource.Load(m_ResupplyTaskSolverEntry.GetUnloadSuppliesWaypointPrefabResourceName());
		return SCR_UnloadSuppliesWaypoint.Cast(SpawnWaypoint(prefab, m_AiGroup.GetOrigin()));
	}

	//------------------------------------------------------------------------------------------------
	void SCR_ResupplyTaskSolver()
	{
		m_VehicleConditionManager = new SCR_VehicleConditionManager();
		m_VehicleConditionManager.AddCheck(new SCR_VehicleOperabilityCheck());
		m_VehicleConditionManager.AddCheck(new SCR_VehiclePilotVacancyCheck());

		m_AiGroupConditionManager = new SCR_AIGroupConditionManager();
		m_AiGroupConditionManager.AddCheck(new SCR_AIGroupAutonomyCheck());
		m_AiGroupConditionManager.AddCheck(new SCR_AIGroupSubordinationCheck());
	}
}
