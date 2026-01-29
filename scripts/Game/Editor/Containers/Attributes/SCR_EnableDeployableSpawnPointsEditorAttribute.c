[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_EnableDeployableSpawnPointsEditorAttribute : SCR_BaseEditorAttribute
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
		
		return SCR_BaseEditorAttributeVar.CreateBool(playerSpawnPointManager.IsDeployingSpawnPointsEnabled());
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
			
		playerSpawnPointManager.EnableDeployableSpawnPoints(var.GetBool(), playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{		
		//Set sub labels
		if (isInit)
		{
			manager.SetAttributeAsSubAttribute(SCR_GlobalDeployableRadioSettingEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_DeployableRadioTicketEditorAttribute);
		}
		
		SCR_BaseEditorAttributeVar typeVar;
		int type = -1;
		
		manager.GetAttributeVariable(SCR_GlobalDeployableRadioSettingEditorAttribute, typeVar);
		if (typeVar)
			type = typeVar.GetInt();
		
		manager.SetAttributeEnabled(SCR_GlobalDeployableRadioSettingEditorAttribute, var && var.GetBool());
		manager.SetAttributeEnabled(SCR_DeployableRadioTicketEditorAttribute, var && var.GetBool() && type == 2);
	}
}
