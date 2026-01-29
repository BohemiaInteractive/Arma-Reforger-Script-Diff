/** @ingroup Editable_Entities
*/ 

//! Unique flags of the entity.
enum EEditableEntityFlag
{
	PLACEABLE			= 1 << 0, ///< Entity is not available in the content browser (read by class EditableEntityCollector, not in run-time)
	VIRTUAL				= 1 << 1, ///< Entity is represented by virtual objects that have to be updated
	HAS_FACTION			= 1 << 2, ///< Entity can have faction assigned
	LOCAL				= 1 << 3, ///< Entity is not a legit editable entity, but merely a light-weight preview posing as an editable entity
	NON_INTERACTIVE		= 1 << 4, ///< Entity cannot be selected and transformed
	SLOT				= 1 << 5, ///< When extending the entity with other entities, check also for prefab parents (slots usually inherit from base classes for cross-compatibility)
	HORIZONTAL			= 1 << 6, ///< Don't orient the entity to terrain normal when transforming
	STATIC_POSITION		= 1 << 7, ///< Entity can be placed only on its pre-defined position
	LAYER				= 1 << 8, ///< Can the entity be 'entered' as a layer?
	GAME_HIERARCHY		= 1 << 9, ///< Should the orignal game hierarchy be preserved instead of flattened upon init?
	ORIENT_CHILDREN		= 1 << 10, ///< When transforming the entity, children wil be oriented to terrain instead of being glued to the entity
	NON_DELETABLE		= 1 << 11, ///< Entity cannot be deleted
	IGNORE_LAYERS		= 1 << 12, ///< When enabled, the entity will be shown even when not in current layer
	INDIVIDUAL_CHILDREN	= 1 << 13, ///< Entity children can be interacted with individually, even when the entity is not the current layer
	NON_SERIALIZABLE	= 1 << 14, ///< When enabled, entity will not be serialized when session is saved
	HAS_AREA			= 1 << 15, ///< Entity has an area, e.g., a trigger or a spawn point
	DIRTY_HIERARCHY		= 1 << 16, ///< Entity was changed by the user
	LINKED_CHILDREN		= 1 << 17, ///< Entity spawns children using SCR_EditorLinkComponent
	SPAWN_UNFINISHED	= 1 << 18, ///< Should entity spawning be handled by Editor or some external system
	FREE_ROAM_BUILDING_AI	= 1 << 19, ///< AI spawned by Free Roam Building.
};