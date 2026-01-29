/**
Respawn attribute to enable/disable respawn and respawn time, for getting and setting varriables in Editor Attribute window
*/
// Script File 
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_RespawnEnabledEditorAttribute : SCR_BaseEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		//If opened in global attributes
		if (!IsGameMode(item)) return null;
		
		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnSystem) return null;
		
		bool value = respawnSystem.IsRespawnEnabled();
		return SCR_BaseEditorAttributeVar.CreateBool(value);
	}
	
	//Disable respawn time if respawning is disabled
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{		
		//Set sub labels
		if (isInit)
		{
			manager.SetAttributeAsSubAttribute(SCR_RespawnTimeEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_SpawnAtPlayersEditorAttribute);
			manager.SetAttributeAsSubAttribute(SCR_SpawnAtRadioVehicleAttribute);
			manager.SetAttributeAsSubAttribute(SCR_LoadoutSupplyCostMultiplierEditorAttribute);
		}
			
		manager.SetAttributeEnabled(SCR_RespawnTimeEditorAttribute, var && var.GetBool());
		manager.SetAttributeEnabled(SCR_SpawnAtPlayersEditorAttribute, var && var.GetBool());
		manager.SetAttributeEnabled(SCR_SpawnAtRadioVehicleAttribute, var && var.GetBool());
		
		SCR_BaseEditorAttributeVar globalVar;
		manager.GetAttributeVariable(SCR_EnableGlobalResourceTypeEditorAttribute, globalVar);
		bool suppliesEnabled = true;
		
		if (globalVar)
			suppliesEnabled = globalVar.GetBool();
		
		manager.SetAttributeEnabled(SCR_LoadoutSupplyCostMultiplierEditorAttribute, var && var.GetBool() && suppliesEnabled);
	}
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		SCR_RespawnSystemComponent respawnSystem = SCR_RespawnSystemComponent.GetInstance();
		if (!respawnSystem) 
			return;
		
		bool value = var.GetBool();

		//Notification
		if (item)
		{
			if (value != respawnSystem.IsRespawnEnabled())
			{
				if (value == true)
				{
					SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_RESPAWN_ENABLED, playerID);
				}
				else {
					SCR_NotificationsComponent.SendToEveryone(ENotification.EDITOR_ATTRIBUTES_RESPAWN_DISABLED, playerID);
				}
			}
		}
		
		respawnSystem.ServerSetEnableRespawn(value);
	}
};