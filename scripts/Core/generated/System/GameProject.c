/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

sealed class GameProject
{
	private void GameProject();
	private void ~GameProject();

	//! Return array of GUIDs of loaded addons
	static proto void GetAvailableAddons(notnull out array<string> addonGUIDs);
	//! Return array of GUIDs of loaded addons
	static proto void GetLoadedAddons(notnull out array<string> addonGUIDs);
	//! Return ID of addon with given GUID (only for available addons)
	static proto string GetAddonID(string GUID);
	//! Return Title of addon with given GUID (only for available addons)
	static proto string GetAddonTitle(string GUID);
	//! Return true if addon with given GUID is part of vanilla addons
	static proto bool IsVanillaAddon(string GUID);
	//! Return game project module config by class name
	static proto BaseContainer GetModuleConfig(string moduleClassName);
	//! Return game project for current configuration
	static proto BaseContainer GetGameProjectSettings();
}

/*!
\}
*/
