/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

sealed class HitZoneContainerComponent: GameComponent
{
	proto external IEntity GetOwner();
	proto external HitZone GetDefaultHitZone();
	//! Get the HitZone that matches the provided collider name. Searches the entire hierarchy.
	//! Returns the hitzone matching the provided name, or null if the hitzone can't be found.
	proto external HitZone GetHitZoneByName(string hitZoneName);
	//! Get the HitZone that matches the provided collider ID. Only searches on the component.
	//! Returns the hitzone matching the collider ID, or null if the hitzone can't be found.
	proto external HitZone GetHitZoneByColliderID(int colliderID);
	//! Clears and fills the specified outHitZones array with all HZs that are attached to colliderIDs.
	//! Returns the hitzones matching those collider IDs
	//! Returns the count of elements that were filled into the array.
	proto external int GetHitZonesByColliderIDs(out notnull array<HitZone> outHitZones, notnull array<int> colliderIDs);
	proto external HitZone GetHitZone(string colliderName);
	//! Counts the number of hitzones in this entity.
	//! Use instead of GetAllHitZones when you only need to get the count because this function does not allocate memory.
	proto external int CountAllHitZones();
	//! Counts the number of hitzones in this entity and its children.
	//! Use instead of GetAllHitZonesInHierachy when you only need to get the count because this function does not allocate memory.
	proto external int CountAllHitZonesInHierarchy();
	//! Clears and fills the specified outHitZones array with all HZs in this entity
	//! Returns the count of elements that were filled into the array.
	proto external int GetAllHitZones(out notnull array<HitZone> outHitZones);
	//! Clears and fills the specified outHitZones array with all HZs in this entity and its children.
	//! Returns the count of elements that were filled into the array.
	proto external int GetAllHitZonesInHierarchy(out notnull array<HitZone> outHitZones);
	proto external HitZoneContainerComponent GetParentHitZoneContainer();

	// callbacks

	//! Use it to save bandwidth in some default conditions. Use it if Save and Load are overriden.
	event protected bool HasDataToReplicate();
	// --------------------------------------------------------------------------------
	event protected bool OnRplSave(ScriptBitWriter writer);
	event protected bool OnRplLoad(ScriptBitReader reader);
}

/*!
\}
*/
