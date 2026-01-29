[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_LoadoutSupplyCostMultiplierEditorAttribute : SCR_BaseValueListEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(item);
		if (!gameMode)
			return null;
		
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateFloat(arsenalManager.GetLoadoutSpawnSupplyCostMultiplier());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(item);
		if (!gameMode)
			return;
		
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return;
		
		arsenalManager.SetLoadoutSpawnCostMultiplier_S(var.GetFloat(), playerID);
	}
}
