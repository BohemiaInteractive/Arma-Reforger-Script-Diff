/**
Shows fuel of fuel providers
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_FuelProviderEditorAttribute : SCR_FuelEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	protected override SCR_EFuelNodeTypeFlag GetFuelNodeFlags()
	{
		return SCR_EFuelNodeTypeFlag.CAN_PROVIDE_FUEL | SCR_EFuelNodeTypeFlag.IS_FUEL_STORAGE;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override SCR_EFuelNodeTypeFlag GetFuelNodeIgnoreFlags()
	{
		return 0;
	}
};