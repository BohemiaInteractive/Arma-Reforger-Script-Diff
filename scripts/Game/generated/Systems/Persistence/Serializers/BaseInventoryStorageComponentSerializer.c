/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence_Serializers
\{
*/

class BaseInventoryStorageComponentSerializer: ScriptedComponentSerializer
{
	/*
	Any entity that could not be loaded at the correct slot from inital items is inserted into the handler array so the order in which they will be inserted can be manipulated.
	This is necessary if the slot order inside the storage is not the same as the dependency between e.g. weapon attachments.
	*/
	event protected void HandleInsertionOrder(inout array<BaseInventoryStorageComponentSerializerSlot> slots);
}

/*!
\}
*/
