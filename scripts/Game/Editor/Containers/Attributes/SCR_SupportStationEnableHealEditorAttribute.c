[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SupportStationEnableHealEditorAttribute : SCR_BaseSupportStationEnableEditorAttribute
{
	protected override SCR_ESupportStationResupplyType GetSupportStationType()
	{
		return ESupportStationType.HEAL;
	}
};