/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class RTTextureWidget: Widget
{
	//! Sets an FPS limit. When value is <=0, there is no limit
	proto external void SetMaxFPS(int fps);
	//! Gets the FPS limit. When value is <=0, there is no limit
	proto external int GetMaxFPS();
	/*!
	Sets resolution scale applied to the scene render target used by the recursive render pass driven by this widget.
	Values below 1.0 cause the scene to be rendered into a smaller intermediate target and upscaled into the widget's
	render target at the end of the render pass.
	*/
	proto external void SetResolutionScale(float scale);
	proto external float GetResolutionScale();
	/*!
	When true and the resolution scale is below 1.0, the upscale copy uses AMD FidelityFX Super Resolution 1.0.
	*/
	proto external void ToggleFSR(bool enabled);
	proto external bool IsFSREnabled();
	/*!
	Sets this widget instance (of type RTTextureWidgetTypeID) as render resource which can be referenced in material as $rendertarget.
	It is possible to use only after object selection. When selecting another object, it is necessary to set the render target again.
	When destroying the widget, use RemoveRenderTarget(IEntity ent) to remove the render target from the entity's mesh object.
	*/
	proto external void SetRenderTarget(IEntity ent);
	/*!
	Removes this widget instance (of type RTTextureWidgetTypeID) as render resource from the entity's mesh object. Must be called
	when destroying the widget.
	*/
	proto external void RemoveRenderTarget(IEntity ent);
	/*!
	Sets Widget of type RTTextureWidgetTypeID, to which it is possible to reference in shader as $renderview
	It is possible to use only after object selection. When selecting another object, it is necessary to set widget again.

	Camera index should be set to the camera that is used for rendering in this render target, set it to -1 if you are destroying
	the widget.
	*/
	proto external void SetRenderView(IEntity ent, int cameraIndex);
}

/*!
\}
*/
