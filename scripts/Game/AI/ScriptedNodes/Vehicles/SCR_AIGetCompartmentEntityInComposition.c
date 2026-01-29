class SCR_AIGetCompartmentEntityInComposition : AITaskScripted
{
	static const string PORT_VEHICLE = "Vehicle";
	static const string PORT_BOARDING_PARAMS = "BoardingParams";
	static const string PORT_COMPARTMENT_ENT = "CompartmentEnt";

	//----------------------------------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		IEntity compositionEntity;
		SCR_AIBoardingParameters boardingParams;
		IEntity compartmentEntity;
		
		array<Managed> compartmentManagers = {};
		array<BaseCompartmentSlot> compartments = {};
		
		GetVariableIn(PORT_VEHICLE, compositionEntity);
		if (!GetVariableIn(PORT_BOARDING_PARAMS, boardingParams) || !boardingParams)
			boardingParams = new SCR_AIBoardingParameters;
		
		if (!compositionEntity)
			return ENodeResult.FAIL;
		
		FindComponentsInAllChildren(BaseCompartmentManagerComponent, compositionEntity, false, 0, 6, compartmentManagers);
		
		foreach (Managed compartmentMan : compartmentManagers)
		{
			BaseCompartmentManagerComponent compartMan = BaseCompartmentManagerComponent.Cast(compartmentMan);
			if (!compartMan)
				continue;
			
			int numOfComp = compartMan.GetCompartments(compartments);
		
			foreach (BaseCompartmentSlot comp : compartments)
			{
				if (PilotCompartmentSlot.Cast(comp) && boardingParams.m_bIsDriverAllowed)
				{
					compartmentEntity = compartMan.GetOwner();
					break;
				}
				else if (TurretCompartmentSlot.Cast(comp) && boardingParams.m_bIsGunnerAllowed)
				{
					compartmentEntity = compartMan.GetOwner();
					break;
				}
				else if (CargoCompartmentSlot.Cast(comp) && boardingParams.m_bIsCargoAllowed)
				{
					compartmentEntity = compartMan.GetOwner();
					break;
				}
				else 
					continue;
			}
		}
		
		if (compartmentEntity)
		{
			SetVariableOut(PORT_COMPARTMENT_ENT, compartmentEntity);
			return ENodeResult.SUCCESS;
		}
		if (!boardingParams.m_bIsGunnerAllowed && !boardingParams.m_bIsCargoAllowed && !boardingParams.m_bIsDriverAllowed)
			return ENodeResult.FAIL;
		
		return NodeError(this, owner, "No entity with CompartmentManagerComponent found!");	
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_VEHICLE, PORT_BOARDING_PARAMS
	};
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_COMPARTMENT_ENT
	};
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette()
    {
        return true;
    }
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "GetCompartmentEntInComposition: finds first entity in a composition that contains compartment manager component";
	}
};