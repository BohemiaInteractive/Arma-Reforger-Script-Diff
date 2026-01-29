//! This will try and find it's parent and add itself as a linked storage to the SCR_UniversalInventoryStorageComponent of parent and will link the parent to itself
//! This means if either storages is opened it will also open the linked storage
[BaseContainerProps(), BaseContainerCustomStringTitleField("Mirror link storage to parent")]
class SCR_MirrorLinkStorageToParent : SCR_BaseLinkedStorageLogic
{
	//------------------------------------------------------------------------------------------------
	protected override void DelayedInit(SCR_UniversalInventoryStorageComponent inventoryStorage)
	{
		IEntity parent = inventoryStorage.GetOwner().GetParent();
		if (!parent)
		{
			Print("'SCR_MirrorLinkStorageToParent DelayedInit()' of: '" + inventoryStorage.GetOwner() + "' is trying to set itself as linked storage but it has no parent!", LogLevel.ERROR);
			return;
		}
		
		SCR_UniversalInventoryStorageComponent parentInventoryStorage = SCR_UniversalInventoryStorageComponent.Cast(parent.FindComponent(SCR_UniversalInventoryStorageComponent));
		if (!parentInventoryStorage)
		{
			Print("'SCR_MirrorLinkStorageToParent DelayedInit()' of: '" + inventoryStorage.GetOwner() + "' is trying to set itself as linked storage but parent has no SCR_UniversalInventoryStorageComponent!", LogLevel.ERROR);
			return;
		}
		
		//~ Link self to parent
		parentInventoryStorage.AddLinkedStorage(inventoryStorage);
		
		//~ Link parent to self
		inventoryStorage.AddLinkedStorage(parentInventoryStorage);
	}
}
