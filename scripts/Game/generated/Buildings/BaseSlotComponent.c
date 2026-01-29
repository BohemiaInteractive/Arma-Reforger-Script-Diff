/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Buildings
\{
*/

class BaseSlotComponent: GameComponent
{
	//! Returns the current slot info.
	proto external EntitySlotInfo GetSlotInfo();
	//! Returns the current attached entity.
	proto external IEntity GetAttachedEntity();

	// callbacks

	//! Runs every time an entity is attached to the slot.
	event void OnAttachedEntity(IEntity ent);
	//! Runs every time an entity is detached from the slot.
	event void OnDetachedEntity(IEntity ent);
}

/*!
\}
*/
