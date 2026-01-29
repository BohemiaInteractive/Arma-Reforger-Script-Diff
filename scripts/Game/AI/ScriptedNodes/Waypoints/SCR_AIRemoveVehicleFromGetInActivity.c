class SCR_AIRemoveVehicleFromGetInActivity : AITaskScripted
{	
	static const string PORT_VEHICLE = "VehicleIn";
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group)
		{
			SCR_AgentMustBeAIGroup(this, owner);
			return ENodeResult.FAIL;
		}

		SCR_AIGroupUtilityComponent utility = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
		if(!utility)
			return ENodeResult.FAIL;
		SCR_AIGetInActivity getIn =  SCR_AIGetInActivity.Cast(utility.GetCurrentAction());
		if (!getIn)
			return ENodeResult.FAIL;
		
		IEntity vehicleToClear;
		if (!GetVariableIn(PORT_VEHICLE, vehicleToClear))
			return ENodeResult.FAIL;
		
		if (getIn.m_Vehicle.m_Value == vehicleToClear) // clear activity vehicle only if it is getIn activity dedicated to the specific vehicle
		{
			getIn.ClearActivityVehicle();
			// if there are no other vehicles, fail straight away without restarting the activity
			if (utility.m_VehicleMgr.GetVehiclesCount() == 0)
				getIn.Fail();
		}
		
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_VEHICLE
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Removes vehicle from get in activity";
	}
};