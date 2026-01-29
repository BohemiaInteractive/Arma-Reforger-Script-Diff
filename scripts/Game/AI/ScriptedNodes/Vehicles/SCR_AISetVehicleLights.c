class SCR_AISetVehicleLights : AITaskScripted
{
	protected IEntity m_vehicleEnt;
	
	[Attribute("0", UIWidgets.ComboBox, "Light Type: ", "", ParamEnumArray.FromEnum(ELightType) )]
	protected ELightType m_eLightType;
	
	[Attribute("0", UIWidgets.CheckBox, "Enabled: ")]
	protected bool m_bEnabled;
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		if (!GetVariableIn("VehicleIn", m_vehicleEnt))
			return ENodeResult.FAIL;
		
		bool bEnabled;
		if (!GetVariableIn("EnabledIn", bEnabled))
			bEnabled = m_bEnabled;
		
		Vehicle vehicle = Vehicle.Cast(m_vehicleEnt);
		if (!vehicle)
			return ENodeResult.FAIL;

		BaseLightManagerComponent vehicleLightManagerComp = BaseLightManagerComponent.Cast(vehicle.FindComponent(BaseLightManagerComponent));
		if (!vehicleLightManagerComp)
			return ENodeResult.FAIL;
		
		vehicleLightManagerComp.SetLightsState(m_eLightType, bEnabled);
		
		return ENodeResult.SUCCESS;
	}	
	
	protected static ref TStringArray s_aVarsIn = {
		"VehicleIn", "EnabledIn"
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	static override bool VisibleInPalette()
    {
        return true;
    }
	
	static override string GetOnHoverDescription()
	{
		return "Toggles the state of lights in a vehicle";
	}	
};