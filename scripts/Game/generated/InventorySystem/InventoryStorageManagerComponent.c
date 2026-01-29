/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup InventorySystem
\{
*/

class InventoryStorageManagerComponent: GameComponent
{
	proto external IEntity GetOwner();
	proto external void AddStorage(BaseInventoryStorageComponent storage);
	//! Will iterate over storages and find if item can be inserted in first best match
	proto external bool CanInsertItem(IEntity item, EStoragePurpose purpose = EStoragePurpose.PURPOSE_ANY);
	proto external bool CanInsertItemInStorage(IEntity item, BaseInventoryStorageComponent storage, int slotID = -1);
	//! Will iterate over storages and find if item can be inserted in first best match
	proto external bool CanInsertResource(ResourceName resourceName, EStoragePurpose purpose = EStoragePurpose.PURPOSE_ANY);
	proto external bool CanInsertResourceInStorage(ResourceName resourceName, BaseInventoryStorageComponent storage, int slotID = -1);
	//! Will iterate over storages and try insert the item at first best match
	proto external bool TryInsertItem(IEntity item, EStoragePurpose purpose = EStoragePurpose.PURPOSE_ANY, InventoryOperationCallback cb = null);
	//! Will try to insert item at storage. If slotID -1 will try to insert in any free storage
	proto external bool TryInsertItemInStorage(IEntity item, BaseInventoryStorageComponent storage, int slotID = -1, InventoryOperationCallback cb = null);
	//! Same as insert, only should allow replacing item at storage slot, slotID is required
	proto external bool CanReplaceItem(IEntity item, BaseInventoryStorageComponent storage, int slotID);
	proto external bool TryReplaceItem(IEntity item, BaseInventoryStorageComponent storage, int slotID, InventoryOperationCallback cb = null);
	proto external bool CanRemoveItemFromStorage(IEntity item, BaseInventoryStorageComponent storage);
	//! Will try to remove item from storage
	proto external bool TryRemoveItemFromStorage(IEntity item, BaseInventoryStorageComponent storage, InventoryOperationCallback cb = null);
	proto external bool CanMoveItemToStorage(IEntity item, BaseInventoryStorageComponent to, int slotID = -1);
	//! Will try to move item from source storage to target storage
	proto external bool TryMoveItemToStorage(IEntity item, BaseInventoryStorageComponent to, int slotID = -1, InventoryOperationCallback cb = null);
	proto external bool CanSwapItemStorages(IEntity itemA, IEntity itemB);
	//! Will try to place itemA to itemB storage slot and itemB to itemA storage slot
	proto external bool TrySwapItemStorages(IEntity itemA, IEntity itemB, InventoryOperationCallback cb = null);
	proto external bool TryDeleteItem(IEntity item, InventoryOperationCallback cb = null);
	//! Spawn and insert into inventory
	//! if provided storage is null then most suitable storage would be chosen from owned storages
	proto external bool TrySpawnPrefabToStorage(ResourceName prefab, BaseInventoryStorageComponent storage = null, int slotID = -1,EStoragePurpose purpose = EStoragePurpose.PURPOSE_ANY, InventoryOperationCallback cb = null, int count = 1);
	//! Validate that the storage is near the player to eliminate potentual message tampering
	proto external bool ValidateStorageRequest( IEntity storageOwner );
	//! Get all managed storages
	proto external int GetStorages(out notnull array<BaseInventoryStorageComponent> outStorages, EStoragePurpose purpose = EStoragePurpose.PURPOSE_ANY);
	//! Get all items from all managed storages
	proto external int GetItems(out notnull array<IEntity> outItems, EStoragePurpose purpose = EStoragePurpose.PURPOSE_ANY);
	//! Returns true if item is in the storage, false otherwise
	proto external bool Contains(IEntity item);
	//! Will try to find suitable storage for item
	proto external BaseInventoryStorageComponent FindStorageForItem(IEntity item, EStoragePurpose purpose = EStoragePurpose.PURPOSE_ANY);
	//! Will try to find suitable storage for resource
	proto external BaseInventoryStorageComponent FindStorageForResource(ResourceName resourceName, EStoragePurpose purpose = EStoragePurpose.PURPOSE_ANY);
	//! returns first storage with empty space from provided storage hierarchy for provided item and provided storage purpose
	proto external BaseInventoryStorageComponent FindStorageForInsert(IEntity item, BaseInventoryStorageComponent fromStorage, EStoragePurpose purpose = EStoragePurpose.PURPOSE_ANY);
	//! returns first storage with empty space from provided storage hierarchy for provided resource and provided storage purpose
	proto external BaseInventoryStorageComponent FindStorageForResourceInsert(ResourceName resourceName, BaseInventoryStorageComponent fromStorage, EStoragePurpose purpose = EStoragePurpose.PURPOSE_ANY);
	//! Fast access to item count in inventory (returns only items that are stored in DEPOSIT storages)
	proto external int GetDepositItemCountByEntity(IEntity verifier, IEntity entity);
	//! Fast access to item count in inventory (returns only items that are stored in DEPOSIT storages)
	proto external int GetDepositItemCountByResource(IEntity verifier, ResourceName resourceName);
	/*! Find Item by providing InventorySearchPredicate object

				class SCR_SomeCrazyPredicate: InventorySearchPredicate
				{
					void SCR_SomeCrazyPredicate()
					{
						QueryComponentTypes.Insert(SCR_ConsumableComponent);
						QueryComponentTypes.Insert(SCR_SomeOptionalComponent);
						QueryAttributeTypes.Insert(SCR_SomeOptionalAttribute);
					}

					override protected bool IsMatch(BaseInventoryStorageComponent storage, IEntity item, array<GenericComponent> queriedComponents, array<BaseItemAttributeData> queriedAttributes)
					{
						// at this point respective components and attributes are guaranteed to exist at respective type index
						SCR_ConsumableComponent optionalConsumable = SCR_ConsumableComponent.Cast(queriedComponents[0]);
						SCR_SomeOptionalComponent optionalComponent = SCR_SomeOtherComponent.Cast(queriedComponents[1]);
						SCR_SomeOptionalAttribute optionalAttribute = SCR_SomeOptionalAttribute.Cast(queriedAttributes[0]);
						return optionalConsumable.IsHealthType() && optionalComponent.ReadyToBeUsed() && optionalAttribute.m_bMyProperty;
					}
				}
				Somewhere down the road...
				IEntity FindCrazyItem()
				{
					// If you plan on constantly using this predicate, consider storing it as member variable to save on additional allocations
					SCR_SomeCrazyPredicate predicate = new SCR_SomeCrazyPredicate();
					IEntity crazyThing = inventoryStorageManagerComponent.FindItem(predicate, EStoragePurpose.PURPOSE_DEPOSIT);
					Print(crazyThing);
					return crazyThing;
				}
			*/
	proto external IEntity FindItem(InventorySearchPredicate predicate, EStoragePurpose purpose = EStoragePurpose.PURPOSE_DEPOSIT);
	proto external int FindItems(out notnull array<IEntity> foundItems, InventorySearchPredicate predicate, EStoragePurpose purpose = EStoragePurpose.PURPOSE_DEPOSIT);
	/*! Find Item by specifying necessary component types (eg returns first Entity that contains all of the provided components)
	IEntity FindGrenadeInDeposit()
	{
		array<typename> components = {};
		components.Insert(WeaponComponent);
		components.Insert(GrenadeMoveComponent);
		IEntity grenade = inventoryStorageManagerComponent.FindItemWithComponents(components, EStoragePurpose.PURPOSE_DEPOSIT);
		Print(grenade);
		return grenade;
	}
	*/
	proto external IEntity FindItemWithComponents(array<typename> componentsQuery, EStoragePurpose purpose = EStoragePurpose.PURPOSE_DEPOSIT);
	//! Find Items by specifying necessary component types (eg all Entities that contain all of the provided components), returns count of found items
	proto external int FindItemsWithComponents(out notnull array<IEntity> foundItems, array<typename> componentsQuery, EStoragePurpose purpose = EStoragePurpose.PURPOSE_DEPOSIT);
	proto external int CountItem(InventorySearchPredicate predicate, EStoragePurpose purpose = EStoragePurpose.PURPOSE_DEPOSIT);
	proto external int GetMagazineCountByWeapon(BaseWeaponComponent weapon);
	proto external int GetMagazineCountByMuzzle(IEntity verifier, BaseMuzzleComponent pMuzzle);
	proto external int GetGrenadesCount();

	// callbacks

	// ------------------------------------------------------------------------------------------
	event void OnDelete(IEntity owner);
	//! Callback when item is added (will be performed locally after server completed the Insert/Move operation)
	event protected void OnItemAdded(BaseInventoryStorageComponent storageOwner, IEntity item);
	//! Callback when item is removed (will be performed locally after server completed the Remove/Move operation)
	event protected void OnItemRemoved(BaseInventoryStorageComponent storageOwner, IEntity item);
	//! Will be called when new storage is registered at manager
	event protected void OnStorageAdded(BaseInventoryStorageComponent storage);
	//! Will be called when storage is unregistered from manager
	event protected void OnStorageRemoved(BaseInventoryStorageComponent storage);
	//! Override this method and fill in storagesToAdd array if you have some storages you'd like to register to manager as owned at initialization time
	event protected void FillInitialStorages(out array<BaseInventoryStorageComponent> storagesToAdd);
	//! Override this method and fill in prefabsToSpawn array if you have some items you'd like to store in inventory at initialization time
	event protected void FillInitialPrefabsToStore(out array<ResourceName> prefabsToSpawn);
	//! Virtual method that allows verification of item removal as a result of move operation on inventory level
	event protected bool ShouldForbidRemoveByInstigator(InventoryStorageManagerComponent instigatorManager, BaseInventoryStorageComponent fromStorage, IEntity item) { return false; };
}

/*!
\}
*/
