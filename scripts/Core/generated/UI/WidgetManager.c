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
Class containing global and util functions regarding widgets
*/
sealed class WidgetManager
{
	private void WidgetManager();
	private void ~WidgetManager();

	//! UTF8 encoded nbsp (Non Breaking Space) character usable in UI
	static const string NonBreakingSpace;

	/*!
	Finds all widgets on specified position and returns them sorted by proximity.
	Traverses the widget hierarchy starting from the given root widget and collects all widgets that
	intersect with the specified position. Widgets with the flags `DISABLED`, or
	those that are not `VISIBLE` are skipped. The resulting list is sorted from the closest to the farthest
	widget relative to the viewer (e.g., front-to-back rendering order).

	\param x X coordinate in screen space, using NATIVE resolution units.
	\param y Y coordinate in screen space, using NATIVE resolution units.
	\param rootWidget The root widget at which to begin the trace. This widget and its entire hierarchy will be considered.
	\param outWidgets Output array to be filled with intersecting widgets, sorted from closest to farthest.
	*/
	static proto void TraceWidgets(int x, int y, notnull Widget rootWidget, notnull array<Widget> outWidgets);
	/*!
	Finds all widgets intersecting a specified rectangle and returns them sorted by proximity.
	Traverses the widget hierarchy starting from the given root widget and collects all widgets that
	intersect with the specified rectangle. Widgets with the flags `DISABLED`, or
	those that are not `VISIBLE` are skipped. The resulting list is sorted from the closest to the farthest
	widget relative to the viewer (e.g., front-to-back rendering order).

	\param x X coordinate (top-left corner) of the rectangle in screen space, using NATIVE resolution units.
	\param y Y coordinate (top-left corner) of the rectangle in screen space, using NATIVE resolution units.
	\param width Width of the rectangle in NATIVE resolution units.
	\param height Height of the rectangle in NATIVE resolution units.
	\param rootWidget The root widget at which to begin the trace. This widget and its entire hierarchy will be considered.
	\param outWidgets Output array to be filled with intersecting widgets, sorted from closest to farthest.
	*/
	static proto void TraceWidgetsRect(int x, int y, int width, int height, notnull Widget rootWidget, notnull array<Widget> outWidgets);
	/*!
	Finds all widgets intersecting a specified circle and returns them sorted by proximity.
	Traverses the widget hierarchy starting from the given root widget and collects all widgets that
	intersect with the specified circle. Widgets with the flags `DISABLED`, or
	those that are not `VISIBLE` are skipped. The resulting list is sorted from the closest to the farthest
	widget relative to the viewer (e.g., front-to-back rendering order).

	\param x X coordinate of the circle center in screen space, using NATIVE resolution units.
	\param y Y coordinate of the circle center in screen space, using NATIVE resolution units.
	\param radius Radius of the circle in NATIVE resolution units.
	\param rootWidget The root widget at which to begin the trace. This widget and its entire hierarchy will be considered.
	\param outWidgets Output array to be filled with intersecting widgets, sorted from closest to farthest.
	*/
	static proto void TraceWidgetsCircle(int x, int y, float radius, notnull Widget rootWidget, notnull array<Widget> outWidgets);
	static proto void SetLanguage(string languageCode);
	static proto void GetLanguage(out string languageCode);
	static proto void SetCursor(int cursorIndex);
	static proto void ReportMouse(int mousex, int mousey, notnull Widget rootWidget);
	/*!
	Gets reference resolution used for DPI scaling
	To work with DPI scaling use WorkspaceWidget.DPIScale and WorkspaceWidget.DPIUnscale
	*/
	static proto void GetReferenceScreenSize(out int width, out int height);
	//! Gets current mouse position in native (current) resolution (not reference)
	static proto void GetMousePos(out int x, out int y);
	static proto Widget GetWidgetUnderCursor();
	//! Returns how long is TextCursor shown/hidden for. Time in milliseconds
	static proto int GetTextCursorBlinkTime();
	//! \see GetTextCursorBlinkTime
	static proto void SetTextCursorBlinkTime(int milliseconds);
	//! Convert given text to localized version (multibyte UTF-8) using text ID's and localization table
	static proto string Translate(string text, void param1 = NULL, void param2 = NULL, void param3 = NULL, void param4 = NULL, void param5 = NULL, void param6 = NULL, void param7 = NULL, void param8 = NULL, void param9 = NULL);
	//! Return indices of IDs which contains searched text. Search is case insensitive.
	static proto int SearchLocalized(string text, notnull array<string> IDs, notnull out array<int> outIndices);
}

/*!
\}
*/
