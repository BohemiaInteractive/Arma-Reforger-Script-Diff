/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup InventorySystem_InventoryTask
\{
*/

class ScriptedInventoryTask: BaseInventoryTask
{
	proto external void SetSlotOperation(InventoryStorageSlot slot, IEntity item, bool removeOp);

	// callbacks

	//! Called upon task initialization owner is Entity with attached InventoryStorageManagerComponent (when adding to tasks queue)
	event protected void Initialize(IEntity owner, InventoryStorageSlot slot, IEntity item, bool removeOp);
	//! Called before first Update, when task is start executing
	event protected void Start(InventoryStorageSlot slot, IEntity item, bool removeOp);
	//! Called on update
	event protected void Update(float dt, InventoryStorageSlot slot, IEntity item, bool removeOp);
	//! Task should return true when done, otherwise it won't be released by manager
	event protected bool IsComplete(InventoryStorageSlot slot, IEntity item, bool removeOp) { return true; };
	//! Called only when I have completed
	event protected void OnTaskCompleted();
}

/*!
\}
*/
