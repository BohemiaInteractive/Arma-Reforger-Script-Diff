/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup SaveGame
\{
*/

sealed class SaveGame
{
	private void SaveGame();
	private void ~SaveGame();

	//! Mission header resource name
	proto external ResourceName GetMissionResource();
	//! Get unique id of the save point - primarily used for CLI loading by given id
	proto external UUID GetId();
	//! Type of save (flags)
	proto external ESaveGameType GetType();
	//! Save point per playthrough number 0-100
	proto external int GetSavePointNumber();
	//! Playthrough number 0-100
	proto external int GetPlaythroughNumber();
	//! Chosen name of save point or auto generated
	proto external string GetSavePointName();
	//! Chosen name of playthrough or auto generated
	proto external string GetPlaythroughName();
	//! When this save was created - UTC
	proto external int GetSavePointCreatedUnix();
	//! Unix time when the playthrough was began - UTC
	proto external int GetPlaythroughStartedUnix();
	//! When this save was created, local date time
	proto void GetSavePointCreatedLocalDateTime(out int year, out int month, out int day, out int hour, out int minute, out int second);
	//! When the playthrough was began, local date time
	proto void GetPlaythroughStartedDateTime(out int year, out int month, out int day, out int hour, out int minute, out int second);
	//! Total seconds elapsed in the playthrough
	proto external int GetPlaytime();
	//! Version of the game the save point was created on
	proto external string GetSavePointGameVersion();
	//! Is the save point's game version compatible with the current one
	proto external bool IsSavePointGameVersionCompatible();
	//! Addons used for the save point
	proto external int GetSavePointAddons(out notnull array<SaveGameAddonVersion> outAddons);
	//! Are the save point's addon versions compatible with the currently loaded ones
	proto external bool AreSavePointAddonsCompatible();
}

/*!
\}
*/
