[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class SCR_LoadoutSavingBlackListEditorAttribute : SCR_BaseMultiSelectPresetsEditorAttribute
{
	protected vector m_vStartingValues = Vector(-1, -1, -1);
	
	static const int MAX_ENTRIES = 62;
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{
		if (!IsGameMode(item))
			return null;
		
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return null;
		
		SCR_LoadoutSaveBlackListHolder saveBlackList = arsenalManager.GetLoadoutSaveBlackListHolder();
		if (!saveBlackList || saveBlackList.GetBlackListsCount() == 0)
			return null;
		
		//~ Third vector is reserved for clearing loadout, Meaning that it can only have 62 values (as it is stored as a flag). So far we do not foresee anyone getting this high with entries but a safty check is added
		if (saveBlackList.GetBlackListsCount() > MAX_ENTRIES)
		{
			Print(string.Format("'SCR_LoadoutSavingBlackListEditorAttribute' Blacklist count cannot be higher than %1!", MAX_ENTRIES), LogLevel.ERROR);
			return null;
		}
		
		super.ReadVariable(item, manager);
		
		array<bool> orderedActiveList = {};
		saveBlackList.GetOrderedBlackListsActive(orderedActiveList);
		
		//~ Save in flags the ordered active bool
		foreach (bool active : orderedActiveList)
		{
			AddOrderedState(!active);
		}
		
		return SCR_BaseEditorAttributeVar.CreateVector(GetFlagVector());
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateInterlinkedVariables(SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, bool isInit = false)
	{			
		if (isInit)
		{
			manager.SetAttributeAsSubAttribute(SCR_LoadoutSavingClearOnChangeEditorAttribute);
			m_vStartingValues = var.GetVector();
		}
		
		bool enableClearAttribute = m_vStartingValues[0] != var.GetVector()[0] || m_vStartingValues[1] != var.GetVector()[1];
		
		//~ Enable the clear attribute
		if (enableClearAttribute)
		{
			manager.SetAttributeEnabled(SCR_LoadoutSavingClearOnChangeEditorAttribute, enableClearAttribute); 
		}
		//~ Disable the clear attribute
		else 
		{
			//~ Set clear to false again
			SCR_BaseEditorAttributeVar clearBool;
			if (manager.GetAttributeVariable(SCR_LoadoutSavingClearOnChangeEditorAttribute, clearBool))
			{
				clearBool.SetBool(false);
				manager.SetAttributeVariable(SCR_LoadoutSavingClearOnChangeEditorAttribute, clearBool);
			}
			
			manager.SetAttributeEnabled(SCR_LoadoutSavingClearOnChangeEditorAttribute, enableClearAttribute); 
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) 
			return;
		
		if (!IsGameMode(item))
			return;
		
		super.WriteVariable(item, var, manager, playerID);
		
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return;
		
		SCR_LoadoutSaveBlackListHolder saveBlackList = arsenalManager.GetLoadoutSaveBlackListHolder();
		if (!saveBlackList)
			return;
		
		array<bool> orderedStates = {};
		
		array<SCR_LoadoutSaveBlackList> loadoutSaveBlackLists = {};
		saveBlackList.GetLoadoutSaveBlackLists(loadoutSaveBlackLists);
		
		//~ Get if blacklist is active
		foreach (SCR_LoadoutSaveBlackList blackList : loadoutSaveBlackLists)
		{
			orderedStates.Insert(!GetOrderedState());
		}
		
		arsenalManager.SetLoadoutBlackListActiveStates(orderedStates, (int)var.GetVector()[2], playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void CreatePresets()
	{
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return;
		
		SCR_LoadoutSaveBlackListHolder saveBlackList = arsenalManager.GetLoadoutSaveBlackListHolder();
		if (!saveBlackList)
			return;
		
		m_aValues.Clear();	
		
		SCR_EditorAttributeFloatStringValueHolder value;
		
		//~ Create presets from existing blacklist items
		array<SCR_LoadoutSaveBlackList> loadoutSaveBlackLists = {};
		saveBlackList.GetLoadoutSaveBlackLists(loadoutSaveBlackLists);
		foreach (SCR_LoadoutSaveBlackList blackList : loadoutSaveBlackLists)
		{
			value = new SCR_EditorAttributeFloatStringValueHolder();
			value.SetWithUIInfo(blackList.GetUIInfo(), !blackList.IsActive());
			m_aValues.Insert(value);
		}
	}
};
