class SCR_AIUnflipVehicleActivity : SCR_AIActivityBase
{
	ref SCR_BTParam<IEntity> m_Vehicle = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParam<float> m_fRadius = new SCR_BTParam<float>(SCR_AIActionTask.RADIUS_PORT);
	ref SCR_BTParam<vector> m_vOrigin = new SCR_BTParam<vector>(SCR_AIActionTask.ORIGIN_PORT);
	
	protected SCR_AIGroup m_OwnerGroup;
	
	void InitParameters(IEntity vehicle, float priorityLevel, float radius, vector origin)
	{
		m_Vehicle.Init(this, vehicle);
		m_fPriorityLevel.Init(this, priorityLevel);
		m_fRadius.Init(this, radius);
		m_vOrigin.Init(this, origin);		
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AIUnflipVehicleActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, IEntity vehicle, 
			float radius = 0, vector origin = vector.Zero, float priority = PRIORITY_ACTIVITY_UNFLIP_VEHICLE, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		SCR_EntityWaypoint entityWp = SCR_EntityWaypoint.Cast(relatedWaypoint);
		if (entityWp)
		{
			vehicle = entityWp.GetEntity();
			radius = entityWp.GetCompletionRadius();
			origin = entityWp.GetOrigin();
		}
		InitParameters(vehicle, priorityLevel, radius, origin);
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityUnflipVehicle.bt";
		SetPriority(priority);
		if (utility)
			m_OwnerGroup = utility.m_Owner;
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " unflipping to " + m_Vehicle.m_Value.ToString();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		if (m_RelatedWaypoint)
			m_OwnerGroup.CompleteWaypoint(m_RelatedWaypoint);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		SendCancelMessagesToAllAgents();
		if (m_RelatedWaypoint)
			m_OwnerGroup.CompleteWaypoint(m_RelatedWaypoint);
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearActivityVehicle()
	{
		m_Vehicle.m_Value = null;
	}
};
