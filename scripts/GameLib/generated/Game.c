/*
===========================================
Do not modify, this script is generated
===========================================
*/

class Game
{
	protected World m_World;
	protected InputManager m_InputManager;
	protected MenuManager m_MenuManager;
	protected WorkspaceWidget m_WorkspaceWidget;
	sealed World GetWorld() { return m_World; }
	sealed InputManager GetInputManager() { return m_InputManager; }
	sealed MenuManager GetMenuManager() { return m_MenuManager; }
	sealed WorkspaceWidget GetWorkspace() { return m_WorkspaceWidget; }

	/*!
	\brief Returns path of world file loaded
	*/
	proto external string GetWorldFile();
	/*!
	Returns the world editor.
	*IMPORTANT* Only works when InPlayMode is false
	*/
	proto external GameWorldEditor GetWorldEditor();
	// --- Cinematics
	proto external void StartCinematic(string name);
	proto external void StopCinematic();
	proto external bool IsPlayingCinematic();
	/*!
	\brief Returns UserSettings container for Engine User Settings
	*/
	proto external UserSettings GetEngineUserSettings();
	/*!
	\brief Returns UserSettings container for Game User Settings
	*/
	proto external UserSettings GetGameUserSettings();
	/*!
	\brief Apply setting preset according to DisplayUserSettings.OverallQuality
	*/
	proto external void ApplySettingsPreset();
	/*!
	\brief Notify engine about changes in Engine User Settings
	*/
	proto external void UserSettingsChanged();
	/*!
	\brief Save Engine User Settings to permanent storage
	*/
	proto external void SaveUserSettings();
	/*!
	\brief Determines the availability of save storage.
	Game can try to call PlatformService::ObtainSaveDataAsync()
	which would reinitilize the storage access if possible.
	\return If the game is capable of accessing save storage.
	*/
	proto external bool IsSaveStorageAvailable();
	/*!
	\brief Backend Api access class
	*/
	proto external BackendApi GetBackendApi();
	proto external BackendDebugApi GetBackendDebugApi();
	/*!
	\brief RESTful Api request access class
	*/
	proto external RestApi GetRestApi();
	/*!
	\brief Begin preloading on given position with given radius. Call is non-blocking, to get status of preload use IsPreloadFinished method.
	\param world
	\param pos Position of place which to preload
	\param radius Radius of area which to preload
	\param maxTime_sec Maximum duration of preload in seconds <0, 300>
	\return Returns true when preload starts, returns false when there is nothing to preload
	*/
	proto external bool BeginPreload(notnull BaseWorld world, vector pos, float radius, int maxTime_sec = 60);
	/*!
	\brief If preload (started with BeginPreload method) is finished, returns true.
	*/
	proto external bool IsPreloadFinished();
	/*!
	\brief Returns load time in milliseconds for the lastly loaded world.
	*/
	proto external int GetLoadTime();
	/*!
	\brief Setting request flag for engine to exit the game
	*/
	proto external void RequestClose();
	/*!
	\brief Returns version of the game
	*/
	proto external string GetBuildVersion();
	/*!
	\brief Returns date and time when the game was built
	*/
	proto external string GetBuildTime();
	/*!
	\brief Returns path of world file loaded
	*/
	proto external GenericWorldEntity GetWorldEntity();
	proto external IEntity FindEntity(string name);
	/*!
	Checks if the game is in playmode (e.g. not in WorldEditor while editing the world)
	@return True if the game is in play mode. False otherwise.
	*/
	proto external bool InPlayMode();
	proto external int ReloadFailureAddons(out notnull array<string> addons);
	proto external PlatformService GetPlatformService();
	//! Return true if executable is developer build.
	static proto bool IsDev();
	proto external ref ScriptModule GetScriptModule();
	/*!
	\brief Setting request flag for the engine to reinitialize the game
	* Doesn't do anything in Workbench
	*/
	[Obsolete("Pass addon GUIDs to the transition request directly")]
	proto external void RequestReload(array<string> addonGUIDs = null);
	/*!
	Safely instantiate the entity and calls EOnInit if the entity sets event mask EntityEvent.INIT.
	\param typeName Name of entity's type to instantiate.
	\return instantiated entity
	*/
	proto external IEntity SpawnEntity(typename typeName, BaseWorld world = null, EntitySpawnParams params = null);
	/*!
	Safely instantiate the entity from template (with all components) and calls EOnInit if the entity sets event mask EntityEvent.INIT.
	\param templateResource Template resource of the entity to instantiate.
	\return instantiated entity
	*/
	proto external IEntity SpawnEntityPrefab(notnull Resource templateResource, BaseWorld world = null, EntitySpawnParams params = null);

