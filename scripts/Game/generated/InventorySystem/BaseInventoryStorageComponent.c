/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup InventorySystem
\{
*/

class BaseInventoryStorageComponentClass: InventoryItemComponentClass
{
}

class BaseInventoryStorageComponent: InventoryItemComponent
{
	/*!
	*IMPORTANT* Should be called upon initialization of slot instance
	 provide newly created slot and desired slot ID
	 */
	proto external sealed protected void SetupSlotHooks(InventoryStorageSlot ownedSlot, int slotID);
	/*!
	*IMPORTANT* Should be called before transfering ownership of holded slot instance
	 In majority of cases should be unnecessary (when storage manages creation and destruction of slots on its own)
	*/
	proto external sealed protected void ReleaseSlotHooks(InventoryStorageSlot ownedSlot);
	/*!
	Check if an item is inside this storage.
	\param item Item which meeds to be searched.
	\return Returns true if the item is inside this storage.
	*/
	proto external sealed bool Contains(IEntity item);
	/*!
	Find the item slot.
	\param item Item which meeds to be searched.
	\return Returns the slot otherwise null.
	*/
	proto external sealed InventoryStorageSlot FindItemSlot(IEntity item);
	//! Returns the number of slots in this storage.
	proto external int GetSlotsCount();
	/*!
	Return slot for specified id
	\param slotID Slot ID.
	\return Returns the slot.
	*/
	proto external sealed InventoryStorageSlot GetSlot(int slotID);
	/*!
	Get item entity at slot ID.
	\param slotID Slot ID
	\return Returns the entity instance or null if slot empty.
	*/
	proto external sealed IEntity Get(int slotID);
	/*!
	Get item at slot ID.
	\param slotID Slot ID
	\return Returns the item component of the slot entity or null if slot empty.
	*/
	proto external sealed InventoryItemComponent GetItem(int slotID);
	// Returns all stored items in this storage and optionally all child storage components
	proto external sealed int GetAll(out notnull array<IEntity> outItems, bool includeChildComponents = true);
	// Returns storage priority
	proto external sealed int GetPriority();
	proto external sealed EStoragePurpose GetPurpose();
	// Search for slot where item can be inserted
	proto external InventoryStorageSlot FindSuitableSlotForItem(IEntity item);
	//! does current storage serves as a compartment of other storage
	proto external bool IsCompartment();
	//! Fills array with attached items, optionally including items from child storage components
	proto external void GetOwnedItems(out notnull array<InventoryItemComponent> outItemsComponents, bool includeChildComponents = true);
	/*!
	Fills array with attached storages up to provided hierarchy depth
	for every top level storage depth with even number will reach compartments level and odd will reach storages attached to slots (including compartment slots)
	\param outStorages - array to be filled with storages
	\param depth - at what depth level should be storages collected
	\param includeHierarchy - should storages from upper depth level be included in array
	\return true if depth was reached
	*/
	proto external bool GetOwnedStorages(out notnull array<BaseInventoryStorageComponent> outStorages, int depth, bool includeHierarchy);
	//! Get the weight of a prefab
	proto external float GetWeightFromResource(ResourceName resourceName);
	// Return dimension limits for storage
	proto external vector GetMaxDimensionCapacity();
	// Convinience method, returns volume calculated from dimension limits
	proto external float GetMaxVolumeCapacity();
	//! Returns amount of space occupied by attached items
	proto external float GetOccupiedSpace();
	//! Performs dimension validation.
	proto external bool PerformDimensionValidation(IEntity item);
	//! Performs item volume and (optionally) dimension validation.
	proto external bool PerformVolumeValidation(IEntity item, bool includeDimensionValidation = true);
	//! Performs prefab volume and (optionally) dimension validation.
	proto external bool PerformVolumeValidationForResource(ResourceName resourceName, bool includeDimensionValidation = true);
	//! Get an estimate of how many times the item can fit in the inventory
	proto external int GetEstimatedCountFitForItem(IEntity item);
	//! Get an estimate of how many times the resource can fit in the inventory
	proto external int GetEstimatedCountFitForResource(ResourceName resourceName);

	// callbacks

	/*!
	It should return true or false depending if the scripter wants to override the behavior of the C++ function or not.
	*IMPORTANT* This should contains one line with return true/false; The result is cached, so this function can't be made dynamic
	*/
	event bool OnOverrideCanStoreItem() { return false; };
	/*!
	It should return true or false depending if the scripter wants to override the behavior of the C++ function or not.
	*IMPORTANT* This should contains one line with return true/false; The result is cached, so this function can't be made dynamic
	*/
	event bool OnOverrideCanStoreResource() { return false; };
	/*!
	It should return true or false depending if the scripter wants to override the behavior of the C++ function or not.
	*IMPORTANT* This should contains one line with return true/false; The result is cached, so this function can't be made dynamic
	*/
	event bool OnOverrideCanRemoveItem() { return false; };
	/*!
	It should return true or false depending if the scripter wants to override the behavior of the C++ function or not.
	*IMPORTANT* This should contains one line with return true/false; The result is cached, so this function can't be made dynamic
	*/
	event bool OnOverrideCanReplaceItem() { return false; };
	/*!
	Will be called when item is added to slot.
	*IMPORTANT* This is called after the C++ event.
	*/
	event protected void OnAddedToSlot(IEntity item, int slotID);
	/*!
	Will be called when item is removed from slot
	*IMPORTANT* This is called after the C++ event.
	*/
	event protected void OnRemovedFromSlot(IEntity item, int slotID);
	//! Usually any slot that item can be inserted to
	event protected InventoryStorageSlot GetEmptySlotForItem(IEntity item);
	//! Implemented logics for can insert here, Manager will provide slotID of -1 in case slot is irrelevant.
	event bool CanStoreItem(IEntity item, int slotID) { return true; };
	//! Implemented logics for can insert here, Manager will provide slotID of -1 in case slot is irrelevant.
	event bool CanStoreResource(ResourceName resourceName, int slotID) { return true; };
	//! Implemented logics for can remove here,
	event bool CanRemoveItem(IEntity item) { return true; };
	//! Implemented logics for can replace to nextItem at slotID,
	event bool CanReplaceItem(IEntity nextItem, int slotID) { return true; };
	//! Should Return slots count
	event protected int GetSlotsCountScr();
	//! Should Return slot for specified id
	event protected InventoryStorageSlot GetSlotScr(int slotID);
	//! Called locally per instance, implement remove logics here
	event protected ref BaseInventoryTask RemoveItem(IEntity item);
	/*!
	Called locally per instance, implement insertion logics here, Manager will provide slotID of -1 in case slot is irrelevant.
	justSpawned parameter says that we are inserting the item right after it was spawned and when this is true on server,
	it usually means that the owner of the slot(f.e. client character) might not have the entity streamed-in when this insertion happens.
	*/
	event protected ref BaseInventoryTask InsertItem(IEntity item, int slotID, bool justSpawned);
	//! Will be called to estimate if storage children has to be included in preview
	event protected bool ShouldPreviewAttachedItems();
	/*!
	Will be called when manager is changed, manager can be null if there is no manager in hierarchy (item drop in world).
	*IMPORTANT* This is called after the C++ event.
	*/
	event protected void OnManagerChanged(InventoryStorageManagerComponent manager);
	//! Virtual method for updating the UI when an item is removed/added
	event protected void UpdateUI();
}

/*!
\}
*/
