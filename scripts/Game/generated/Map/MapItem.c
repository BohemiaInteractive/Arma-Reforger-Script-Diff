/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Map
\{
*/

class MapItem
{
	/*!
	\brief Get owner entity
	*/
	proto external IEntity Entity();
	/*!
	\brief Get descriptor component
	*/
	proto external MapDescriptorComponent Descriptor();
	/*!
	\brief Mark this to be recycled (removed from map forever)
	Once you call this, when item loose all references it will be auto removed from map
	*/
	proto external void Recycle();
	/*!
	\brief Enable/ disable visibility in map
	*/
	proto external void SetVisible( bool enable );
	/*!
	\brief Returns true of visibility set On
	*/
	proto external bool IsVisible();
	/*!
	\brief Enable/ disable editor-related flags (Editor's descriptor)
	*/
	proto external void SetEditor( bool enable );
	/*!
	\brief Returns true of descriptor belongs to Editor
	*/
	proto external bool IsEditor();
	/*!
	\brief Enable/ disable hover-state in map
	*/
	proto external void SetHovering( bool enable );
	/*!
	\brief Returns true if hover-state set On
	*/
	proto external bool IsHovering();
	/*!
	\brief Enable/ disable highlighted-state in map
	*/
	proto external void SetHighlighted( bool enable );
	/*!
	\brief Returns true if highlighted-state set On
	*/
	proto external bool IsHighlighted();
	/*!
	\brief Enable/ disable selection state in map
	*/
	proto external void Select( bool enable );
	/*!
	\brief Returns true of selected set On
	*/
	proto external bool IsSelected();
	/*!
	\brief Marks the map item for using MapLayer's link props
	*/
	proto external void SetLayerLinkProps( bool enable );
	/*!
	\brief Returns true if the map item is marked for using MapLayer's link props
	*/
	proto external bool IsUseLayerLinkProps();
	/*!
	\brief Get currently assigned text
	*/
	proto external string GetDisplayName();
	/*!
	\brief Set new text
	*/
	proto external void SetDisplayName(string name, array<string> params = null);
	/*!
	\brief Adds a new link between the caller and item (parameter), returns the link created.
	*/
	proto external MapLink LinkTo( MapItem item );
	/*!
	\brief Determine group type (EMapDescriptorGroup) of item
	*/
	proto external int GetGroupType();
	/*!
	\brief Assign different group type (EMapDescriptorGroup) of item
	*/
	proto external void SetGroupType( int type );
	/*!
	\brief Retrieve priority of item
	*/
	proto external int GetPriority();
	/*!
	\brief Assign different priority to item
	*/
	proto external void SetPriority( int iPriority );
	//! Remove link (or all links if pItem==nullptr)
	proto external void UnLink( MapItem item );
	/*!
	\brief Get current links
	*/
	proto external int GetLinks( out notnull array<MapLink> outItems );
	/*!
	\brief Clear all links
	*/
	proto external void ClearLinks();
	/*!
	\brief Get currently assigned Info text
	*/
	proto external string GetInfoText();
	/*!
	\brief Set new Info text
	*/
	proto external void SetInfoText(string name, array<string> params = null);
	/*!
	\brief Get current image def
	*/
	proto external string GetImageDef();
	/*!
	\brief Set image def
	*/
	proto external void SetImageDef( string name );
	/*!
	\brief Get range size (radio signal for example)
	*/
	proto external float GetRange();
	/*!
	\brief Set range size (radio signal for example)
	*/
	proto external void SetRange( float range );
	/*!
	\brief Set angle
	*/
	proto external void SetAngle( float angle );
	/*!
	\brief Get current angle
	*/
	proto external float GetAngle();
	/*!
	\brief Get current position (script does not support vector2d)
	*/
	proto external vector GetPos();
	/*!
	\brief Set new position X, Z
	*/
	proto external void SetPos( float x, float z );
	/*
	\brief Get access to object local properties
	*/
	proto external MapDescriptorProps GetProps();
	/*!
	\brief Updates and setups properties of a specific MapItem.
	*/
	proto external void SetProps(MapDescriptorProps newProps);
	/*!
	\brief Determine type (EMapDescriptorType) of item
	*/
	proto external int GetBaseType();
	/*!
	\brief Assign different type (EMapDescriptorType) of item
	*/
	proto external void SetBaseType( int type );
	/*!
	\brief Get currently assigned faction index
	0 == Neutral
	1 == East
	2 == West
	*/
	proto external int GetFactionIndex();
	/*!
	\brief Set new faction index
	0 == Neutral
	1 == East
	2 == West
	*/
	proto external void SetFactionIndex( int iIndex );
}

/*!
\}
*/