	// callbacks

	// script callbacks
	event typename GetMenuPreset() { return GameLibMenusEnum; };
	/*!
	Creates handler for loading screen, its animation and resources. Override this
	in your Game implementation to customize loading screen look and behavior.
	Default implementation does nothing.

	\param[in]	workspaceWidget	The workspace widget used by the loading screen.
								LoadingAnim implementation may only use this
								WorkspaceWidget to work with UI.
	\return						Class implementing loading screen animation. It's fine to return null (though you probably want at least something).
	*/
	event LoadingAnim CreateLoadingAnim(WorkspaceWidget workspaceWidget);
	/*!
	\brief Called on World update
	@param world current world
	@param timeslice time elapsed from last call
	*/
	event void OnUpdate(BaseWorld world, float timeslice);
	/*!
	\brief Called after full initialization of Game instance
	*/
	event void OnAfterInit(BaseWorld world);
	/*!
	\brief Event which is called right before game starts (all entities are created and initialized). Returns true if the game can start.
	*/
	event bool OnGameStart() { return true; };
	/*!
	\brief Event which is called right before game end.
	*/
	event void OnGameEnd();
	/*!
	\brief Called when some system event occur. (NOTE: currently not called because of problems with recursion)
	@param eventTypeId event type.
	@param params Tuple object, cast to specific Tuple class to get parameters for particular event.
	*/
	event void OnEvent(EventType eventTypeId, Tuple params);
	/*!
	\brief Event which is called when user change settings.
	*/
	event void OnUserSettingsChangedEvent();
	/*!
	\brief Event which is called when input device binded to user changed.
	*/
	event void OnInputDeviceUserChangedEvent(EInputDeviceType oldDevice, EInputDeviceType newDevice);
	/*!
	\brief Event which is called when input device binded to user changes between gamepad and keyboard/mouse, ignoring Joystick and TrackIR.
	*/
	event void OnInputDeviceIsGamepadEvent(bool isGamepad);
	/*!
	\brief Event called once loading of all entities of the world have been finished. (still within the loading)
	*/
	event void OnWorldPostProcess(World world);
	/*!
	\brief Event which is called before each fixed step of the physics simulation
	@param timeSlice simulation step length
	*/
	event void OnWorldSimulatePhysics(float timeSlice);
	/*
	\brief Event which is called after each fixed step of the physics simulation
	*param timeSlice simulation step length
	*/
	event void OnWorldPostSimulatePhysics(float timeSlice);
	/*!
	\brief Event which is called when window size of fullscreen state changed
	@param w new width of window in screen pixels
	@param h new height of window in screen pixels
	@param windowed true when is windowed, false then fullscreen
	*/
	event void OnWindowResize(int w, int h, bool windowed);
	//! Called after reload to host a modded scenario.
	event void HostGameConfig();
	event void PlayGameConfig(ResourceName sResource, string addonsList);
	event ref array<ResourceName> GetDefaultGameConfigs();
	event Managed ReadGameConfig(string sResource);
	/*!
	\brief Event which is called on Gamepad Connection/Disconnection.
	@param isConnected is false on disconnection, true on re-connection.
	*/
	event void OnGamepadConnectionStatus(bool isConnected);
}
