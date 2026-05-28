//! Returns true if character is in a vehicle
[BaseContainerProps()]
class SCR_VehicleDrivingAssistanceCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: EVehicleDrivingAssistanceMode.FULL.ToString(), desc: "Driving Assistance Mode", uiwidget: UIWidgets.ComboBox, enumType: EVehicleDrivingAssistanceMode)]
	EVehicleDrivingAssistanceMode m_eDrivingAssistanceMode;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity is in vehicle
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(m_eDrivingAssistanceMode == CarControllerComponent.GetDrivingAssistanceMode());
	}
}
