[BaseContainerProps()]
class SCR_ScenarioFrameworkVehicleActionResourceUnloadAction : SCR_ScenarioFrameworkVehicleActionBase
{
	[Attribute(defvalue: "0", desc: "Resource Type", uiwidget: UIWidgets.ComboBox, enumType: EResourceType)]
	EResourceType m_eResourceType;
	
	[Attribute(defvalue: "0", desc: "Resources To Unload", params: "0 inf 0.01")]
	float m_fResourcesToUnload;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] character
	override void Init(Vehicle vehicle)
	{
		// Here you can step out to the OnActivate method and debug this specific action if m_bDebug attribute is set to true.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Action Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkVehicleActionBase.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		SCR_ResourceComponent resourceComp = SCR_ResourceComponent.Cast(vehicle.FindComponent(SCR_ResourceComponent));
		if (!resourceComp)
			return;
		
		SCR_ResourceGenerator resourceGen = resourceComp.GetGenerator(EResourceGeneratorID.VEHICLE_UNLOAD, m_eResourceType);
		if (!resourceGen)
			return;

		SCR_ResourceConsumer resourceCon = resourceComp.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, m_eResourceType);
		if (!resourceCon)
			return;
		
		float resourcesToUnload = m_fResourcesToUnload;
		SCR_ResourceConsumtionResponse response = resourceCon.RequestConsumtion(resourcesToUnload);
		if (response.GetReason() != EResourceReason.SUFFICIENT)
			resourcesToUnload = response.GetAvailableSupply();

		SCR_ResourceGeneratorActionDropContainers action = new SCR_ResourceGeneratorActionDropContainers();
		if (action)
			action.PerformAction(resourceGen, resourcesToUnload);
	}
}