/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup SaveGame
\{
*/

sealed class SaveGameManager: EventProvider
{
	//! Remember info for save-point creation and kick off transition to mission load
	proto external void StartPlaythrough(ResourceName missionResource, string optionalName = string.Empty, bool transition = true);
	//! True if saving is currently possible
	proto external bool IsSavingPossible();
	//! Configures which save types are currently available
	proto external void SetEnabledSaveTypes(ESaveGameType enabled);
	//! Get the currently enabled save types
	proto external ESaveGameType GetEnabledSaveTypes();
	/*!
	Allow or disallow saving. Can be used to avoid auto-save or manual save during cutscenes or other non-saveable events.
	If an auto-save was due it will execute as soon as it allowed again. Manual saves requested are ignored and will not be resumed later.
	*/
	proto external void SetSavingAllowed(bool allowed);
	//! True if saving is currently possible (incl. busy state) AND allowed
	proto external bool IsSavingAllowed();
	//! Get the number of the current playthrough
	proto external int GetCurrentPlaythroughNumber();
	/*!
	Request a new save point to be created. It will be created as soon as saving is possible.
	\param[in] type What save game type should be created. Used mainly for UI logic
	\param[in] displayName Name shown in UIs
	\param[in] flags Options for blocking (save all immediately) and shutdown after save complte (callback will invoke prior to it)
	\param[in] callback Async result handler that invokes after commit of save data completed
	\return False if a save was already requested or saving is currently not possible (e.g. scripted scene).
	*/
	proto external bool RequestSavePoint(ESaveGameType type, string displayName = string.Empty, ESaveGameRequestFlags flags = 0, SaveGameOperationCb callback = null);
	/*!
	Request to overridde a save with new data. It will keep i's old type, display name,playthrough and savepoint number.
	If the override fails the old save remains intact.
	\param[in] save Which save instance to overwrite
	\param[in] flags s. RequestSavePoint
	\param[in] callback s. RequestSavePoint
	\return False if the provided save can not be overwritten, another save was already requested or saving is currently not possible (e.g. scripted scene).
	*/
	proto external bool RequestSavePointOverwrite(notnull SaveGame save, ESaveGameRequestFlags flags = 0, SaveGameOperationCb callback = null);
	//! Busy state is entered on e.g. save creation, migration, deletion.
	proto external bool IsBusy();
	/*!
	Attempts to retrieve save game info from storage source.
	Filter can be filled with mission resource names. If no filter is provided it loads all.
	Returns false if an operation is already in progress
	*/
	proto external bool RetrieveSaveGameInfo(array<ResourceName> missionfilter = null, SaveGameOperationCb callback = null);
	/*!
	Access data for known save game info (RetrieveSaveGameInfo has to complete first to load existing saves).
	Returns count of save games matching missing filter.
	*/
	proto external int GetSaves(notnull out array<SaveGame> outSaveGames, ResourceName missionfilter = ResourceName.Empty);
	//! Get save game currently being played
	proto external SaveGame GetActiveSave();
	//! Resets the active save to none. So e.g. quick load no longer uses it.
	proto external void ResetActiveSave();
	//! Continue the game from this save point
	proto external void Load(notnull SaveGame saveGame, bool transition = true);
	//! Delete the save point
	proto external void Delete(notnull SaveGame saveGame, SaveGameOperationCb callback = null);
	//! Delete all save points of the given playthrough
	proto external void DeletePlaythrough(ResourceName mission, int playthrough, SaveGameOperationCb callback = null);
	//! Get the currently active mission name (Mission header or world file)
	static proto ResourceName GetCurrentMissionResource();

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
