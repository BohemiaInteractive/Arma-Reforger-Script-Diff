/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class CanvasWidget: CanvasWidgetBase
{
	/*!
	Commands in the array will be used to draw from now on
	The caller needs to keep the array alive - the callee takes just a pointer to it.
	Commands in the array are batched by the widget if possible. However, to allow batching
	some rules need to be followed.
	Consecutive commands of different type always start a new batch.
	For consecutive commands of the same type, using a different texture starts a new batch.
	ImageDrawCommand is not batched.
	TextDrawCommands with different rotations start a new batch.
	*/
	proto external void SetDrawCommands(array<ref CanvasWidgetCommand> drawCommands);
	//! Loads a texture which can be then used in draw commands
	static proto ref SharedItemRef LoadTexture(ResourceName resource);
	//! Loads imageset and setup ImageDrawCommand texture and UV
	proto ref ImageDrawCommand CreateCommandFromImageSet(ResourceName resource, string imageName, vector size);
	//! Tessellate an ellipse to prepare it for rendering. center and radius uses only x and y values
	proto void TessellateEllipse(vector center, vector radius, int segmentCount, out notnull array<float> vertices);
	//! Tessellate a circle to prepare it for rendering. center uses only x and y values
	proto void TessellateCircle(vector center, float radius, int segmentCount, out notnull array<float> vertices);
	//! Tessellate a rounded rectangle to prepare it for rendering
	proto void TessellateRoundedRectangle(vector mins, vector maxs, float cornerRadius, int cornerSegments, RectangleCorner roundedCorners, out notnull array<float> vertices);
}

/*!
\}
*/
