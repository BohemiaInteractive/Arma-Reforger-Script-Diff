/** @ingroup Editable_Entities
*/ 

/*!
State of editable entity (SCR_EditableEntityComponent) saved in mission save file.

Not related to EEditableEntityState, although there may be overlapping values.
*/
enum EEditableEntitySaveFlag
{
	PLAYER			= 1 << 0, ///< Entity is a player
	DESTROYED		= 1 << 1, ///< Entity was destroyed
	NOT_SPAWNED		= 1 << 2, ///< Children that were supposed to be spawned by SCR_EditorLinkComponent were not spawned yet (e.g., in Conflict compositions)
}