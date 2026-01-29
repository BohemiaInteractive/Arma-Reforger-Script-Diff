/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Graph
\{
*/

class GraphNodeView: Managed
{
	void GraphNodeView(GraphNodeScene scene) {}

	proto external float GetZoom();
	//! Scale view matrix, returns new zoom
	proto external float SetZoom(float zoom);
	//! Check if rectangle is in view of the view matrix, position and size should be in scene space
	proto external bool IsRectangleInView(vector scenePos, vector sceneSize);
	//! Translate view matrix
	proto external void Translate(vector sceneTranslate);
	//! Scale view matrix, returns new zoom
	proto external float ZoomAt(vector zoomScenePosition, float zoom);
	//! Takes into account view, convert from scene space to pixel space
	proto external vector ScenePosToPixels(vector scenePos);
	//! Takes into account view, convert from scene space to pixel space
	proto external vector SceneSizeToPixels(vector sceneSize);
	//! Takes into account view, convert from pixel space to scene space
	proto external vector PosPixelsToScene(vector pixelPos);
	//! Takes into account view, convert from pixel space to scene space
	proto external vector SizePixelsToScene(vector pixelPos);
}

/*!
\}
*/
