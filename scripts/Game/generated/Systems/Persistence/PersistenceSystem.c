/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence
\{
*/

class PersistenceSystem: GameSystem
{
	static proto static PersistenceSystem GetInstance();
	/*!
	Triger a global save which will also flush changes to the database at the end.
	Returns false if the save process could not be started.
	*/
	proto external bool TriggerSave(ESaveGameType saveType);
	/*!
	Save the current state of the instance. Data is transient until the storage is flushed,
	but the entity can safely be deleted already after this (e.g. saving a disconnecting player).
	*/
	proto external bool Save(notnull Managed entityOrState, ESaveGameType saveType = ESaveGameType.MANUAL);
	//! Send of an async spawn request to fetch and instantiate the save-data. Any already available istances will instantly complete the callback with OK.
	proto external void RequestSpawn(notnull PersistenceSpawnRequest spawnRequest, PersistenceResultCallback callback = null);
	//! Send of an async spawn request to fetch and apply the save-data to existing instances.
	proto external void RequestLoad(notnull PersistenceLoadRequest loadRequest, PersistenceResultCallback callback = null);
	//! Returns wether the entity is part of a tracked parent or if it would be it's own root record
	proto external IEntity GetTrackedParent(IEntity entity);
	//! Get the current state of the system
	proto external EPersistenceSystemState GetState();
	//! Returns configuration choice to be used by serializers
	proto external bool IsPrefabDataForced();
	//! Is the instance configured to be persisted or will it be skipped in save-data.
	proto external bool IsTracked(notnull Managed entityOrState);
	/*!
	Make an instance known to the persistence system for further tracking and to apply save-data on load.
	Mostly for scripted states. IEntity should usually be registered by putting the PersistenceComponent on it!
	*/
	proto external bool StartTracking(notnull Managed entityOrState);
	//! Pause any save-data processing for this instance.
	proto external bool PauseTracking(notnull Managed entityOrState);
	//! Undo of PauseTracking.
	proto external bool ResumeTracking(notnull Managed entityOrState);
	//! Stop the tracking of the instance permanently. This will also release any persistent data next time a relevant save runs.
	proto external bool StopTracking(notnull Managed entityOrState);
	/*!
	Will release any tracker for this instance. None of the prior operations will be done. This is for memory optimization,
	so that if this instance will not be interacted with in any way in the near future it can be released.
	Useful for cleaning up trackers of players that disconnected and other situations where data should stay, but no more tracking is needed.
	*/
	proto external bool ReleaseTracking(notnull Managed entityOrState);
	//! Get the persistence id of the instance. Null if not persistent.
	proto external UUID GetId(Managed entityOrState);
	/*!
	Set the persistence id of the instance. False if not persistent, or the id is already in use.
	\param entityOrState	Instance to set the id for
	\param id				Id to assign onto the instance
	\param makeAvailable	Resolve pending WhenAvailable calls if all relevant data is applied already.
	*/
	proto external bool SetId(notnull Managed entityOrState, UUID id, bool makeAvailable = false);
	//! Find a tracked instance by id. Returns null if nothing under that id is found.
	proto external Managed FindById(UUID id);
	//! Find a collection by its ResourceName from the collections configuration entry or the chosen displayName.
	proto external PersistenceCollection FindCollection(string resourceOrDisplayName);
	/*!
	Obtain a configuration instance that can be adjusted dynamically through script.
	By default it is populated with the values from the system configuration.
	You can not get/create an instance for something that has no configuration at all.
	*/
	proto external PersistenceConfig GetConfig(notnull Managed entityOrState);
	//! Apply a configuration obtained by GetConfig and possibly changed via script.
	proto external void SetConfig(notnull Managed entityOrState, notnull PersistenceConfig config);
	/*!
	If the circumstances change this allows to re-evaluate the rules and select the currently appropriate config.
	Note: This will reset any customization done via script on SetConfig.
	*/
	proto external bool ReloadConfig(notnull Managed entityOrState);
	//! Serialize an entity instance into the provided save context.
	proto external ESerializeResult SerializeEntity(notnull IEntity entity, notnull BaseSerializationSaveContext context, SerializerDefaultSpawnData defaultData = null);
	/*!
	Apply serialized data to an existing entity instance (only FINALIZE handlers are allowed for loaded entities)
	Returns loaded entity, or recreated one, or null in case of failure
	*/
	proto external IEntity DeserializeLoadEntity(notnull IEntity entity, notnull BaseSerializationLoadContext context, bool allowRecreate = true);
	//! Spawn a new entity from serialization data.
	proto external IEntity DeserializeSpawnEntity(notnull EntitySpawnParams params, notnull BaseSerializationLoadContext context);
	/*!
	The deserialization completed callback is invoked after all tasks are completed.
	Adding a task gives the ability to wait for a call queue, frame or other delayed event until the instance is considered done and can be used by the callback handler.
	If the original request does not provide any callback to invoke these tasks will be ignored.
	The tasks is automatically added for the current load instance. Calling the method outside of a current load phase will have no effect.
	*/
	proto external ref PersistenceDeferredDeserializeTask AddDeferredDeserializeTask();
	//! Give ownership of a deferred task back to the system which will complete it after the given time in seconds has passed.
	proto external void ResolveDeferredDeserializeTaskAfterTime(notnull PersistenceDeferredDeserializeTask deferredDeserializeTask, float timeSeconds);
	/*!
	Await the presence of an instance tracked by the system under the given UUID.
	\param uuid						The id to wait for. Could become known through registration of an existing id or spawn/load process of a record from the databse.
									NOTE: It is not guarantueed that the instance has completed all its deferred deserialization tasks already.
	\param task						The task you want to execute on availability. Will be instantly executed on call if the UUID is already available.
	\param maxWaitSeconds			Time after which availability check expires and the callback wil invoke with expired=true and instance=null. Wait time during async inital setup does not count.
	\param deferredDeserializeTask	If a deserialization completion should await this additional task, then it can be passed here.
									If the UUID is already available it will prematurely complete the deferred task AFTER the work was done.
	*/
	proto external void WhenAvailable(UUID uuid, notnull PersistenceWhenAvailableTask task, float maxWaitSeconds = 0.0, PersistenceDeferredDeserializeTask deferredDeserializeTask = null);
	//! Info if the system inited with loading data or not on new playthrough
	proto external bool WasDataLoaded();
	proto external Managed GetPersistentState(typename stateType);
	/*!
	Deletes all data for session storage if any is currently used. To be invoked on e.g. GamemodeEnd or debug purposes.
	*/
	proto external void ClearSessionData(PersistenceStatusCallback callback = null);

	// callbacks

	event protected void OnStateChanged(EPersistenceSystemState oldState, EPersistenceSystemState newState);
	//! Called before save data is being read (and comitted).
	event protected void OnBeforeSave(ESaveGameType saveType);
	/*!
	After the save was created and comitted to database.
	\param saveType Type of save that was done
	\param success True or false if save data was successfully comitted to database
	*/
	event protected void OnAfterSave(ESaveGameType saveType, bool success);
	//! Called after initial world load completed. Success = false indicates fatal deserialization which is likely to negatively impact gameplay.
	event protected void OnAfterLoad(bool success);
	/*!
	Handle deletion of an entity to do some pre or post processing
	Reasons for deletion include:
		- Removed in previous session and the change is being applied on load
		- Deserialize failure that was configured not to be tolerated
	*/
	event protected void HandleDelete(IEntity entity);
}

/*!
\}
*/
