[BaseContainerProps()]
class SCR_LastUsedInputDevice : SCR_AvailableActionCondition
{
	[Attribute(defvalue: EInputDeviceType.KEYBOARD.ToString(), desc: "Input Device Type", uiwidget: UIWidgets.ComboBox, enumType: EInputDeviceType)]
	private EInputDeviceType m_eInputDeviceType;

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		return GetReturnResult(GetGame().GetInputManager().GetLastUsedInputDevice() == m_eInputDeviceType);
	}
}
