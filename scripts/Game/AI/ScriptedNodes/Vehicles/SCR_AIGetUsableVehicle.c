class SCR_AIGetUsableVehicle : AITaskScripted
{
	static const string PORT_COMPARTMENT_TYPE = "CompartmentTypeIn";
	static const string PORT_VEHICLE = "VehicleOut";
	static const string PORT_COMPARTMENT = "CompartmentOut";
	
	[Attribute("0", UIWidgets.ComboBox, "Find vehicle for:", "", ParamEnumArray.FromEnum(ECompartmentType) )]
	protected ECompartmentType m_eCompartmentType;
	
	[Attribute("0", UIWidgets.CheckBox, "Reserve compartment?" )]
	protected bool m_bReserveCompartment;
	
	protected ref array<BaseCompartmentSlot> m_aOutCompartments = {};
		
	//----------------------------------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		BaseCompartmentSlot compartmentOut;
		ECompartmentType compType;
		SCR_AIGroup group = SCR_AIGroup.Cast(owner);
		if (!group) 
			group = SCR_AIGroup.Cast(owner.GetParentGroup());
		if (!group)
			return ENodeResult.FAIL;
		
		if (!GetVariableIn(PORT_COMPARTMENT_TYPE,compType))
			compType = m_eCompartmentType;
		
		array<IEntity> vehicles = {};
		group.GetGroupUtilityComponent().m_VehicleMgr.GetAllVehicleEntities(vehicles);
		if (!vehicles || vehicles.IsEmpty())
		 	return ENodeResult.FAIL;
		
		IEntity vehicle;
		if (SCR_AICompartmentHandling.FindAvailableCompartmentInVehicles(vehicles, compType, compartmentOut, vehicle))
		{
			if (m_bReserveCompartment)
				group.AllocateCompartment(compartmentOut);
			SetVariableOut(PORT_COMPARTMENT, compartmentOut);
			SetVariableOut(PORT_VEHICLE, vehicle);
			return ENodeResult.SUCCESS;
		}
		ClearVariable(PORT_VEHICLE);
		return ENodeResult.FAIL;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_COMPARTMENT_TYPE
	};
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_VEHICLE,PORT_COMPARTMENT
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
		return "GetUsableVehicle: takes vehicles from the list of known vehicles of group and checks if the slot of m_compartmentType is available.\n It does not relaese reservation the found compartment!";
	}
};