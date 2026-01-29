//------------------------------------------------------------------------------------------------
//! Returns true if current vehicle lights are enabled
[BaseContainerProps()]
class SCR_VehicleLightsCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: SCR_Enum.GetDefault(ELightType.NoLight), uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ELightType))]
	ELightType m_eLightType;

	//! Side of turn signals
	[Attribute("-1", uiwidget: UIWidgets.ComboBox, enums: {ParamEnum("Either", "-1"), ParamEnum("Left", "0"), ParamEnum("Right", "1")})]
	protected int m_iLightSide;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		VehicleControllerComponent controller = VehicleControllerComponent.Cast(data.GetCurrentVehicleController());
		if (!controller)
			return false;

		BaseLightManagerComponent lightManager = controller.GetLightManager();
		if (!lightManager)
			return false;

		bool result = lightManager.GetLightsState(m_eLightType, m_iLightSide);
		return GetReturnResult(result);
	}
}
