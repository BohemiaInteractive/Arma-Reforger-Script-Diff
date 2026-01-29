/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components_InventorySystem
\{
*/

class InventoryItemComponent: GameComponent
{
	ref ScriptInvoker<bool> m_OnLockedStateChangedInvoker = new ScriptInvoker<bool>();

	ref ScriptInvoker<InventoryStorageSlot, InventoryStorageSlot> m_OnParentSlotChangedInvoker = new ScriptInvoker<InventoryStorageSlot, InventoryStorageSlot>();

	//! Returns Entity owner of current component instance
	proto external IEntity GetOwner();
	//! Returns locked state of item (for both user and system locks)
	proto external bool IsLocked();
	proto external bool IsUserLocked();
	proto external bool IsSystemLocked();
	// Get slot where item is located (returns null if none)
	proto external InventoryStorageSlot GetParentSlot();
	proto external ItemAttributeCollection GetAttributes();
	//! Hide owner entity
	proto external void HideOwner();
	//! Show owner entity
	proto external void ShowOwner();
	//! Set the entity to be traceable or not
	proto external void SetTraceable(bool traceable);
	//! Disable owners physical interactions
	proto external void DisablePhysics();
	//! Enable owners physical interactions
	proto external void EnablePhysics();
	//! Enable/Disable entity active state
	proto external void ActivateOwner(bool active);
	//! Returns the current additional weight
	proto external float GetAdditionalWeight();
	/*!
	*IMPORTANT* Could only be called on the master.
	Set the additional weight of the item
	\param additionalWeight additional weight in kg
	*/
	proto external void SetAdditionalWeight(float additionalWeight);
	//! Returns the weight + the additional weight
	proto external float GetTotalWeight();
	//! Returns the current additional volume
	proto external float GetAdditionalVolume();
	/*!
	*IMPORTANT* Could only be called on the master.
	Set the additional volume of the item
	\param additionalVolume additional volume in cm3
	*/
	proto external void SetAdditionalVolume(float additionalVolume);
	//! Returns the volume + the additional volume
	proto external float GetTotalVolume();
	/*!
	*IMPORTANT* This function is heavy in performance because of the multiple traces used inside the function.
	Place the item on the ground
	\param callerEntity The entity used for the rotation calculation. Should be kept to null per default
	*/
	proto external void PlaceOnGround(IEntity callerEntity = null);
	//! returns true if item should be hidden
	proto external bool IsHiddenInVicnity();
	//! Returns UI info of this item
	proto external UIInfo GetUIInfo();
	//! Set external lock on item
	//! optional caller is entity that manipulates with item and has inventory manager present (usually character)
	proto external bool RequestUserLock(IEntity caller, bool locked);
	/*!
	Convinience method
	Finds first occurance of the coresponding attribute data object in owned PrefabData AttributeCollection.
	\param typeName type of the component
	*/
	proto external BaseItemAttributeData FindAttribute(typename typeName);
	//! Creates preview entity in the provided world
	proto external IEntity CreatePreviewEntity(BaseWorld world, int camera);

	// callbacks

	// It should be called with a right return value to change the visibility of entities with hierarchy according to needs.
	event protected bool ShouldChangeVisibilityOfHierarchy() { return false; };
	//! implement custom visibility behavior in vicinity
	event protected bool ShouldHideInVicinity();
	//! Override final transformation of dropped item, return true in case transformation should be applied
	event protected bool OverridePlacementTransform(IEntity caller, out vector computedTransform[4]);
	//! Override this function if for example physics should be enabled or item's owner should be activated when an item is placed on ground.
	event protected void OnPlacedOnGround ();
	event protected void OnPostInit(IEntity owner);
	event protected void OnDelete(IEntity owner);
	event private void OnLockedStateChanged(bool nowLocked) { if (m_OnLockedStateChangedInvoker) m_OnLockedStateChangedInvoker.Invoke(nowLocked); };
	event private void OnParentSlotChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot) { if (m_OnParentSlotChangedInvoker) m_OnParentSlotChangedInvoker.Invoke(oldSlot, newSlot); };
}

/*!
\}
*/
