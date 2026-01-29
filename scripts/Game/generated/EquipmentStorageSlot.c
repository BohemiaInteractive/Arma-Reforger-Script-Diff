/*
===========================================
Do not modify, this script is generated
===========================================
*/

class EquipmentStorageSlot: InventoryStorageSlot
{
	proto external bool CanAttachItem(IEntity item);
	proto external bool IsOccluded();

	// callbacks

	//! override this method to handle custom behavior when state is changed
	//! return true if you don't want slot to handle attached item visibility changes
	event protected bool OnOccludedStateChanged(bool occluded) { return false; };
}
