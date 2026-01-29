[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_SupportStationEnableResupplyEditorAttribute : SCR_BaseSupportStationEnableEditorAttribute
{
	protected override SCR_ESupportStationResupplyType GetSupportStationType()
	{
		return ESupportStationType.RESUPPLY_AMMO;
	}
};