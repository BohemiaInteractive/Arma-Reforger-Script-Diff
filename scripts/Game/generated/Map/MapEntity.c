/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Map
\{
*/

class MapEntityClass: GenericEntityClass
{
}

class MapEntity: GenericEntity
{
	proto external void UpdateTexts();
	//! Absorb world data and create precalculated form for visualisation
	proto external void AbsorbData();
	//! Starting offset of the map.
	proto external vector Offset();
	//! Terrain dimensions (x, height = maxElevation-minElevation, z)
	proto external vector Size();
	//! Terrain unit scale
	proto external float TerrainUnitScale();
	//! Set selection to visualise (from -> to)
	proto external void SetSelection( vector start, vector end );
	//! Reset selection to visualise (from -> to)
	proto external void ResetSelection();
	//! Set frame to visualise (from -> to)
	proto external void SetFrame( vector start, vector end );
	//! Set new zoom
	proto external void ZoomChange( float level );
	//! Set new pos of map
	proto external void PosChange( float x, float y );
	//! Set new pos of cursor
	proto external void CursorChange( float x, float y );
	//! Minimal map elevation
	proto external float MinElevation();
	//! Maximal map elevation
	proto external float MaxElevation();
	//! Get elevation at coords
	proto external float ElevationAt( vector pos );
	//! Show/ Hide diag
	proto external void ShowDiag( bool bEnable );
	/*!
	\brief Create custom representation from script
	This representation has no entity and you have to support position and orientation yourself (watch MapItem Api)
	*/
	proto external MapItem CreateCustomMapItem();
	//! Count of visible
	proto external int CountOfVisible();
	//! Visible by index
	proto external MapItem GetVisible( int iIndex );
	//! Gets all the visible map entities in the current view
	proto external int GetVisibleAll( out notnull array<MapItem> outItems);
	//! Get entities in rectangle. Flips the screen y-axis before performing its operation.
	proto external int GetInsideRect( out notnull array<MapItem> outItems, vector from, vector to );
	//! Get entities in circle. Flips the screen y-axis before performing its operation.
	proto external int GetInsideCircle( out notnull array<MapItem> outItems, vector origin, float range );
	//! Get entities in rectangle. Flips the screen y-axis before performing its operation.
	proto external MapItem GetClose( vector origin, float range );
	//! Get all selected entities
	proto external int GetSelected( out notnull array<MapItem> outItems );
	//! Get entities by descriptor type
	proto external int GetByType( out notnull array<MapItem> outItems, /*EMapDescriptorType*/ int type );
	//! Reset all entity selection
	proto external void ResetSelected();
	//! Reset all entity hovering tag
	proto external void ResetHovering();
	//! Reset all entity highlighted tag
	proto external void ResetHighlighted();
	//! Show/ hide multiple items by type (EMapDescriptorType) of item
	proto external void ShowMultiple( int type, bool bEnable );
	//! Show/ hide editor items
	proto external void ShowEditor( bool bEnable );
	//! Show/ hide debug items (camera, map sizes)
	proto external void ShowDebug( bool bEnable );
	//! Enable/ Disable visualisation
	proto external void EnableVisualisation( bool bValue );
	//! Enable/ Disable grid visibility
	proto external void EnableGrid( bool bValue );
	//! Enable/ Disable overlay
	proto external void EnableOverlay( bool bValue );
	//! Enable/ Disable legend
	proto external protected void EnableLegend( bool bValue );
	//! Clear layer setup + set new layer count
	proto external void InitializeLayers(int count, int factionSize = 4);
	//! Get layer count
	proto external int LayerCount();
	//! Get layer by Index
	proto external MapLayer GetLayer( int index );
	//! Set active layer
	proto external void SetLayer( int index );
	//! Returns -1 if no valid index previously set
	proto external int GetLayerIndex();
	//! Sets corresponding multiple imageset indices for MapDescriptors usage
	proto external void SetImagesetMapping(notnull array<int> values);
}

/*!
\}
*/
