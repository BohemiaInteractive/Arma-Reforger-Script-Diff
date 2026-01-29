/** @ingroup Editable_Entities
*/ 

/*!
Auto-registration type.
When SCR_EditableEntityComponent is created, this decides whether it should be added to the list of editable entities or not.
When registered, this value is set to -1, which then marks the entity is registered.
Upon unregistering, the value is set to ALWAYS, 
*/
enum EEditableEntityRegister
{
	ALWAYS, ///< Entity is always registered automatically
	WHEN_SPAWNED, ///< Entity is registered only when spawned dynamically, not when present in the world from the beginning
	NEVER ///< Entity is never registered automatically, it must be done manually using SCR_EditableEntityComponent.Register()
};