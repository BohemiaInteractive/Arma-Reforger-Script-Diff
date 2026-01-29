/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Map
\{
*/

class MapLegendProps
{
	proto external int GetLineWidth();
	proto external void SetLineWidth(int width);
	proto external int GetSegmentNumber();
	proto external void SetSegmentNumber(int num);
	proto external int GetTotalSegmentLength();
	proto external void SetTotalSegmentLength(int length);
	proto external int GetDistanceTextSize();
	proto external void SetDistanceTextSize(int size);
	proto external int GetUnitTextSize();
	proto external void SetUnitTextSize(int size);
	proto external float GetOffsetX();
	proto external void SetOffsetX(float offset);
	proto external float GetOffsetY();
	proto external void SetOffsetY(float offset);
	//! Gets the current alignment flag for the side of the screen (Left or Right)
	proto external bool GetAlignRight();
	//! Sets the screen side alignment flag (true = Right)
	proto external void SetAlignRight(bool align);
	//! Gets the current alignment flag for the side of the screen (Bottom or Top)
	proto external bool GetAlignTop();
	//! Sets the screen side alignment flag (true = Top)
	proto external void SetAlignTop(bool align);
	proto ref Color GetSegment1Color();
	proto external void SetSegment1Color(notnull Color color);
	proto ref Color GetSegment2Color();
	proto external void SetSegment2Color(notnull Color color);
	proto ref Color GetDistanceTextColor();
	proto external void SetDistanceTextColor(notnull Color color);
	proto ref Color GetUnitTextColor();
	proto external void SetUnitTextColor(notnull Color color);
	proto external void SetTextBold();
	proto external void SetTextItalic();
	proto void SetFont(ResourceName fontPath);
}

/*!
\}
*/
