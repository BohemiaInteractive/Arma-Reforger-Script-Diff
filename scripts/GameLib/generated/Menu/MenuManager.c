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
	//! Put menu with given iPresetId into queue for closing (which is processed during next MenuManager update)
	proto external bool CloseMenuByPreset(ScriptMenuPresetEnum preset);
	//! Put menu with given iUserId into queue for closing (which is processed during next MenuManager update)
	proto external bool CloseMenuByUserData(int userId);
	//! Put menu into queue for closing (which is processed during next MenuManager update)
	proto external bool CloseMenu(MenuBase menu);
	//! Put all menus into queue for closing (which is processed during next MenuManager update), returns count of opened menus
	proto external int CloseAllMenus();
	/*!
	Opens a menu by given preset index. If unique is true and a menu with this
	preset already exists, the existing instance is returned. Otherwise a new
	menu is created or moved from cache.
	\param preset          Preset index identifying the menu to open.
	\param userId          User data stored on the menu instance.
	\param unique          When true, reuses an already opened menu with the same preset instead of creating a new one.
	\param hideParentMenu  When true, the parent menu receives OnMenuHide notification upon opening.
	*/
	proto external MenuBase OpenMenu(ScriptMenuPresetEnum preset, int userId = 0, bool unique = false, bool hideParentMenu = true);
	/*!
	Opens a dialog by given preset index. If unique is true and a dialog with
	this preset already exists, the existing instance is returned. Otherwise a
	new dialog is created or moved from cache. Dialogs are stacked by priority.
	\param preset          Preset index identifying the dialog to open.
	\param priority        Priority used for ordering in the dialog stack.
	\param iUserData       User data stored on the dialog instance.
	\param unique          When true, reuses an already opened dialog with the same preset instead of creating a new one.
	\param hideParentMenu  When true, the parent menu receives OnMenuHide notification upon opening.
	*/
	proto external MenuBase OpenDialog(ScriptMenuPresetEnum preset, int priority = DialogPriority.INFORMATIVE, int iUserData = 0, bool unique = false, bool hideParentMenu = true);
}

/*!
\}
*/
