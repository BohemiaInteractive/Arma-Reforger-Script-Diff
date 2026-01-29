[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_ArsenalEnabledTypesEditorAttribute : SCR_BaseMultiSelectPresetsEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		if (!IsGameMode(item))
			return null;
		
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return null;

		return SCR_BaseEditorAttributeVar.CreateInt(arsenalManager.GetEnabledArsenalTypes());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		if (!IsGameMode(item))
			return;
		
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return;
		
		arsenalManager.SetEnabledArsenalTypes(var.GetInt(), playerID);
	}
};
