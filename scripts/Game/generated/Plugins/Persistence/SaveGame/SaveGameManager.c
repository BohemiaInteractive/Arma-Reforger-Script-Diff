/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Persistence_SaveGame
\{
*/

sealed class SaveGameManager: EventProvider
{
	//! Get the save game manager instance
	static proto SaveGameManager Get();
	//! Busy state is entered on e.g. save creation, migration, deletion.
	proto external bool IsBusy();
	//! True if saving is generally enabled for the current scenario or not.
	proto external bool IsSavingEnabled();
	//! Configures which save types are currently available
	proto external void SetEnabledSaveTypes(ESaveGameType enabled);
	//! Get the currently enabled save types
	proto external ESaveGameType GetEnabledSaveTypes();
	/*!
	Allow or disallow saving. Can be used to avoid auto-save or manual save during cutscenes or other non-saveable events.
	Any requested save is queued until saving is allowed again.
	*/
	proto external void SetSavingAllowed(bool allowed);
	//! True if saving is enabled AND allowed
	proto external bool IsSavingAllowed();
	//! Remember info for save-point creation and kick off transition to mission load
	proto external void StartPlaythrough(string mission, string optionalName = string.Empty, bool transition = true);
	//! Get the number of the current playthrough
	proto external int GetCurrentPlaythroughNumber();
	/*!
	Request a new save point to be created. It will be created as soon as saving is possible.
	\param[in] type What save game type should be created. Used mainly for UI logic
	\param[in] displayName Name shown in UIs
	\param[in] flags Options for blocking (save all immediately) and shutdown after save complte (callback will invoke prior to it)
	\param[in] callback Async result handler that invokes after commit of save data completed
	*/
	proto external void RequestSavePoint(ESaveGameType type, string displayName = string.Empty, ESaveGameRequestFlags flags = 0, SaveGameOperationCallback callback = null);
	/*!
	Request to override a save with new data. It will keep the old type, display name, playthrough and savepoint number.
	If the override fails the old save remains intact.
	\param[in] save Which save instance to overwrite
	\param[in] flags s. RequestSavePoint
	\param[in] callback s. RequestSavePoint
	*/
	proto external void RequestSavePointOverwrite(notnull SaveGame save, ESaveGameRequestFlags flags = 0, SaveGameOperationCallback callback = null);
	/*!
	Attempts to retrieve save games from cache or else reads them from storage source.
	\param[in] missionfilter Load only for a specific mission or all known saves if passed Empty.
	\param[in] callback Asnc handler that gets invoked with saves found.
	*/
	proto external void GetSaves(string missionfilter, notnull SaveGameObtainCallback callback);
	//! Get save game currently being loaded/played
	proto external SaveGame GetActiveSave();
	//! Continue the game from this save point
	proto external void Load(notnull SaveGame saveGame, bool transition = true);
	//! Delete the save point
	proto external void Delete(notnull SaveGame saveGame, SaveGameOperationCallback callback = null);
	//! Delete all save points of the given mission (optionally: of a specific playthrough)
	proto external void Purge(string mission, int playthroughFilter = -1, SaveGameOperationCallback callback = null);
	//! Get the currently active mission name (Mission header or world file)
	static proto string GetCurrentMissionResource();

	// callbacks

	//! Called when a new save point was created
	[EventAttribute()]
	event void OnSaveCreated(SaveGame save);
	//! Called when a new save point was created
	[EventAttribute()]
	event void OnSaveDeleted(SaveGame save);
	//! Called when busy state changes
	[EventAttribute()]
	event void OnBusyStateChanged(bool busy);
}

/*!
\}
*/
