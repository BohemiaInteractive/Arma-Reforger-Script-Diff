//! This will add all children as linked storages. Use SCR_SetLinkedStorageToParent if you know the exact storage and storage is SCR_UniversalInventoryStorageComponent
[BaseContainerProps(), BaseContainerCustomStringTitleField("Get all children storages and set as linked")]
class SCR_GetLinkedStorageChildren : SCR_BaseLinkedStorageLogic
{	
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(SCR_UniversalInventoryStorageComponent inventoryStorage)
	{
		IEntity child = inventoryStorage.GetOwner().GetChildren();
		BaseInventoryStorageComponent storage;
		
		while (child)
		{
			storage = BaseInventoryStorageComponent.Cast(child.FindComponent(BaseInventoryStorageComponent));
			if (storage && storage != inventoryStorage)
				inventoryStorage.AddLinkedStorage(storage);
		}
	}
}
