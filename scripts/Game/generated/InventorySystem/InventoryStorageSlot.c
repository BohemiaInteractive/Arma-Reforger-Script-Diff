/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup InventorySystem
\{
*/

class InventoryStorageSlot: EntitySlotInfo
{
	proto external bool ScriptOnRplSave(ScriptBitWriter writer);
	proto external bool ScriptOnRplLoad(ScriptBitReader reader);
	proto external vector GetInspectionWidgetOffset();
	//! Either current slot is locked
	proto external bool IsLocked();
	//! Get slot id
	proto external int GetID();
	//! EntityContainer which created the slot
	proto external GenericComponent GetParentContainer();
	//! storage that our slot belongs to
	proto external BaseInventoryStorageComponent GetStorage();
}

/*!
\}
*/
