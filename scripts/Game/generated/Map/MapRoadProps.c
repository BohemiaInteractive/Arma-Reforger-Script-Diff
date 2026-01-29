/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Map
\{
*/

class MapRoadProps
{
	proto external int GetLOD();
	proto external void SetLOD(int lod);
	proto external int GetCount();
	proto external void SetCount(int count);
	proto external void SetVisibility(RoadType type, bool visibility);
	proto external void SetBorderVisibility(RoadType type, bool visibility);
	proto external void SetLineWidth(RoadType type, float width);
	proto external void SetBorderLineWidth(RoadType type, float width);
	proto external void SetLineType(RoadType type, EMapLineType lineType);
	proto external void SetDashedLineLength(RoadType type, float length);
	proto external void SetDashedLineGapLength(RoadType type, float length);
	proto external void SetColor(RoadType type, notnull Color color);
	proto external void SetSecondaryColor(RoadType type, notnull Color color);
	proto external void SetBorderColor(RoadType type, notnull Color color);
	proto external void SetVisible(bool visibility);
	proto external bool IsVisible();
}

/*!
\}
*/
