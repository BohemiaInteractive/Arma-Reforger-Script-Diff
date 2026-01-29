//------------------------------------------------------------------------------------------------
//! Returns true if character is in a vehicle
[BaseContainerProps()]
class SCR_VehicleDrivingAssistanceCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: SCR_Enum.GetDefault(EVehicleDrivingAssistanceMode.FULL), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EVehicleDrivingAssistanceMode))]
	EVehicleDrivingAssistanceMode m_eDrivingAssistanceMode;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled entity is in vehicle
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		EVehicleDrivingAssistanceMode drivingAssistanceMode = CarControllerComponent.GetDrivingAssistanceMode();
		bool result = m_eDrivingAssistanceMode == drivingAssistanceMode;
		return GetReturnResult(result);
	}
};
