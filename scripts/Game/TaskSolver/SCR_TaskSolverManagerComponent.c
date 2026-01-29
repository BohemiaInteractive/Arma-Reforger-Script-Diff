[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_TaskSolverManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_TaskSolverManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute()]
	protected ref SCR_TaskSolverConfig m_TaskSolverConfig;

	protected ref array<ref SCR_TaskSolverBase> m_aActiveSolvers = {};

	protected RplComponent m_RplComponent;

	static protected int s_iMaxSolverId = -1;

	//------------------------------------------------------------------------------------------------
	//! Creates, configures, and registers a new solver.
	//! \param[in] solverType The solver typename.
	//! \return The created solver or null if failed.
	SCR_TaskSolverBase CreateSolver(typename solverType)
	{
		if (!solverType)
			return null;

		SCR_TaskSolverBase solver = SCR_TaskSolverBase.Cast(solverType.Spawn());
		if (!solver)
			return null;

		s_iMaxSolverId++;
		solver.SetId(s_iMaxSolverId);

		SetSolverEntry(solver);

		m_aActiveSolvers.Insert(solver);

		if (IsMaster())
		{
			SCR_ResupplyTaskSolver resupplyTaskSolver = SCR_ResupplyTaskSolver.Cast(solver);
			if (resupplyTaskSolver)
				resupplyTaskSolver.GetOnResupplyTaskSolverStateChanged().Insert(OnResupplyTaskSolverStateChanged);
		}

		return solver;
	}

	//------------------------------------------------------------------------------------------------
	//! Starts and injects task/context into the given solver.
	//! \param[in] solver The solver to initialize and start.
	//! \param[in] task The task to inject.
	//! \param[in] context The context.
	void StartSolverTask(SCR_TaskSolverBase solver, SCR_Task task, SCR_TaskSolverContext context)
	{
		int index = m_aActiveSolvers.Find(solver);
		if (!m_aActiveSolvers.IsIndexValid(index))
			return;

		solver.Init(task, context);
		solver.StartTaskSolving();
	}

	//------------------------------------------------------------------------------------------------
	//! Removes and cleans up a solver.
	//! \param[in] solver The solver to remove.
	void RemoveSolver(notnull SCR_TaskSolverBase solver)
	{
		int idx = m_aActiveSolvers.Find(solver);
		if (idx != -1)
		{
			solver.Deinit();
			m_aActiveSolvers.Remove(idx);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Updates all solvers
	void Update(float timeSlice)
	{
		foreach (SCR_TaskSolverBase solver : m_aActiveSolvers)
		{
			solver.Update(timeSlice);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsMaster()
	{
		return !m_RplComponent || m_RplComponent.IsMaster();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnResupplyTaskSolverStateChanged(int solverId, SCR_EResupplyTaskSolverState resupplyTaskSolverState)
	{
		Rpc(RpcDo_SetResupplyTaskSolverState, solverId, resupplyTaskSolverState);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetResupplyTaskSolverState(int solverId, SCR_EResupplyTaskSolverState resupplyTaskSolverState)
	{
		SCR_ResupplyTaskSolver resupplyTaskSolver = SCR_ResupplyTaskSolver.Cast(Find(solverId));
		if (!resupplyTaskSolver)
			return;

		resupplyTaskSolver.SetResupplyTaskSolverState(resupplyTaskSolverState);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_TaskSolverBase Find(int solverId)
	{
		foreach (SCR_TaskSolverBase solver : m_aActiveSolvers)
		{
			if (solver.GetId() == solverId)
				return solver;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Finds a solver associated with a task (linear search).
	//! \param[in] task The task to search for.
	//! \return The solver or null if not found.
	protected SCR_TaskSolverBase Find(notnull SCR_Task task)
	{
		foreach (SCR_TaskSolverBase solver : m_aActiveSolvers)
		{
			if (solver.GetTask() == task)
				return solver;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Configures the solver with entries/config as needed.
	protected void SetSolverEntry(SCR_TaskSolverBase solver)
	{
		if (!m_TaskSolverConfig || !solver)
			return;

		SCR_ResupplyTaskSolver resupplySolver = SCR_ResupplyTaskSolver.Cast(solver);
		if (resupplySolver)
		{
			SCR_ResupplyTaskSolverEntry entry = SCR_ResupplyTaskSolverEntry.Cast(
				m_TaskSolverConfig.GetTaskSolverEntry(SCR_ResupplyTaskSolverEntry)
			);
			if (entry)
				resupplySolver.SetResupplyTaskSolverEntry(entry);
		}
		// Extend as needed for other types.
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		s_iMaxSolverId = -1;

		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! Cleans up all solvers on delete.
	override protected void OnDelete(IEntity owner)
	{
		SCR_ResupplyTaskSolver resupplyTaskSolver;
		foreach (SCR_TaskSolverBase solver : m_aActiveSolvers)
		{
			solver.Deinit();

			if (IsMaster())
			{
				resupplyTaskSolver = SCR_ResupplyTaskSolver.Cast(solver);
				if (resupplyTaskSolver)
					resupplyTaskSolver.GetOnResupplyTaskSolverStateChanged().Remove(OnResupplyTaskSolverStateChanged);
			}
		}

		m_aActiveSolvers.Clear();
		super.OnDelete(owner);
	}
}
