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
	//! Returns the default hitzone.
	//! In play mode, if there are no hitzones, the component is not created. Otherwise, a default hitzone is guaranteed to be present.
	//! In edit mode, the component is always created even if no hitzone is defined. In this case the hitzone might return null.
	proto external HitZone GetDefaultHitZone();
	//! Clears and fills the specified outHitZones array with all HZs in this entity.
	//! The ID of a hitzone is its position on the array returned by GetAllHitZones
	//! Returns the count of elements that were filled into the array.
	proto external int GetHitZonesByID(out notnull array<HitZone> outHitZones, notnull array<int> hitZoneIDs);
	//! Clears and fills the specified outHitZones array with all HZs in this entity and its children.
	//! The ID of a hitzone is its position on the array returned by GetAllHitZones
	//! Returns the count of elements that were filled into the array.
	proto external int GetHitZonesByIDInHierarchy(out notnull array<HitZone> outHitZones, notnull array<int> hitZoneIDs);
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
	//! Gets called if a child HitZoneContainerComponent registers to a parent HitZoneContainerComponent
	event protected void OnRegisteredToParent(notnull HitZoneContainerComponent outHitZoneContainerComponent);
	//! Gets called if a child HitZoneContainerComponent unregisters from a parent HitZoneContainerComponent
	event protected void OnUnRegisteredFromParent(notnull HitZoneContainerComponent outHitZoneContainerComponent);
}

/*!
\}
*/
