class SCR_ScenarioFrameworkCallQueueSystem : GameSystem
{
	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo
			.SetAbstract(false)
			.AddPoint(ESystemPoint.FixedFrame);
	}

	protected static ref ScriptCallQueue s_CallQueueNonPausable = new ScriptCallQueue();
	protected float m_fTimer;
	protected float m_fCheckInterval;

	//------------------------------------------------------------------------------------------------
	//! \return the instance of ScenarioFrameworkSystem.
	static SCR_ScenarioFrameworkCallQueueSystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;

		return SCR_ScenarioFrameworkCallQueueSystem.Cast(world.FindSystem(SCR_ScenarioFrameworkCallQueueSystem));
	}

	//------------------------------------------------------------------------------------------------
	//! \return the instance of ScenarioFramework Call Queue.
	static ScriptCallQueue GetCallQueueNonPausable()
	{
		return s_CallQueueNonPausable;
	}


	//------------------------------------------------------------------------------------------------
	override event protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float timeSlice = args.GetTimeSliceSeconds();

		m_fTimer += timeSlice;

		if (m_fTimer < m_fCheckInterval)
			return;

		m_fTimer = 0;

		s_CallQueueNonPausable.Tick(timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clears CallQueue after destruction of this system
	void ~SCR_ScenarioFrameworkCallQueueSystem()
	{
		if (s_CallQueueNonPausable)
			s_CallQueueNonPausable.Clear();
	}
}
