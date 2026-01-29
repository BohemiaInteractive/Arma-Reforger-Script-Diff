[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_EnableMilitarySupplyAllocationByRankEditorAttribute : SCR_BaseEditorAttribute
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

		return SCR_BaseEditorAttributeVar.CreateBool(arsenalManager.IsMilitarySupplyAllocationEnabled());
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

		arsenalManager.SetMilitarySupplyAllocationEnabled(var.GetBool());
	}
}