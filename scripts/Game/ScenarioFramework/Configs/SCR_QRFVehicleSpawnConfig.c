class SCR_QRFVehicleSpawnConfig
{
	SCR_BaseCompartmentManagerComponent m_VehicleCompartmentMGR;
	SCR_EQRFGroupType m_eGroupType;
	vector m_vTargetPosition;
	SCR_ScenarioFrameworkQRFSlotAI m_Slot;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] vehicleCompartmentMGR
	//! \param[in] groupType type of a QRF force that this is meant to represent
	//! \param[in] targetPosition loaction to which QRF group will be sent
	//! \param[in] slot which was used to spawn this vehicle
	void SCR_QRFVehicleSpawnConfig(notnull SCR_BaseCompartmentManagerComponent vehicleCompartmentMGR, SCR_EQRFGroupType groupType, vector targetPosition, notnull SCR_ScenarioFrameworkQRFSlotAI slot)
	{
		m_VehicleCompartmentMGR = vehicleCompartmentMGR;
		m_eGroupType = groupType;
		m_vTargetPosition = targetPosition;
		m_Slot = slot;
	}
}