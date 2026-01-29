[BaseContainerProps(), BaseContainerCustomStringTitleField("DO NOT USE BASE CLASS!")]
class SCR_BaseLinkedStorageLogic
{
	//------------------------------------------------------------------------------------------------
	sealed void Init(notnull SCR_UniversalInventoryStorageComponent inventoryStorage)
	{
		//~ Delay init so slotted entities know correctly who their parents are
		GetGame().GetCallqueue().CallLater(DelayedInit, param1: inventoryStorage);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedInit(SCR_UniversalInventoryStorageComponent inventoryStorage);
}
