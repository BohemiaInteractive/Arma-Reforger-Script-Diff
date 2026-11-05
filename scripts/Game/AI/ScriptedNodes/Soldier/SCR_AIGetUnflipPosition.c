class SCR_AIGetUnflipPosition : AITaskScripted
{
	static const string PORT_VEHICLE_ENTITY = "VehicleEntityIn";
	static const string PORT_GROUP_AGENT = "GroupAgentIn";
	static const string PORT_POSITION = "UnflipPositionLocalOut";
	static const float SCALE_FACTOR = 1.3; 				// how to scale local position of entry door wrt flipped bounding box
	
	[Attribute("2", UIWidgets.EditBox, "Max number of unflipping agents when vehicle has no doors defined")]
	protected int m_iMaxUnflippersWithoutDoors;
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity vehicleEntity;
		GetVariableIn(PORT_VEHICLE_ENTITY, vehicleEntity);
		if (!vehicleEntity)
			return ENodeResult.FAIL;
		
		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(vehicleEntity.FindComponent(BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return ENodeResult.FAIL;
			
		AIAgent agent;
		GetVariableIn(PORT_GROUP_AGENT, agent);
		if (!agent)
			return ENodeResult.FAIL;
		
		AIGroup group = agent.GetParentGroup();
		if (!group) 
			return ENodeResult.FAIL;
		
		array<AIAgent> agents = {};
		group.GetAgents(agents);
		int agentIndex = agents.Find(agent);
		if (agentIndex < 0)
			return ENodeResult.FAIL;
		
		bool doorIndexAvailable = compartmentManager.GetDoorInfo(0) != null;
		// we have no vehicle door to align with
		if (!doorIndexAvailable) 
		{
			if (agentIndex >= m_iMaxUnflippersWithoutDoors)
				return ENodeResult.FAIL; // this agent does not unflip -> stays on perimeter

			SetVariableOut(PORT_POSITION, vector.Zero);
			return ENodeResult.SUCCESS;
		}
		
		// calculate how many doors the vehicle has
		int doorCount;
		while (doorIndexAvailable)
		{
			if (compartmentManager.GetDoorInfo(doorCount) == null)
			{
				doorIndexAvailable = false;
				doorCount--;
			}
			doorCount++;
		}
		
		// agents that don't fit to any door do not go to the vehicle
		if (agentIndex >= doorCount)
			return ENodeResult.FAIL;
		
		CompartmentDoorInfo doorInfo = compartmentManager.GetDoorInfo(agentIndex);
		if (!doorInfo)
		{
			SetVariableOut(PORT_POSITION, vector.Zero);
			return ENodeResult.SUCCESS;
		}
		
		PointInfo pointInfo = doorInfo.GetAIEntryPointInfo();
		if (!pointInfo)
		{
			SetVariableOut(PORT_POSITION, vector.Zero);
			return ENodeResult.SUCCESS;
		}
		
		// where is the entry point in worldspace? 
		// take the unturned vehicle pos + local transform of the entry
		vector mat[4];
		pointInfo.GetModelTransform(mat);
		vector doorPositionLocal = mat[3] * SCALE_FACTOR;
		
		SetVariableOut(PORT_POSITION, doorPositionLocal);
		return ENodeResult.SUCCESS;
	}
	
	
	//------------------------------------------------------------------------------------------------
	override static event protected bool VisibleInPalette() { return true; }
	override static event protected string GetOnHoverDescription() { return "Returns good local offset for unflipping a given vehicle for this agent. FAILS if agent has no free spot to unflip."; }
	
	protected static ref TStringArray s_aVarsIn = {	PORT_VEHICLE_ENTITY, PORT_GROUP_AGENT };
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	protected static ref TStringArray s_aVarsOut = { PORT_POSITION };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
};