/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup InventorySystem
\{
*/

class ItemPreviewManagerEntityClass: GenericEntityClass
{
}

class ItemPreviewManagerEntity: GenericEntity
{
	//! Set item to preview, optional override for PreviewRenderAttributes and forcing refresh of preview item (usualy not needed since system is taking care of updating hierarchy when entity hierarchy is changed)
	//! maxLiftTime sets the maximum time that the item preview can remain in cache after being unused. if maxLifeTimeMS <= 0, the entry will always stay in cache
	proto external void SetPreviewItem(ItemPreviewWidget widget, IEntity item, PreviewRenderAttributes attributes = null, bool forceRefresh = false, int maxLifeTimeMS = 30000);
	proto external void SetPreviewItemFromPrefab(ItemPreviewWidget widget, ResourceName prefabResource, PreviewRenderAttributes attributes = null, bool forceRefresh = false, int maxLifeTimeMS = 30000);
	proto external IEntity ResolvePreviewEntityForPrefab(ResourceName prefabResource);
}

/*!
\}
*/
