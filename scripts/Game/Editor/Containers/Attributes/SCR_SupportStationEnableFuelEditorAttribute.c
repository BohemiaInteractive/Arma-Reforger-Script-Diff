[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SupportStationEnableFuelEditorAttribute : SCR_BaseSupportStationEnableEditorAttribute
{
	protected override SCR_ESupportStationResupplyType GetSupportStationType()
	{
		return ESupportStationType.FUEL;
	}
};