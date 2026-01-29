enum EEditorModeFlag
{
	DEFAULT				= 1 << 0, ///< Is added to all players by default
	ADMIN				= 1 << 1, ///< Is added to admins by default
	EDITABLE			= 1 << 2, ///< Can be configured by admin for others
	AUTO_SELECT			= 1 << 3, ///< Set the mode as current when it's added (only when the editor is closed)
	WORKBENCH			= 1 << 4, ///< Is added to all players in Workbench
	PERSISTENT			= 1 << 5, ///< Mode is saved when players disconnect and restored when they reconnect within time limit
};