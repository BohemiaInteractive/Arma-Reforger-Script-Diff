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
An event handler for UI widgets can be attached to any widget using Widget.AddHandler. It can also be attached to multiple widgets.
After attaching, the handler will start to receive events from the widget as well as unprocessed events from all its children.
An event is considered processed when the event method returns TRUE. Events are first called on all event handlers of the widget,
then on the parent widget in the hierarchy until the event is processed or the hierarchy root widget is reached.
*/
class ScriptedWidgetEventHandler: Managed
{
	/*!
	Called when a button is clicked (including buttons on widgets like checkboxes, scrollbars, combo-boxes).
	\param w The widget on which the event was triggered.
	\param x Mouse position (if triggered by the mouse).
	\param y Mouse position (if triggered by the mouse).
	\param button Index of the mouse button (if triggered by the mouse).
	\return When returning true, the event is considered processed.
	*/
	event bool OnClick(Widget w, int x, int y, int button);
	/*!
	Called when a widget's visibility flag has changed and the widget is now flagged as visible. This information does not take into account whether the whole hierarchy is visible or not, only that the widget itself is flagged as visible.
	\param w The widget on which the event was triggered.
	\return When returning true, the event is considered processed.
	*/
	event bool OnShow(Widget w);
	/*!
	Called when a widget's visibility flag has changed and the widget is now flagged as hidden. This information does not take into account whether the whole hierarchy is visible or not, only that the widget itself is flagged as hidden.
	\param w The widget on which the event was triggered.
	\return When returning true, the event is considered processed.
	*/
	event bool OnHide(Widget w);
	/*!
	Called when a widget's enabled/disabled state has changed and the widget is now enabled. This information does not take into account whether the whole hierarchy is enabled or not, only that the widget itself is flagged as enabled.
	\param w The widget on which the event was triggered.
	\return When returning true, the event is considered processed.
	*/
	event bool OnEnable(Widget w);
	/*!
	Called when a widget's enabled/disabled state has changed and the widget is now disabled. This information does not take into account whether the whole hierarchy is enabled or not, only that the widget itself is flagged as disabled.
	\param w The widget on which the event was triggered.
	\return When returning true, the event is considered processed.
	*/
	event bool OnDisable(Widget w);
	/*!
	Called when a modal widget is closed.
	\param modalRoot The widget on which the event was triggered.
	\return When returning true, the event is considered processed.
	*/
	event bool OnModalClosed(Widget modalRoot);
	/*!
	Called when the user clicks outside a modal widget. If the event is not processed, the click procedure is repeated to enable clicking on the widgets underneath once the modal is closed.
	\param modalRoot The widget on which the event was triggered.
	\param x Mouse position (if triggered by the mouse).
	\param y Mouse position (if triggered by the mouse).
	\param button Index of the mouse button (if triggered by the mouse).
	\return When returning true, the event is considered processed.
	*/
	event bool OnModalClickOut(Widget modalRoot, int x, int y, int button) { return true; };
	/*!
	Called when the user double-clicks on a widget.
	\param w The widget on which the event was triggered.
	\param x Mouse position.
	\param y Mouse position.
	\param button Index of the mouse button.
	\return When returning true, the event is considered processed.
	*/
	event bool OnDoubleClick(Widget w, int x, int y, int button);
	/*!
	Called when the user selects an item in a listbox.
	\param w The widget on which the event was triggered.
	\return When returning true, the event is considered processed.
	*/
	event bool OnItemSelected(Widget w, int row, int column, int oldRow, int oldColumn);
	/*!
	Called when a widget is focused.
	\param w The widget on which the event was triggered.
	\param x Mouse position (if triggered by the mouse).
	\param y Mouse position (if triggered by the mouse).
	\return When returning true, the event is considered processed.
	*/
	event bool OnFocus(Widget w, int x, int y);
	/*!
	Called when a widget loses focus.
	\param w The widget on which the event was triggered.
	\param x Mouse position (if triggered by the mouse).
	\param y Mouse position (if triggered by the mouse).
	\return When returning true, the event is considered processed.
	*/
	event bool OnFocusLost(Widget w, int x, int y);
	/*!
	Called when a widget gets under the mouse cursor.
	\param w The widget on which the event was triggered.
	\param x Mouse position.
	\param y Mouse position.
	\return When returning true, the event is considered processed.
	*/
	event bool OnMouseEnter(Widget w, int x, int y);
	/*!
	Called when a widget is no longer under the mouse cursor.
	\param w The widget on which the event was triggered.
	\param x Mouse position.
	\param y Mouse position.
	\return When returning true, the event is considered processed.
	*/
	event bool OnMouseLeave(Widget w, Widget enterW, int x, int y);
	/*!
	Called on a widget under the cursor when the user uses the mouse wheel.
	\param w The widget on which the event was triggered.
	\param x Mouse position.
	\param y Mouse position.
	\param wheel Value of the mouse wheel.
	\return When returning true, the event is considered processed.
	*/
	event bool OnMouseWheel(Widget w, int x, int y, int wheel);
	/*!
	Called on a widget under the cursor when the user presses a mouse button.
	\param w The widget on which the event was triggered.
	\param x Mouse position.
	\param y Mouse position.
	\param button Index of the mouse button.
	\return When returning true, the event is considered processed.
	*/
	event bool OnMouseButtonDown(Widget w, int x, int y, int button);
	/*!
	Called when the user releases a mouse button on the widget under the cursor and also on the widget where the mouse down event was triggered.
	\param w The widget on which the event was triggered.
	\param x Mouse position.
	\param y Mouse position.
	\param button Index of the mouse button.
	\return When returning true, the event is considered processed.
	*/
	event bool OnMouseButtonUp(Widget w, int x, int y, int button);
	/*!
	Called when a hyper link gets under the mouse cursor.
	\param w The widget on which the event was triggered.
	\param x Mouse position.
	\param y Mouse position.
	\param link Link value.
	\return When returning true, the event is considered processed.
	*/
	event bool OnLinkEnter(Widget w, int x, int y, string link);
	/*!
	Called when a hyper link is no longer under the mouse cursor.
	\param w The widget on which the event was triggered.
	\param x Mouse position.
	\param y Mouse position.
	\param link Link value.
	\return When returning true, the event is considered processed.
	*/
	event bool OnLinkLeave(Widget w, int x, int y, string link);
	/*!
	Called when a hyper link is clicked.
	\param w The widget on which the event was triggered.
	\param x Mouse position (if triggered by the mouse).
	\param y Mouse position (if triggered by the mouse).
	\param button Index of the mouse button (if triggered by the mouse).
	\param link Link value.
	\return When returning true, the event is considered processed.
	*/
	event bool OnLinkClick(Widget w, int x, int y, int button, string link);
	/*!
	Called on a focused widget when the user triggers a controller action via keyboard or game device.
	\param w The widget on which the event was triggered.
	\param control Type of the control action.
	\param value Value of the control action.
	\return When returning true, the event is considered processed.
	*/
	event bool OnController(Widget w, ControlID control, int value);
	/*!
	Called when the user types on a focused widget that accepts text input (EditBoxWidget, MultilineEditBox, Listbox widgets).
	\param w The widget on which the event was triggered.
	\param charCode Typed character.
	\return When returning true, the event is considered processed.
	*/
	event bool OnChar(Widget w, int charCode);
	/*!
	Called when the internal state of a widget is changed in widgets with internal states (CheckBoxWidget, EditBoxWidget, MultilineEditBox, SliderWidget, ComboBoxWidget).
	\param w The widget on which the event was triggered.
	\param finished Indicates if the change of state is finished or in progress.
	\return When returning true, the event is considered processed.
	*/
	event bool OnChange(Widget w, bool finished);
	/*!
	Called when the user enter write mode on a edit box (EditBoxWidget, MultilineEditBox).
	\param w The widget on which the event was triggered.
	\return When returning true, the event is considered processed.
	*/
	event bool OnWriteModeEnter(Widget w);
	/*!
	Called when the user leave write mode on a edit box (EditBoxWidget, MultilineEditBox).
	\param w The widget on which the event was triggered.
	\return When returning true, the event is considered processed.
	*/
	event bool OnWriteModeLeave(Widget w);
	/*!
	Called when a child widget is added to a widget.
	\param w The widget on which the event was triggered.
	\param child The child widget.
	\return When returning true, the event is considered processed.
	*/
	event bool OnChildAdd(Widget w, Widget child);
	/*!
	Called when a child widget is removed from a widget.
	\param w The widget on which the event was triggered.
	\param child The child widget.
	\return When returning true, the event is considered processed.
	*/
	event bool OnChildRemove(Widget w, Widget child);
	/*!
	Called when event is emitted by call of Widget.EmitCustomEvent
	\param w The widget on which the event was triggered.
	\param iUserData User parameter passed to Widget.EmitCustomEvent
	\param pUserData User parameter passed to Widget.EmitCustomEvent
	\return When returning true, the event is considered processed.
	*/
	event bool OnCustomEvent(Widget w, int iUserData, Managed pUserData);
	/*!
	Called when a widget is updated due to a layout change.
	\param w The widget on which the event was triggered.
	\return When returning true, the event is considered processed.
	*/
	event bool OnUpdate(Widget w);
	/*!
	Called when a new handler (widget component) is attached to a widget.
	\param w The widget on which the event was triggered.
	*/
	event void HandlerAttached(Widget w);
	/*!
	Called when a handler (widget component) is detached from a widget.
	\param w The widget on which the event was triggered.
	*/
	event void HandlerDeattached(Widget w);
}

/*!
\}
*/
