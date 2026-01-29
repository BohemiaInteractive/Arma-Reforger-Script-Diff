class SCR_AIGetFailedNavlinkEntity : AITaskScripted
{
	protected static const string PORT_NAVLINK_ENTITY = "NavlinkEntity";
	protected static const string PORT_VEHICLE_ENTITY = "VehicleEntity";
	
	//-----------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIGroup group = AIGroup.Cast(owner);
		if (!group)
			return ENodeResult.FAIL;
		
		//--------------------------------------------------
		// Find navlink entity
		
		IEntity navlinkEntity;
		IEntity vehicleEntity;

		array<AIAgent> agents = {};
		group.GetAgents(agents);
		
		foreach (AIAgent agent : agents)
		{
			AIBaseMovementComponent movementComp = agent.GetMovementComponent();
			
			AICarMovementComponent carMovementComp = AICarMovementComponent.Cast(movementComp);
			if (carMovementComp)
			{
				navlinkEntity = carMovementComp.GetLastNavlinkEntity();
				if (navlinkEntity)
				{
					BaseCompartmentSlot compartmentSlot;
					vehicleEntity = SCR_AICompartmentHandling.GetAgentVehicleAndCompartment(agent, compartmentSlot);
					break;
				}
			}
		}
		
		if (!navlinkEntity)
			return ENodeResult.FAIL;
		
		SetVariableOut(PORT_NAVLINK_ENTITY, navlinkEntity);
		SetVariableOut(PORT_VEHICLE_ENTITY, vehicleEntity);
		
		return ENodeResult.SUCCESS;
	}
	
	protected static ref TStringArray s_aVarsOut = {PORT_NAVLINK_ENTITY, PORT_VEHICLE_ENTITY};
	override TStringArray GetVariablesOut() { return s_aVarsOut; };
	
	static override bool VisibleInPalette() { return true; }
}

