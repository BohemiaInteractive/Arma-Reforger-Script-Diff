/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Map
\{
*/

class MapDrawingProps
{
	private void MapDrawingProps() { }
	private void ~MapDrawingProps() { }

	proto external void SetVisible(bool visibility);
	proto external bool IsVisible();
	proto external void SetHollow(bool hollow);
	proto external bool IsHollow();
	proto external void SetShapeType( EMapDrawingShapeType type );
	proto external void SetDetail( int iDetail );
	proto external int GetDetail();
	proto external void SetOutlineType(EMapLineType type );
	proto external float GetOutlineWidth();
	proto external void SetOutlineWidth( float outlineWidth );
	proto ref Color GetOutlineColor();
	proto external void SetOutlineColor( notnull Color outsideColor );
	proto ref Color GetInteriorColor();
	proto external void SetInteriorColor( notnull Color interiorColor );
}

/*!
\}
*/
