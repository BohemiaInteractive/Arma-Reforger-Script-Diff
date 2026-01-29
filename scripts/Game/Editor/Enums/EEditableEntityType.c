/** @ingroup Editable_Entities
*/ 

//! Defines type of SCR_EditableEntityComponent. Assigned automatically based on IEntity inheritance.
enum EEditableEntityType
{
	GENERIC, ///< Default entity type (inherited from GenericEntity)
	GROUP, ///< AI Group (inherited from AIGroup)
	CHARACTER, ///< Human character (inherited from ChimeraCharacter)
	VEHICLE, ///< Vehicle with crew slots inside (inherited from Vehicle)
	WAYPOINT, ///< AI Waypoint (inherited from AIWaypoint)
	ITEM, ///< Inventory item
	COMMENT, ///< Comment visible only in the editor
	FACTION, ///< Faction registered in FactionManager
	TASK, /// Player task (inherited from SCR_BaseTask)
	SYSTEM, /// Scripted mechanics
	SLOT /// Editor slots for composition building
};