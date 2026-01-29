/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Menu
\{
*/

sealed class MenuManager
{
	protected void MenuManager();
	protected void ~MenuManager();

	//! Finds first menu/dialog with given preset index, or nullptr when there is no such menu opened.
	proto external MenuBase FindMenuByPreset(ScriptMenuPresetEnum preset);
	//! Finds first menu/dialog with given user id, or nullptr when there is no such menu opened.
	proto external MenuBase FindMenuByUserData(int userId);
	//! Returns most top opened menu or nullptr when no menu is opened.
	proto external MenuBase GetTopMenu();
	//! Return owner menu of Widget
	proto external MenuBase GetOwnerMenu(Widget w);
	proto external bool IsAnyMenuOpen();
	proto external bool IsAnyDialogOpen();
	//! Put menu with given iPresetId into queue for closing (which is processed during next MenuManeger update)
	proto external bool CloseMenuByPreset(ScriptMenuPresetEnum preset);
	//! Put menu with given iUserId into queue for closing (which is processed during next MenuManeger update)
	proto external bool CloseMenuByUserData(int userId);
	//! Put menu into queue for closing (which is processed during next MenuManeger update)
	proto external bool CloseMenu(MenuBase menu);
	//! Put all menus into queue for closing (which is processed during next MenuManeger update), returns count of opened menus
	proto external int CloseAllMenus();
	proto external MenuBase OpenMenu(ScriptMenuPresetEnum preset, int userId = 0, bool unique = false, bool hideParentMenu = true);
	proto external MenuBase OpenDialog(ScriptMenuPresetEnum preset, int priority = DialogPriority.INFORMATIVE, int iUserData = 0, bool unique = false);
}

/*!
\}
*/
