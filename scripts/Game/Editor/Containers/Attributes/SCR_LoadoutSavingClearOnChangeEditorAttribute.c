/*!
This is a attribute that is never set anywhere and reset when attribute dialog is opened and used by SCR_LoadoutSavingBlackListEditorAttribute to clear player loadouts if the loadout changed
*/
[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_LoadoutSavingClearOnChangeEditorAttribute : SCR_BaseEditorAttribute
{	
	//------------------------------------------------------------------------------------------------
	override bool IsSerializable()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{				
		if (!IsGameMode(item))
			return null;
		
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return null;
		
		SCR_LoadoutSaveBlackListHolder saveBlackList = arsenalManager.GetLoadoutSaveBlackListHolder();
		int blackListCount = saveBlackList.GetBlackListsCount();
		if (!saveBlackList || blackListCount == 0 || blackListCount > SCR_LoadoutSavingBlackListEditorAttribute.MAX_ENTRIES)
			return null;

		
		return SCR_BaseEditorAttributeVar.CreateBool(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{			
		if (isInit || !var)
			return;
		
		//~ On update value set the third value in the Vector of SCR_LoadoutSavingBlackListEditorAttribute to the value if the change should clear existing loadouts or not
		SCR_BaseEditorAttributeVar savingBlackListVar;
		if (!manager.GetAttributeVariable(SCR_LoadoutSavingBlackListEditorAttribute, savingBlackListVar))
			return;
		
		vector SavingBlacklistValue = savingBlackListVar.GetVector();
		
		SavingBlacklistValue[2] = var.GetInt();
		savingBlackListVar.SetVector(SavingBlacklistValue);
		
		manager.SetAttributeVariable(SCR_LoadoutSavingBlackListEditorAttribute, savingBlackListVar);
	}
};
