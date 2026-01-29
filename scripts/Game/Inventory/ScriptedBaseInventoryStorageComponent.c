class ScriptedBaseInventoryStorageComponentClass : BaseInventoryStorageComponentClass
{
}

class ScriptedBaseInventoryStorageComponent : BaseInventoryStorageComponent
{
	//------------------------------------------------------------------------------------------------
	// Virtual method for updating the UI when a storage slot changes
	override protected void UpdateUI()
	{
		SCR_InventoryMenuUI menu = SCR_InventoryMenuUI.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.Inventory20Menu));
		if (!menu)
			return;
		
		SCR_InventoryStorageBaseUI storageUI = menu.GetStorageUIByBaseStorageComponent(this);
		if (!storageUI)
			return;
		
		if (storageUI == menu.GetStorageList())
		{
			menu.ShowStoragesList();
			menu.ShowAllStoragesInList();
		}
		else
		{
			storageUI.Refresh();
			BaseInventoryStorageComponent storage = storageUI.GetCurrentNavigationStorage();
			SCR_InventoryOpenedStorageUI open = menu.GetOpenedStorage(storage);
			if (open)
				open.Refresh();
		}
	}
}
