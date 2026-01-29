[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_GlobalDeployableRadioSettingEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		BaseGameMode gameMode = BaseGameMode.Cast(item);
		if (!gameMode)
			return null;
		
		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
		if (!playerSpawnPointManager)
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateInt(playerSpawnPointManager.GetDeployableSpawnPointBudgetType());
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
				
		BaseGameMode gameMode = BaseGameMode.Cast(item);
		if (!gameMode)
			return;
		
		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
		if (!playerSpawnPointManager)
			return;
		
		return playerSpawnPointManager.SetDeployableSpawnPointBudgetType(var.GetInt(), playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{		
		//Set sub labels
		if (isInit)
			manager.SetAttributeAsSubAttribute(SCR_DeployableRadioTicketEditorAttribute);
		
		SCR_BaseEditorAttributeVar enabledVar;
		bool enabled;
		
		manager.GetAttributeVariable(SCR_EnableDeployableSpawnPointsEditorAttribute, enabledVar);
		if (enabledVar)
			enabled = enabledVar.GetBool();
		
		manager.SetAttributeEnabled(SCR_DeployableRadioTicketEditorAttribute, enabled && var && var.GetInt() == 2);
	}
}
