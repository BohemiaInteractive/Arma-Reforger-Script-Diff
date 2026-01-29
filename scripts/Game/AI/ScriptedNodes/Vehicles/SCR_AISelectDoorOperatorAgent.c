class SCR_AISelectDoorOperatorAgent : AITaskScripted
{
	// Inputs
	protected static const string PORT_VEHICLE_ENTITY = "VehicleEntity";
	protected static const string PORT_NAVLINK_ENTITY = "NavlinkEntity";
	
	// Outputs
	protected static const string PORT_DOOR_USER_AGENT = "DoorUserAgent";
	protected static const string PORT_DOOR_USER_ENTITY = "DoorUserEntity";
	protected static const string PORT_USE_TELEKINESIS = "UseTelekinesis";
	
	//------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIGroup group = AIGroup.Cast(owner);
		
		if (!group)
			group = owner.GetParentGroup();
		
		if (!group)
			return ENodeResult.FAIL;
		
		array<AIAgent> agents = {};
		int nAgents = group.GetAgents(agents);
		
		IEntity vehicleEntity;
		GetVariableIn(PORT_VEHICLE_ENTITY, vehicleEntity);
		
		IEntity navlinkEntity;
		GetVariableIn(PORT_NAVLINK_ENTITY, navlinkEntity);
		if (!navlinkEntity)
			return ENodeResult.FAIL;
		vector navlinkPos = navlinkEntity.GetOrigin();
		
		AIAgent bestAgent = null;
		float bestScore = -float.MAX;
		
		for (int i = 0; i < nAgents; i++)
		{
			AIAgent agent = agents[i];
			
			IEntity agentControlledEntity = agent.GetControlledEntity();
			if (!agentControlledEntity)
				continue;
			
			BaseCompartmentSlot agentCompartment;
			IEntity agentVehicleEntity = SCR_AICompartmentHandling.GetAgentVehicleAndCompartment(agent, agentCompartment);
			
			float distanceToNavlink = vector.Distance(agentControlledEntity.GetOrigin(), navlinkPos);
			
			float score = 200.0 - distanceToNavlink;
			
			if (!agentVehicleEntity)
			{
				// Not in vehicle, best
				score += 1600.0;
			}
			else
			{
				// In vehicle
				
				// Check compartment type
				ECompartmentType compType = agentCompartment.GetType();
				switch (compType)
				{
					case ECompartmentType.CARGO: score += 400.0; break; // Best
					case ECompartmentType.PILOT: score += 200.0; break; // Worse
					//case ECompartmentType.TURRET: score += 0; break; // Worst - gunner should be last to leave
				}
				
				// Is in same vehicle?
				if (agentVehicleEntity == vehicleEntity)
				{
					// In same vehicle, better
					score += 800.0;
				}
			}
			
			if (score > bestScore)
			{
				bestAgent = agent;
				bestScore = score;
			}
		}
		
		bool telekinesisUse = false;
		IEntity bestAgentEntity;
		if (bestAgent)
		{
			bestAgentEntity = bestAgent.GetControlledEntity();
			if (bestAgent.GetLOD() == bestAgent.GetMaxLOD())
			{
				telekinesisUse = true;
			}
		}
		
		SetVariableOut(PORT_USE_TELEKINESIS, telekinesisUse);
		SetVariableOut(PORT_DOOR_USER_AGENT, bestAgent);
		SetVariableOut(PORT_DOOR_USER_ENTITY, bestAgentEntity);
		
		if (bestAgent && bestAgentEntity)
			return ENodeResult.SUCCESS;
		else
			return ENodeResult.FAIL;
	}
	
	//------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	//------------------------------------------------------------------------
	static override string GetOnHoverDescription() { return "Selects an absolutely best possible group member to open a gate when driving a vehicle";};
	
	//------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {PORT_DOOR_USER_AGENT, PORT_DOOR_USER_ENTITY, PORT_USE_TELEKINESIS };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	//------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {PORT_VEHICLE_ENTITY, PORT_NAVLINK_ENTITY };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
}