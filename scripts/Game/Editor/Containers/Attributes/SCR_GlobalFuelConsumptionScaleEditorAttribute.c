/**
Attribute to set gloval Fuel consumption scale
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_GlobalFuelConsumptionScaleEditorAttribute : SCR_BaseValueListEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		if (!IsGameMode(item))
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateFloat(SCR_FuelConsumptionComponent.GetGlobalFuelConsumptionScale());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		if (!IsGameMode(item))
			return;
		
		SCR_FuelConsumptionComponent.SetGlobalFuelConsumptionScale(var.GetFloat(), playerID);
	}
};
