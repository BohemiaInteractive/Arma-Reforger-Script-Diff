[BaseContainerProps()]
class SCR_LastUsedInputDevice : SCR_AvailableActionCondition
{
	[Attribute("0", UIWidgets.ComboBox, "Device type", "", ParamEnumArray.FromEnum(EInputDeviceType) )]
	private EInputDeviceType m_eInputDeviceType;

	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		return GetReturnResult(GetGame().GetInputManager().GetLastUsedInputDevice() == m_eInputDeviceType);
	}
};
