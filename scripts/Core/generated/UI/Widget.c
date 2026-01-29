/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class Widget: Managed
{
	private void Widget();

	proto external void Update();
	//!Returns parent in hierarchy, or nullptr
	proto external Widget GetParent();
	//!Returns children in hierarchy, or nullptr
	proto external Widget GetChildren();
	//!Returns next sibling in hierarchy, or nullptr
	proto external Widget GetSibling();
	proto external void RemoveChild(notnull Widget child);
	proto external void SetName(string name);
	proto external string GetName();
	/*!
	Sets user ID
	\param id User ID
	*/
	proto external void SetUserID(int id);
	/*!
	Gets user ID
	\return User ID
	*/
	proto external int GetUserID();
	proto external bool IsFocusable();
	//! Adds given widget as a child. Possible only if this widget accepts more children.
	proto external void AddChild(notnull Widget child);
	//! Add (hook) handler to widget. Adds reference to the handler.
	proto external void AddHandler(ScriptedWidgetEventHandler eventHandler);
	//! Unhook handler from widget. Release reference to the handler.
	proto external void RemoveHandler(ScriptedWidgetEventHandler eventHandler);
	//! Return number of all handlers attached to widget (C++ & scripted)
	proto external int GetNumHandlers();
	//! return widget event handler on given index, when the event handler is not inherited from ScriptedWidgetEventHandler, null is returned
	proto external ScriptedWidgetEventHandler GetHandler(int index);
	//! return first widget event handler of given type, when none of event handlers has the type null is returned
	proto external ScriptedWidgetEventHandler FindHandler(typename type);
	//! Sets visibility of this widget (VISIBLE flag)
	proto external void SetVisible(bool show);
	//! Sets enabled state of this widget (DISABLED flag)
	proto external void SetEnabled(bool enable);
	//! Returns whether is this widget visible (VISIBLE flag is set)
	proto external bool IsVisible();
	//! Returns whether are this widget and all its ancesstors visible (VISIBLE flag is set)
	proto external bool IsVisibleInHierarchy();
	//! Returns whether is this widget enabled (DISABLED flag is not set)
	proto external bool IsEnabled();
	//! Returns whether are this widget and all its ancesstors enabled (DISABLED flag is not set)
	proto external bool IsEnabledInHierarchy();
	//! Returns wheter this widget clips its children
	proto external bool IsClippingChildren();
	/*!
	\return Whether this widget inherits color from its parent (INHERIT_COLOR flag)
	*/
	proto external bool GetIsColorInherited();
	//! Sets whether this widget inherits color from its parent
	proto external void SetIsColorInherited(bool isColorInherited);
	//! Sets color of this widget
	proto external void SetColor(notnull Color color);
	/*!
	Set ARGB color
	*/
	proto external void SetColorInt(int color);
	/*!
	\return ARGB color
	*/
	proto external int GetColorInt();
	//! Sets opacity which applies to this widget and all its descendants
	proto external void SetOpacity(float alpha);
	/*!
	\return Opacity which applies to this widget and all its descendants
	*/
	proto external float GetOpacity();
	/*!
	Changes z-order value. Widget will be moved into appropriate order. Higher values put the widget more to the in front.
	\param zOrder ZOrder value
	*/
	proto external void SetZOrder(int zOrder);
	/*!
	Higher values mean more in front.
	\return Current ZOrder value.
	*/
	proto external int GetZOrder();
	/*!
	Removes this widget from its parent and from the WidgetManager.
	When there are no more strong references to this widget, it will be destroyed.
	*/
	proto external void RemoveFromHierarchy();
	//! Find Widget by path. e.g FindWidget("widget1.widget2.widget3.mywidget")
	proto external Widget FindWidget(string pathname);
	//! Looks for a widget with given `name` in `this` widget's children
	proto external Widget FindAnyWidget(string pathname);
	//! Find Widget by userID
	proto external Widget FindAnyWidgetById(int user_id);
	proto external string GetStyleName();
	/*!
	\return Workspace which this widget belongs to. nullptr if there is no workspace in ancestors
	*/
	proto external WorkspaceWidget GetWorkspace();
	//! Returns color of this widget
	proto ref Color GetColor();
	/*!
	Gets navigation rule for given direction
	\param rule Out value of rule in given direction
	\return Explicit target if rule is EXPLICIT. Otherwise undefined.
	*/
	proto string GetNavigation(WidgetNavigationDirection direction, out WidgetNavigationRuleType rule);
	/*!
	Sets navigation rule for given direction
	\param explicitTarget Used only when rule is set to EXPLICIT
	 */
	proto external void SetNavigation(WidgetNavigationDirection direction, WidgetNavigationRuleType rule, string explicitTarget = string.Empty);
	proto external string GetTypeName();
	/*!
	\return WidgetFlags which are set for this widget
	*/
	proto external int GetFlags();
	/*!
	Sets given WidgetFlags for this widget
	\return Previous state of the flags
	*/
	proto external int SetFlags(int flags);
	/*!
	Removes all flags
	\return Previous state of the flags
	*/
	proto external int ClearFlags(int flags);
	proto external WidgetType GetTypeID();
	//! Returns position of this widget's top left corner in physical resolution
	proto void GetScreenPos(out float x, out float y);
	//! Returns size of this widget in physical resolution
	proto void GetScreenSize(out float width, out float height);
	//! Remove callback with given callbackId (callback ID is returned from AddCallback method)
	proto external bool RemoveCallback(int eventId, int callbackId);
	//! Remove callback with given caller object (in case the callback method is on object instance)
	proto external bool RemoveCallbackByOwner(int eventId, Class owner);
	//! Trigger callbacks attached to eventID on the widget (not called over hierarchy)
	proto external bool EmitCallback(int eventId);
	//! Add callback for given event id and returns callback id
	proto int AddCallback(int eventId, WidgetEventCallback fn);
	/*!
	Emits custom widget event (called over hierarchy like other widget events).
	\see ScriptedWidgetEventHandler

	\param iData User parameter passed to event listener.
	\param pData User parameter passed to event listener.
	\return Returns true when event is processed by any listener.
	*/
	proto external bool EmitCustomEvent(int iData, Managed pData);
}

/*!
\}
*/
