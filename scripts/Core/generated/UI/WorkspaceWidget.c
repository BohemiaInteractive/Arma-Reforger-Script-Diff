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
WorkspaceWidget class
*/
sealed class WorkspaceWidget: RTTextureWidget
{
	/*!
	Sets focus (necessary when using keyboard/joyped) to a particular widget. Widget must have some inputs like button, listbox, checkbox, combobox etc.
	\param newFocus A widget to focus. Passing null results in no widget being focused in this workspace.
	*/
	proto external void SetFocusedWidget(Widget newFocus, bool noCrossFade = false);
	//! Returns focused widget in this workspace
	proto external Widget GetFocusedWidget();
	//! Gets the only top Widget, which receives events
	proto external Widget GetModal();
	/*!
	Adds given Widget on top as a modal widget
	\param modalRoot Widget to add as a modal widget
	\param newFocus Widget to set as a new focus. If null, focus is set to first control element in modalRoot.
	*/
	proto external void AddModal(Widget modalRoot, Widget newFocus);
	/*!
	Removes given Widget from list of modal widgets
	\param modalRoot Widget to remove from the list
	*/
	proto external bool RemoveModal(Widget modalRoot);
	//!Returns Current render-target width
	proto external int GetWidth();
	//!Returns Current render-target height
	proto external int GetHeight();
	/*!
	Projects world position to screen position in reference resolution

	\param cam Which camera to use for projection. If '-1', current camera from current world is used.
	\return Screen coordinates in reference resolution(x, y axes) and depth in world units(z axis)
	*/
	proto external vector ProjWorldToScreen(vector pos, BaseWorld world, int cam = -1);
	/*!
	Projects screen position in reference resolution to world position and direction.

	\param x Coordinate on screen in reference resolution
	\param y Coordinate on screen in reference resolution
	\param outDir Returned direction vector
	\param cam Which camera to use for projection. If '-1', current camera from current world is used.
	\return World coordinates
	*/
	proto external vector ProjScreenToWorld(float x, float y, out vector outDir, BaseWorld world, int cam = -1);
	//! Works just like \see ProjWorldToScreen but returns x and y in native (current) not reference resolution
	proto external vector ProjWorldToScreenNative(vector pos, BaseWorld world, int cam = -1);
	//! Works just like \see ProjScreenToWorld but takes x and y in native (current) not reference resolution
	proto external vector ProjScreenToWorldNative(float x, float y, out vector outDir, BaseWorld world, int cam = -1);
	//! Return user DPI scale (1.0 by default)
	proto external float GetUserDPIScale();
	//! Set user DPI scale (1.0 by default), user DPI scale multiply window DPI scale.
	proto external void SetUserDPIScale(float fUserDPIScale);
	//!Scale value from reference to physical resolution
	proto external float DPIScale(float valueInReferenceResolution);
	//!Scale value from physical to reference resolution
	proto external float DPIUnscale(float valueInPhysicalResolution);
	//!Create widgets by \ref WidgetType, null Color means full alpha white
	proto external Widget CreateWidget(WidgetType type, WidgetFlags flags, Color color, int sort, Widget parentWidget = NULL);
	//!Create widget in this workspace, null Color means full alpha white
	proto external Widget CreateWidgetInWorkspace(WidgetType type, int left, int top, int width, int height, WidgetFlags flags, Color color, int sort);
	//!Create widgets from *.layout file
	proto Widget CreateWidgets(ResourceName layoutResourceName, Widget parentWidget = NULL);
}

/*!
\}
*/
