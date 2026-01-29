/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Core
\{
*/

sealed class ChimeraWorld: ChimeraWorldBase
{
	//! Returns the time and weather manager entity
	proto external TimeAndWeatherManagerEntity GetTimeAndWeatherManager();
	// Returns garbage manager instance or null if none.
	proto external GarbageSystem GetGarbageSystem();
	proto external RadioManagerEntity GetRadioManager();
	proto external ItemPreviewManagerEntity GetItemPreviewManager();
	proto external MusicManager GetMusicManager();
	proto external SoundWorld GetSoundWorld();
	proto external DestructionManager GetDestructionManager();
	/*!
	Returns server-side timestamp of the world. On server, this is the same as
	local timestamp. On client, this is a prediction of what current timestamp
	on server might be, taking into account network latency. This timestamp
	may jump forward or experience time dilation (slow down or speed up), but
	it will never go backwards.
	*/
	proto external WorldTimestamp GetServerTimestamp();
	/*!
	Returns local timestamp of the world. \see BaseWorld.GetTimestamp().
	*/
	proto external WorldTimestamp GetLocalTimestamp();
	/*!
	Pause game time after the current frame. This will tick only specified entities in the world. This is working only in single-player.
	Only FRAME and POSTFRAME events will be fired for entities that have been registered to be updated while the game is paused.
	*/
	proto external void PauseGameTime(bool state);
	proto external bool IsGameTimePaused();
	//! Register entity to be updated while the game is paused. Deleting of entity is handled correctly on the C++ side.
	proto external void RegisterEntityToBeUpdatedWhileGameIsPaused(IEntity entity);
	//! Unregister the entity, it will not be updated anymore while the game is paused.
	proto external void UnregisterEntityToBeUpdatedWhileGameIsPaused(IEntity entity);
	//! Get 2D observers position
	proto external int GetObservers(notnull array<vector> observers);
	//! Get 2D multiplayer observers position
	proto external int GetMPObservers(notnull array<vector> observers);
	static proto ChimeraWorld CastFrom(BaseWorld world);
}

/*!
\}
*/
