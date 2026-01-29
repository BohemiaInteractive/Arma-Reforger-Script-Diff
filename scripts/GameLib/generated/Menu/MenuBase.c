/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Menu
\{
*/

/*!
Base class for menus in menu manager
order callbacks call:
opening:
1) OnMenuInit - called during load, when MenuManager loading menus config
2) OnMenuOpen - called when menu is beeing open
3) OnMenuShow
4) OnMenuFocusGained
5) OnMenuOpened - called after menu is opened and is ready

loop:
OnMenuUpdate
OnMenuItem

closing:
1) OnMenuFocusLost - called when menu is closed or other menu/dialog going to overlap the menu
2) OnMenuHide
3) OnMenuClose
*/
class MenuBase: ScriptedWidgetEventHandler
{
	//! Returns user id uint, set on menu opening
	proto external int GetUserData();
	proto external MenuManager GetManager();
	proto external Widget GetRootWidget();
	//! Returns pointer to widget associated with MenuItem (defined in menu config). If there is no MenuItem with given name, or there is no widget associated to it, returns nullptr
	proto external Widget GetItemWidget(string menuItemName);
	//! Sets text to widget associated with MenuItem (works only with text containing widgets like ButtonWidget, TextWidget, MultilineTextWidget, RichTextWidget, on other widgets do nothing)
	proto external MenuBase SetLabel(string menuItemName, string text);
	//! Put menu into queue for closing (which is processed during next MenuManeger update)
	proto external void Close();
	proto external bool IsFocused();
	proto external bool IsOpen();
	proto external bool AddMenuItem(string menuItemName, Widget w, string actionName);
	proto external bool RemoveMenuItem(string menuItemName);
	proto external void SetActionContext(string actionContextName);
	/*! Registers MenuItem callback. Callback is called when:
	-	Widget associated with MenuItem is clicked or changed (details are contained in MenuItemEvent argument of callback)
	- InputAction associated with MenuItem is triggered
	*/
	proto MenuBase BindItem(string menuItemName, MenuItemCallback callback);

	// callbacks

	event void OnMenuFocusGained();
	event void OnMenuFocusLost();
	event void OnMenuShow();
	event void OnMenuHide();
	event void OnMenuOpen();
	event void OnMenuOpened();
	event void OnMenuClose();
	event void OnMenuInit();
	event void OnMenuUpdate(float tDelta);
	event void OnMenuItem(string menuItemName, bool changed, bool finished);
}

/*!
\}
*/
