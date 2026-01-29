/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup InventorySystem
\{
*/

class EquipedLoadoutStorageComponentClass: ScriptedBaseInventoryStorageComponentClass
{
}

class EquipedLoadoutStorageComponent: ScriptedBaseInventoryStorageComponent
{
	//! Get the first slot that satisfies the condition : "slot area type is inherited by pAreaType".
	proto external LoadoutSlotInfo GetSlotFromArea(typename pAreaType);
	/*!
	Get the slots that satisfies the condition : "slot area type is inherited by pAreaType".

	\param outSlots Array where slots will be added.

	\return How many elements have been added to the array.
	*/
	proto external int GetSlotsFromArea(typename pAreaType, out array<ref LoadoutSlotInfo> outSlots);
	//! Get the first cloth entity that satisfies the condition : "slot area type is inherited by pAreaType AND slot has attached entity".
	proto external IEntity GetClothFromArea(typename pAreaType);
}

/*!
\}
*/
