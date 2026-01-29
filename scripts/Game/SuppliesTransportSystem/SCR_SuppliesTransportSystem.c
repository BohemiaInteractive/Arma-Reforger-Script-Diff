void OnTransportUnitChangedDelegate(SCR_TransportUnitComponent transportUnit);
typedef func OnTransportUnitChangedDelegate;
typedef ScriptInvokerBase<OnTransportUnitChangedDelegate> OnTransportUnitChangedInvoker;

class SCR_SuppliesTransportSystem : GameSystem
{
	[Attribute("5", desc: "How often will the Supplies transport system be updated [s].", params: "0 100 0.1")]
	protected float m_fUpdateInterval;

	protected float m_fTimer;

	protected ref set<SCR_TransportUnitComponent> m_TransportUnits = new set<SCR_TransportUnitComponent>();

	protected ref map<SCR_ResupplyCampaignMilitaryBaseTaskEntity, ref array<SCR_TransportUnitComponent>> m_mResupplyTaskToTransportUnitsMap = new map<SCR_ResupplyCampaignMilitaryBaseTaskEntity, ref array<SCR_TransportUnitComponent>>();

	protected ref OnTransportUnitChangedInvoker m_OnTransportUnitAdded;
	protected ref OnTransportUnitChangedInvoker m_OnTransportUnitRemoved;

	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.AddPoint(ESystemPoint.FixedFrame);
	}

	//------------------------------------------------------------------------------------------------
	//! \return the instance of Supplies Transport System.
	static SCR_SuppliesTransportSystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;

		return SCR_SuppliesTransportSystem.Cast(world.FindSystem(SCR_SuppliesTransportSystem));
	}

	//------------------------------------------------------------------------------------------------
	OnTransportUnitChangedInvoker GetOnTransportUnitAdded()
	{
		if (!m_OnTransportUnitAdded)
			m_OnTransportUnitAdded = new OnTransportUnitChangedInvoker();

		return m_OnTransportUnitAdded;
	}

	//------------------------------------------------------------------------------------------------
	OnTransportUnitChangedInvoker GetOnTransportUnitRemoved()
	{
		if (!m_OnTransportUnitRemoved)
			m_OnTransportUnitRemoved = new OnTransportUnitChangedInvoker();

		return m_OnTransportUnitRemoved;
	}

	//------------------------------------------------------------------------------------------------
	set<SCR_TransportUnitComponent> GetTransportUnits()
	{
		return m_TransportUnits;
	}

	//------------------------------------------------------------------------------------------------
	int GetTransportUnits(out notnull array<SCR_TransportUnitComponent> transportUnits, Faction faction)
	{
		transportUnits.Clear();
		int index;
		foreach (SCR_TransportUnitComponent transportUnit : m_TransportUnits)
		{
			if (transportUnit.GetFaction() == faction)
			{
				transportUnits.Insert(transportUnit);
				index++;
			}
		}

		return index;
	}

	//------------------------------------------------------------------------------------------------
	bool Register(notnull SCR_TransportUnitComponent transportUnit)
	{
		if (m_TransportUnits.Insert(transportUnit))
		{
			if (m_OnTransportUnitAdded)
				m_OnTransportUnitAdded.Invoke(transportUnit);

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool Unregister(notnull SCR_TransportUnitComponent transportUnit)
	{
		if (m_TransportUnits.RemoveItem(transportUnit))
		{
			if (m_OnTransportUnitRemoved)
				m_OnTransportUnitRemoved.Invoke(transportUnit);

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Determines whether the specified vehicle is currently in use by any transport unit, excluding an optional transport unit.
	//! \param[in] vehicle The vehicle to check usage status for.
	//! \param[in] ignoredTransportUnit (Optional) A transport unit to exclude from the usage check.
	//! \return Returns true if the vehicle is in use by any transport unit except the ignored one; otherwise, false.
	bool IsVehicleUsed(Vehicle vehicle, SCR_TransportUnitComponent ignoredTransportUnit = null)
	{
		foreach (SCR_TransportUnitComponent transportUnit : m_TransportUnits)
		{
			if (transportUnit == ignoredTransportUnit)
				continue;

			if (transportUnit.GetState() == SCR_ETransportUnitState.ON_TASK && transportUnit.GetVehicle() == vehicle)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Determines whether the specified vehicle is currently selected by any transport unit, excluding an optional transport unit.
	//! \param[in] vehicle The vehicle to check selection status for.
	//! \param[in] ignoredTransportUnit (Optional) A transport unit to exclude from the selection check.
	//! \return Returns true if the vehicle is selected by any transport unit except the ignored one; otherwise, false.
	bool IsVehicleSelected(Vehicle vehicle, SCR_TransportUnitComponent ignoredTransportUnit = null)
	{
		foreach (SCR_TransportUnitComponent transportUnit : m_TransportUnits)
		{
			if (transportUnit == ignoredTransportUnit)
				continue;

			if (transportUnit.GetVehicle() == vehicle)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Determines whether the specified vehicle is currently being boarded by any transport unit, excluding an optional transport unit.
	//! \param[in] vehicle The vehicle to check boarding status for.
	//! \param[in] ignoredTransportUnit (Optional) A transport unit to exclude from the boarding check.
	//! \return Returns true if the vehicle is being boarded by any transport unit except the ignored one; otherwise, false.
	bool IsVehicleBoarded(Vehicle vehicle, SCR_TransportUnitComponent ignoredTransportUnit = null)
	{
		foreach (SCR_TransportUnitComponent transportUnit : m_TransportUnits)
		{
			if (transportUnit == ignoredTransportUnit)
				continue;

			if (transportUnit.GetVehicle() != vehicle)
				continue;

			if (transportUnit.GetResupplyTaskSolverState() == SCR_EResupplyTaskSolverState.BOARDING_VEHICLE)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Determines whether the specified task is currently being solved by any transport unit, excluding an optional transport unit.
	//! \param[in] task The task to check if it's being solved.
	//! \param[in] ignoredTransportUnit (Optional) A transport unit to exclude from the check.
	//! \return Returns true if the task is being solved by any transport unit except the ignored one; otherwise, false.
	bool IsTaskSolved(SCR_Task task, SCR_TransportUnitComponent ignoredTransportUnit = null)
	{
		foreach (SCR_TransportUnitComponent transportUnit : m_TransportUnits)
		{
			if (transportUnit == ignoredTransportUnit)
				continue;

			if (transportUnit.GetState() == SCR_ETransportUnitState.ON_TASK && transportUnit.GetPreferableResupplyTask() == task)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float timeSlice = args.GetTimeSliceSeconds();

		m_fTimer += timeSlice;
		if (m_fTimer < m_fUpdateInterval)
			return;

		m_fTimer = 0;

		foreach (SCR_TransportUnitComponent transportUnit : m_TransportUnits)
		{
			transportUnit.Update(timeSlice);
		}

		EvaluateTransportUnits();

		foreach (SCR_TransportUnitComponent transportUnit : m_TransportUnits)
		{
			transportUnit.ReturnToSourceBaseIfNeeded();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void PopulateResupplyTaskToTransportUnitsMap()
	{
		m_mResupplyTaskToTransportUnitsMap.Clear();
		foreach (SCR_TransportUnitComponent transportUnit : m_TransportUnits)
		{
			if (transportUnit.IsReadyForTaskSolving())
			{
				SCR_ResupplyCampaignMilitaryBaseTaskEntity task = transportUnit.GetPreferableResupplyTask();
				if (!task || IsTaskSolved(task))
					continue;

				array<SCR_TransportUnitComponent> units = m_mResupplyTaskToTransportUnitsMap.Get(task);
				if (units)
				{
					units.Insert(transportUnit);
				}
				else
				{
					units = {};
					units.Insert(transportUnit);
					m_mResupplyTaskToTransportUnitsMap.Insert(task, units);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void StartSolvingTaskByClosestUnit(SCR_ResupplyCampaignMilitaryBaseTaskEntity task, array<SCR_TransportUnitComponent> units)
	{
		float minSqDistance = float.MAX;
		SCR_TransportUnitComponent closestTransportUnit;
		foreach (SCR_TransportUnitComponent transportUnit : units)
		{
			float sqDistance = vector.DistanceSqXZ(task.GetOrigin(), transportUnit.GetOwner().GetOrigin());
			if (sqDistance < minSqDistance)
			{
				minSqDistance = sqDistance;
				closestTransportUnit = transportUnit;
			}
		}

		if (closestTransportUnit)
		{
			closestTransportUnit.SelectForTaskSolving(task);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void EvaluateTransportUnits()
	{
		PopulateResupplyTaskToTransportUnitsMap();

		foreach (SCR_ResupplyCampaignMilitaryBaseTaskEntity task, array<SCR_TransportUnitComponent> units : m_mResupplyTaskToTransportUnitsMap)
		{
			StartSolvingTaskByClosestUnit(task, units);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnInit()
	{
		super.OnInit();

		if (GetNode().GetRole() == RplRole.Proxy)
			Enable(false);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] timeSlice interval for diagnostics update.
	override protected void OnDiag(float timeSlice)
	{
		super.OnDiag(timeslice);

		DrawDebug();
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawDebug()
	{
		DbgUI.Begin("SCR_SuppliesTransportSystem", 0, 100);

		string time = SCR_FormatHelper.FormatTime(GetWorld().GetWorldTime() * 0.001);
		DbgUI.Text(string.Format("Time: %1.", time));
		foreach (SCR_TransportUnitComponent transportUnit : m_TransportUnits)
		{
			if (!transportUnit)
				continue;

			string sourceBaseName = "None";
			SCR_CampaignMilitaryBaseComponent base = transportUnit.GetSourceBase();
			if (base)
				sourceBaseName = base.GetCallsignDisplayNameOnly();

			string transportUnitState = SCR_Enum.GetEnumName(SCR_ETransportUnitState, transportUnit.GetState());

			SCR_ResupplyTaskSolver resupplySolver = transportUnit.GetSolver();
			string solverState = "No Solver";
			string resupplySolverState = solverState;
			if (resupplySolver)
			{
				solverState = SCR_Enum.GetEnumName(SCR_ETaskSolverState, resupplySolver.GetState());
				resupplySolverState = SCR_Enum.GetEnumName(SCR_EResupplyTaskSolverState, resupplySolver.GetResupplyTaskSolverState());
			}

			string preferableTaskName = "None";
			SCR_ResupplyCampaignMilitaryBaseTaskEntity resupplyTask = transportUnit.GetPreferableResupplyTask();
			if (resupplyTask)
			{
				SCR_CampaignMilitaryBaseComponent suppliedBase = resupplyTask.GetMilitaryBase();
				if (suppliedBase)
				{
					preferableTaskName = string.Format("Resupply %1 (%2)", suppliedBase.GetBaseNameUpperCase(), suppliedBase.GetCallsignDisplayNameOnlyUC());
				}
			}

			string transportUnitName = transportUnit.GetName();
			
			// Remove the hash from the string; otherwise, DbgUI.Text will throw a warning about a missing string ID.
			if (transportUnitName.StartsWith("#"))
				transportUnitName = transportUnitName.Substring(1, transportUnitName.Length() - 1);

			DbgUI.Text(string.Format("%1 - state: %2, usable vehicle: %3, source base: %4, prefered task: %5, solver state: %6, resupply solver state: %7",
				transportUnitName, transportUnitState, transportUnit.IsVehicleUsable(), sourceBaseName, preferableTaskName, solverState,
				resupplySolverState));

			Vehicle vehicle = transportUnit.GetVehicle();
			if (!vehicle)
				continue;

			int lineColor = Color.DODGER_BLUE;
			if (transportUnit.GetState() == SCR_ETransportUnitState.ON_TASK)
				lineColor = Color.SPRING_GREEN;

			Shape.Create(ShapeType.LINE, lineColor, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, transportUnit.GetOwner().GetOrigin(), vehicle.GetOrigin());
		}

		DbgUI.End();
	}
}
