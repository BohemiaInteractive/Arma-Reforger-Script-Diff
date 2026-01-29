[BaseContainerProps()]
class SCR_ScenarioFrameworkVehicleActionSetHandbrake : SCR_ScenarioFrameworkVehicleActionBase
{
	[Attribute(defvalue: "1", desc: "Set Handbrake")]
	bool m_bSetHandbrake;
	
	protected CarControllerComponent m_VehicleController;
	protected VehicleWheeledSimulation m_VehicleWheelSimulation;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] character
	override void Init(Vehicle vehicle)
	{
		m_VehicleController = CarControllerComponent.Cast(vehicle.GetVehicleController());
		if (!m_VehicleController)
			return;
		
		m_VehicleWheelSimulation = m_VehicleController.GetWheeledSimulation();
		if (!m_VehicleWheelSimulation)
			return;
		
		OnActivate();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		m_VehicleController.SetPersistentHandBrake(m_bSetHandbrake);
		m_VehicleWheelSimulation.SetBreak(m_bSetHandbrake, m_bSetHandbrake);
	}
}