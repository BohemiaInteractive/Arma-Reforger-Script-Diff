[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SupportStationEnableRepairEditorAttribute : SCR_BaseSupportStationEnableEditorAttribute
{
	protected override SCR_ESupportStationResupplyType GetSupportStationType()
	{
		return ESupportStationType.REPAIR;
	}
};