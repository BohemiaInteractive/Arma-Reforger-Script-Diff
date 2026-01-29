/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

/*!
All methods working with vector work only with its x and y components, the z is always ignored
*/
sealed class CanvasWidgetBase: Widget
{
	//! How many pixels are currently drawn per one virtual unit
	proto external float PixelPerUnit();
	//! Scales given scalar by current zoom value
	proto external float ScaleByZoom(float originalSize);
	//! Gets the current zoom level
	proto external float GetZoom();
	//! Sets the current zoom level
	proto external void SetZoom(float zoomLevel);
	//! Transforms given position to pixel position which can used for drawing
	proto external vector PosToPixels(vector posUnits);
	//! Transforms given size to pixel size which can be used for drawing
	proto external vector SizeToPixels(vector sizeUnits);
	/*!
	Performs zooming to `zoomLevel` and keeps the `posUnits` constant
	E.g. if pos is half of the whole range, the zoom will be performed such as that the center
	of the widget will remain in the center.
	*/
	proto external void ZoomAt(vector posUnits, float zoomLevel);
	//! Gets the current offset in pixels
	proto external vector GetOffsetPx();
	//! Sets the current offset in pixels
	proto external void SetOffsetPx(vector offsetPx);
	proto external vector GetSizeInUnits();
	proto external void SetSizeInUnits(vector newSize);
}

/*!
\}
*/
