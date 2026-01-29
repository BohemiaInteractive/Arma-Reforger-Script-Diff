class SCR_ScenarioFrameworkSystem : GameSystem
{
	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)												// TODO: move down with other methods
	{
		outInfo
			.SetAbstract(false)
			.AddPoint(ESystemPoint.FixedFrame);
	}

	[Attribute("Available Tasks for the Scenario", category: "Tasks")]
	ref array<ref SCR_ScenarioFrameworkTaskType> m_aTaskTypesAvailable;

	[Attribute(defvalue: "3", desc: "Maximal number of tasks that can be generated", category: "Tasks")]
	int m_iMaxNumberOfTasks;

	[Attribute(UIWidgets.Auto, desc: "Actions that will be activated after tasks are initialized", category: "Tasks")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aAfterTasksInitActions;

	[Attribute(desc: "Debug actions accessible from Debug Menu.", category: "Debug")]
	ref array<ref SCR_ScenarioFrameworkDebugAction> m_aDebugActions;

	[Attribute(desc: "List of Core Areas that are essential for the Scenario to spawn alongside Debug Areas", category: "Debug")]
	ref array<string> m_aCoreAreas;

	[Attribute(desc: "List of Areas that will be spawned (Optionally with desired Layer Task) as opposed to leaving it to random generation", category: "Debug")]
	ref array<ref SCR_ScenarioFrameworkDebugArea> m_aDebugAreas;

	[Attribute(desc: "Should the dynamic Spawn/Despawn based on distance from player characters be enabled for the whole GameMode?", category: "Dynamic Spawn/Despawn")]
	bool m_bDynamicDespawn;

	[Attribute(defvalue: "4", UIWidgets.Slider, params: "0 600 1", desc: "How frequently is dynamic spawn/despawn being checked in seconds", category: "Dynamic Spawn/Despawn")]
	int m_iUpdateRate;

	[Attribute(desc: "Config with voice over data for whole scenario", params: "conf class=SCR_VoiceoverData")]
	ResourceName m_sVoiceOverDataConfig;

	bool m_bMatchOver;
	bool m_bDebugInit;
	bool m_bIsWorldLoadInit;
	bool m_bIsSaveGameLoad;

	ref ScriptInvoker m_OnAllAreasInitiated;
	ref ScriptInvoker m_OnTaskStateChanged;
	static ref ScriptInvokerBase<ScriptInvokerScenarioFrameworkSlotAIMethod> m_OnSlotAISpawned;
	protected FactionManager m_FactionManager;
	protected SCR_TaskSystem m_TaskSystem;

	SCR_ScenarioFrameworkLayerBase m_LastFinishedTaskLayer;
	SCR_Task m_LastFinishedTask;
	EGameOverTypes m_eGameOverType = EGameOverTypes.COMBATPATROL_DRAW;

	ref array<SCR_ScenarioFrameworkArea> m_aAreas = {};
	ref array<SCR_ScenarioFrameworkArea> m_aSelectedAreas = {};
	ref array<SCR_ScenarioFrameworkLayerTask> m_aLayerTasksToBeInitialized = {};
	ref array<SCR_ScenarioFrameworkLayerTask> m_aLayerTasksForRandomization = {};
	ref array<int> m_aIntroVoicelineIndexes = {};
	ref array<string> m_aAreasTasksToSpawn = {};
	ref array<string> m_aLayersTaskToSpawn = {};
	ref array<string> m_aSlotsTaskToSpawn = {};
	ref array<SCR_ESFTaskType> m_aESFTaskTypesAvailable = {};
	ref array<SCR_ESFTaskType> m_aESFTaskTypeForRandomization = {};

	ref array<ref Tuple3<SCR_ScenarioFrameworkArea, vector, int>> m_aSpawnedAreas = {};
	ref array<ref Tuple3<SCR_ScenarioFrameworkArea, vector, int>> m_aDespawnedAreas = {};
	ref array<ref Tuple3<EntityID, bool, float>> m_aDebugShapesLayers = {};
	ref array<vector> m_aObservers = {};
	ref map<string, string> m_mVariableMap = new map<string, string>;

	protected static ref ScriptCallQueue s_CallQueuePausable = new ScriptCallQueue();
	protected float m_fTimer;
	protected float m_fCheckInterval;

	//------------------------------------------------------------------------------------------------
	//! \return the instance of ScenarioFrameworkSystem.
	static SCR_ScenarioFrameworkSystem GetInstance()
	{
		World world = GetGame().GetWorld();
		if (!world)
			return null;

		return SCR_ScenarioFrameworkSystem.Cast(world.FindSystem(SCR_ScenarioFrameworkSystem));
	}

	//------------------------------------------------------------------------------------------------
	//! \return the instance of ScenarioFramework Call Queue.
	static ScriptCallQueue GetCallQueuePausable()
	{
		return s_CallQueuePausable;
	}

	//------------------------------------------------------------------------------------------------
	//! \return a ScriptInvokerBase object representing the invoker triggered when SlotAI spawned AI character/group.
	static ScriptInvokerScenarioFrameworkSlotAI GetOnSlotAISpawned()
	{
		if (!m_OnSlotAISpawned)
			m_OnSlotAISpawned = new ScriptInvokerBase<ScriptInvokerScenarioFrameworkSlotAIMethod>();

		return m_OnSlotAISpawned;
	}

	//------------------------------------------------------------------------------------------------
	static void InvokeSlotAISpawned(SCR_ScenarioFrameworkLayerBase layer, IEntity entity)
	{
		if (m_OnSlotAISpawned)
			m_OnSlotAISpawned.Invoke(layer, entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Is this system should be paused while the simulation is paused?
	override event bool ShouldBePaused()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets signal value on entity by ID.
	//! \param[in] pEntID Entity ID of an object in the game world.
	//! \param[in] signalName Signal name is an identifier for a specific signal within an entity, used to set its value in the method.
	//! \param[in] val val represents the value to set for the specified signal on the entity.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_SetSignalOnEntity(EntityID pEntID, string signalName, int val)
	{
		if (!pEntID)
			return;

		IEntity entity = GetGame().GetWorld().FindEntityByID(pEntID);
		if (!entity)
			return;

		SignalsManagerComponent signalComponent = SignalsManagerComponent.Cast(entity.FindComponent(SignalsManagerComponent));
		if (!signalComponent)
			return;

		signalComponent.SetSignalValue(signalComponent.FindSignal(signalName), val);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets signal value on entity via RPC.
	//! \param[in] entity Represents an in-game object with properties and behavior.
	//! \param[in] signalName SignalName is the name of the signal on the entity to set its value.
	//! \param[in] val val: Integer representing signal value to set on entity.
	void SetSignalValue(IEntity entity, string signalName, int val)
	{
		if (!entity)
			return;

		if (IsMaster())
			Rpc(RpcDo_SetSignalOnEntity, entity.GetID(), signalName, val);

		RpcDo_SetSignalOnEntity(entity.GetID(), signalName, val);
	}

	//------------------------------------------------------------------------------------------------
	//! Plays sound on entity by ID.
	//! \param[in] pEntID Entity ID representing an object in the game world.
	//! \param[in] sSndName Sound name for playing sound on entity.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PlaySoundOnEntity(EntityID pEntID, string sSndName)
	{
		if (!pEntID)
			return;

		IEntity entity = GetGame().GetWorld().FindEntityByID(pEntID);
		if (!entity)
			return;

		SoundComponent pSndComp = SoundComponent.Cast(entity.FindComponent(SoundComponent));
		if (!pSndComp)
			return;

		pSndComp.SoundEvent(sSndName);
	}

	//------------------------------------------------------------------------------------------------
	//! Plays sound on entity or game mode if entity is null.
	//! \param[in] entity Represents an in-game object or character.
	//! \param[in] sSndName sSndName is the name of the sound file to play on the specified entity.
	void PlaySoundOnEntity(IEntity entity, string sSndName)
	{
		if (!entity)
			entity = GetGame().GetGameMode();		//play it on game mode if any entity is passed

		if (!entity)
			return;

		if (IsMaster())
			Rpc(RpcDo_PlaySoundOnEntity, entity.GetID(), sSndName);

		RpcDo_PlaySoundOnEntity(entity.GetID(), sSndName);
	}

	//------------------------------------------------------------------------------------------------
	//! Plays sound on entity by ID.
	//! \param[in] pEntID Entity ID representing an object in the game world.
	//! \param[in] sSndName Sound name for playing sound on entity.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PlayCommunicationSoundOnEntity(EntityID pEntID, string sSndName)
	{
		if (!pEntID)
			return;

		IEntity entity = GetGame().GetWorld().FindEntityByID(pEntID);
		if (!entity)
			return;

		CommunicationSoundComponent pSndComp = CommunicationSoundComponent.Cast(entity.FindComponent(CommunicationSoundComponent));
		if (!pSndComp)
			return;

		pSndComp.SoundEvent(sSndName);
	}

	//------------------------------------------------------------------------------------------------
	//! Plays communication sound on entity or game mode if entity is null.
	//! \param[in] entity Represents an in-game object or character.
	//! \param[in] sSndName sSndName is the name of the sound file to play on the specified entity.
	void PlayCommunicationSoundOnEntity(IEntity entity, string sSndName)
	{
		if (!entity)
			entity = GetGame().GetGameMode();		//play it on game mode if any entity is passed

		if (!entity)
			return;

		if (IsMaster())
			Rpc(RpcDo_PlayCommunicationSoundOnEntity, entity.GetID(), sSndName);

		RpcDo_PlayCommunicationSoundOnEntity(entity.GetID(), sSndName);
	}

	//------------------------------------------------------------------------------------------------
	//! Plays intro voiceline for player with specified event name and entity ID.
	//! \param[in] playerID Player ID represents the unique identifier for the player in the game.
	//! \param[in] eventName Plays intro voiceline for event.
	//! \param[in] entityID EntityID represents the ID of the entity playing the intro voiceline.
	void PlayIntroVoiceline(int playerID, string eventName, EntityID entityID)
	{
		Rpc(RpcDo_PlayIntroVoiceline, playerID, eventName, entityID);
		RpcDo_PlayIntroVoiceline(playerID, eventName, entityID);
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//! Plays intro voiceline event for player, sets objectives based on indexes, and plays sound event.
	//! \param[in] playerID Player ID represents the unique identifier for the player in the game.
	//! \param[in] eventName EventName is the name of the sound event to be played for the player when intro voiceline is triggered.
	//! \param[in] entityID EntityID represents an optional parameter for specifying an entity in the game world to which the intro voice line should be played.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PlayIntroVoiceline(int playerID, string eventName, EntityID entityID)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		if (playerID != playerController.GetPlayerId())
			return;

		IEntity entity;
		if (entityID)
		{
			entity = GetGame().GetWorld().FindEntityByID(entityID);
			if (!entity)
				return;
		}
		else
		{
			entity = playerManager.GetPlayerControlledEntity(playerID);
			if (!entity)
				return;
		}

		SignalsManagerComponent signalComp = SignalsManagerComponent.Cast(entity.FindComponent(SignalsManagerComponent));
		if (!signalComp)
			return;

		if (m_aIntroVoicelineIndexes.IsEmpty())
		{
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("PlanName"), 0);
		}
		else
		{
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("PlanName"), Math.RandomIntInclusive(0, 2));
		}

		int indexCount = m_aIntroVoicelineIndexes.Count();

		if (indexCount > 0)
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective1"), m_aIntroVoicelineIndexes[0]);
		else
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective1"), 1);

		if (indexCount > 1)
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective2"), m_aIntroVoicelineIndexes[1]);
		else
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective2"), 2);

		if (indexCount > 2)
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective3"), m_aIntroVoicelineIndexes[2]);
		else
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective3"), 3);

		if (indexCount > 3)
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective4"), m_aIntroVoicelineIndexes[3]);
		else
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective4"), 4);

		if (indexCount > 4)
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective5"), m_aIntroVoicelineIndexes[4]);
		else
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective5"), 5);

		if (indexCount > 5)
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective6"), m_aIntroVoicelineIndexes[5]);
		else
			signalComp.SetSignalValue(signalComp.AddOrFindSignal("Objective6"), 6);

		SoundComponent soundComp = SoundComponent.Cast(entity.FindComponent(SoundComponent));
		if (!soundComp)
			return;

		soundComp.SoundEvent(eventName);
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//! Plays sound on entity position with given sound file and event name.
	//! \param[in] objectID Represents an entity in the game world.
	//! \param[in] soundFile Sound file is the audio resource played at the entity's position.
	//! \param[in] soundEventName SoundEventName is the name of the sound event to play on the specified entity's position.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PlaySoundOnEntityPosition(RplId objectID, string soundFile, string soundEventName)
	{
		IEntity object = IEntity.Cast(Replication.FindItem(objectID));
		if (!object)
			return;

		SCR_AudioSourceConfiguration audioConfig = new SCR_AudioSourceConfiguration();
		audioConfig.m_sSoundProject = soundFile;
		audioConfig.m_sSoundEventName = soundEventName;
		audioConfig.m_eFlags = EAudioSourceConfigurationFlag.FinishWhenEntityDestroyed;

		SCR_SoundManagerModule.CreateAndPlayAudioSource(object, audioConfig);
	}

	//------------------------------------------------------------------------------------------------
	//! Plays sound event on the position of provided entity
	//! \param[in] object where taht sound will be played
	//! \param[in] soundFile resource name of a sound file that contains desired event
	//! \param[in] soundEventName name of a sound event that will be used to play a sound
	void PlaySoundOnEntityPosition(IEntity object, string soundFile, string soundEventName)
	{
		RplId objectID = Replication.FindId(object);
		if (!objectID.IsValid())
			return;

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(soundFile))
			return;

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(soundEventName))
			return;

		if (IsMaster())
			Rpc(RpcDo_PlaySoundOnEntityPosition, objectID, soundFile, soundEventName);

		RpcDo_PlaySoundOnEntityPosition(objectID, soundFile, soundEventName);
	}

	//------------------------------------------------------------------------------------------------
	//! \return Last finished task.
	SCR_Task GetLastFinishedTask()
	{
		return m_LastFinishedTask;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Last finished task layer in scenario execution.
	SCR_ScenarioFrameworkLayerBase GetLastFinishedTaskLayer()
	{
		return m_LastFinishedTaskLayer;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Array of scenario framework areas.
	array<SCR_ScenarioFrameworkArea> GetAreas()
	{
		return m_aAreas;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] task Displays a message when a new task is created, indicating if it's for a specific faction or not.
	void OnTaskAdded(SCR_Task task)
	{
		s_CallQueuePausable.CallLater(OnTaskAddedCalledLater, 200, false, task)
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] task Displays a message when a new task is created, indicating if it's for a specific faction or not.
	void OnTaskAddedCalledLater(SCR_Task task)
	{
		SCR_ScenarioFrameworkTask sfTask = SCR_ScenarioFrameworkTask.Cast(task);
		if (!sfTask)
			return;

		if (!SCR_Enum.HasFlag(sfTask.GetTaskNotificationSettings(), SCR_ETaskNotificationSettings.ON_CREATED))
			return;

		if (!m_TaskSystem)
		{
			m_TaskSystem = SCR_TaskSystem.GetInstance();
			if (!m_TaskSystem)
				return;
		}

		array<string> factionKeys = {};
		factionKeys = m_TaskSystem.GetTaskFactions(task);
		if (!factionKeys)
			return;
		
		SCR_ScenarioFrameworkLayerTask layerTask = sfTask.GetLayerTask();
		if (!layerTask)
			return;
		
		string titleParam = layerTask.GetOverridenObjectDisplayName();
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(titleParam) && layerTask.m_SlotTask)
			titleParam = layerTask.m_SlotTask.GetSpawnedEntityDisplayName();
		
		foreach (string factionKey : factionKeys)
		{
 			PopUpMessage(layerTask.GetTaskTitle(), "#AR-CampaignTasks_NewObjectivesAvailable-UC", factionKey, titleParam1:titleParam);
		}
	}


	//------------------------------------------------------------------------------------------------
	//! Updates task properties, checks if task is finished, and triggers pop-up messages or slot task updates based on event
	//! \param[in] task Updates task properties, checks for finished tasks, triggers pop-up messages, and invokes onTaskStateChanged event
	//! \param[in] taskState representing changes in task properties, excluding creation, completion, or assignee changes.
	void OnTaskUpdate(SCR_Task task, SCR_ETaskState taskState)
	{
		if (!m_TaskSystem)
		{
			m_TaskSystem = SCR_TaskSystem.GetInstance();
			if (!m_TaskSystem)
				return;
		}

		SCR_ScenarioFrameworkTask frameworkTask = SCR_ScenarioFrameworkTask.Cast(task);
		if (!frameworkTask)
			return;

		SCR_ScenarioFrameworkLayerTask layerTask = frameworkTask.GetLayerTask();
		if (!layerTask)
			return;

		SCR_ScenarioFrameworkSlotTask slotTask = layerTask.GetSlotTask();
		if (!slotTask)
		{
			slotTask = frameworkTask.GetSlotTask();
			if (!slotTask)
				return;
		}

		slotTask.OnTaskStateChanged(taskState);

		array<string> factionKeys = {};
		factionKeys = m_TaskSystem.GetTaskFactions(task);
		if (!factionKeys)
			return;

		if (m_bIsSaveGameLoad)
			return; // No notification on load of save game
		
		string titleParam = layerTask.GetOverridenObjectDisplayName();
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(titleParam))
			titleParam = slotTask.GetSpawnedEntityDisplayName();
		
		foreach (string factionKey : factionKeys)
		{
			if (task.GetTaskState() == SCR_ETaskState.COMPLETED)
			{
				m_LastFinishedTaskLayer = SCR_ScenarioFrameworkTask.Cast(task).GetLayerTask();
				m_LastFinishedTask = task;

				if (SCR_Enum.HasFlag(frameworkTask.GetTaskNotificationSettings(), SCR_ETaskNotificationSettings.ON_FINISH))
					PopUpMessage(layerTask.GetTaskTitle(), "#AR-Tasks_StatusFinished-UC", factionKey, titleParam1:titleParam);
			}
			else if (taskState == SCR_ETaskState.FAILED)
			{
				if (SCR_Enum.HasFlag(frameworkTask.GetTaskNotificationSettings(), SCR_ETaskNotificationSettings.ON_FAILED))
					PopUpMessage(layerTask.GetTaskTitle(), "#AR-Tasks_StatusFailed-UC", factionKey, titleParam1:titleParam);
			}
			else if (taskState == SCR_ETaskState.CANCELLED)
			{
				if (SCR_Enum.HasFlag(frameworkTask.GetTaskNotificationSettings(), SCR_ETaskNotificationSettings.ON_CANCELLED))
					PopUpMessage(layerTask.GetTaskTitle(), "#AR-Tasks_StatusCancelled-UC", factionKey, titleParam1:titleParam);
			}
			else if (taskState == SCR_ETaskState.PROGRESSED)
			{
				if (SCR_Enum.HasFlag(frameworkTask.GetTaskNotificationSettings(), SCR_ETaskNotificationSettings.ON_UPDATED))
					PopUpMessage(layerTask.GetTaskTitle(), "#AR-Workshop_ButtonUpdate", factionKey, titleParam1:titleParam);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] GameOverType Represents mission end type for displaying appropriate end screen in game.
	void SetMissionEndScreen(EGameOverTypes GameOverType)
	{
		m_eGameOverType = GameOverType;
	}

	//------------------------------------------------------------------------------------------------
	//! Finishes current game mode, sets match over flag, and ends game mode with specified type.
	void Finish()
	{
		SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(m_eGameOverType, 0, 0);

		m_bMatchOver = true;

		SCR_BaseGameMode.Cast(GetGame().GetGameMode()).EndGameMode(endData);
	}

	//------------------------------------------------------------------------------------------------
	//! \return whether match is over.
	bool GetIsMatchOver()
	{
		return m_bMatchOver;
	}

	//------------------------------------------------------------------------------------------------
	//! \return a ScriptInvoker object for handling all area initiation events.
	ScriptInvoker GetOnAllAreasInitiated()
	{
		if (!m_OnAllAreasInitiated)
			m_OnAllAreasInitiated = new ScriptInvoker();

		return m_OnAllAreasInitiated;
	}

	//------------------------------------------------------------------------------------------------
	//! Selects nearest area with suitable task type for last finished task position.
	//! \param[in] eTaskType eTaskType represents the type of task required for selecting an area in the scenario framework.
	//! \return Selects nearest area suitable for given task type based on last finished task position.
	SCR_ScenarioFrameworkArea SelectNearestAreaByTaskType(SCR_ESFTaskType eTaskType)
	{
		if (m_aAreas.IsEmpty())
			return null;

		SCR_ScenarioFrameworkArea selectedArea = null;
		if (!m_LastFinishedTask)
			return null;

		vector vTaskPos = m_LastFinishedTask.GetOrigin();
		float fMinDistance = float.MAX;
		float fDistance = 0;
		for (int i = 0, count = m_aAreas.Count(); i < count; i++)
		{
			if (!m_aAreas[i].GetIsTaskSuitableForArea(eTaskType))
				continue;

			fDistance = vector.Distance(vTaskPos, m_aAreas[i].GetOwner().GetOrigin());
			if (fDistance < fMinDistance)
			{
				fMinDistance = fDistance;
				selectedArea = m_aAreas[i];
			}
		}
		return selectedArea;
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//! Checks if current game mode has RplComponent and is not a proxy.
	//! \return true if the current game mode is not a proxy RplComponent, otherwise false.
	bool IsMaster() // IsServer
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return false;

		RplComponent comp = RplComponent.Cast(gameMode.FindComponent(RplComponent));
		if (!comp)
			return false;			//by purpose - debug

		return !comp.IsProxy();
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//! Registers new area in scenario framework areas list.
	//! \param[in] area to be registered.
	void RegisterArea(SCR_ScenarioFrameworkArea area)
	{
		if (!m_aAreas.Contains(area))
			m_aAreas.Insert(area);
	}

	//------------------------------------------------------------------------------------------------
	//! Loads component settings from SCR_GameModeSFManager
	void LoadComponentSettings()
	{
		SCR_GameModeSFManager sfManager = SCR_GameModeSFManager.Cast(GetGame().GetGameMode().FindComponent(SCR_GameModeSFManager));
		if (!sfManager)
			return;

		m_aTaskTypesAvailable = sfManager.m_aTaskTypesAvailable;
		m_iMaxNumberOfTasks = sfManager.m_iMaxNumberOfTasks;
		m_aAfterTasksInitActions = sfManager.m_aAfterTasksInitActions;
		m_aDebugActions = sfManager.m_aDebugActions;
		m_aCoreAreas = sfManager.m_aCoreAreas;
		m_aDebugAreas = sfManager.m_aDebugAreas;
		m_bDynamicDespawn = sfManager.m_bDynamicDespawn;
		m_iUpdateRate = sfManager.m_iUpdateRate;
		m_sVoiceOverDataConfig = sfManager.m_sVoiceOverDataConfig;
	}

	//------------------------------------------------------------------------------------------------
	//! Loads mission header settings
	void LoadHeaderSettings()
	{
		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (!header)
			return;

		SCR_MissionHeaderScenarioFramework scenarioFrameworkHeader = SCR_MissionHeaderScenarioFramework.Cast(header);
		if (!scenarioFrameworkHeader)
			return;

		if (scenarioFrameworkHeader.m_iMaxNumberOfTasks != -1)
			m_iMaxNumberOfTasks = scenarioFrameworkHeader.m_iMaxNumberOfTasks;

		if (!scenarioFrameworkHeader.m_aTaskTypesAvailable.IsEmpty())
		{
			m_aTaskTypesAvailable.Clear();
			foreach (SCR_ScenarioFrameworkTaskType taskType : scenarioFrameworkHeader.m_aTaskTypesAvailable)
			{
				m_aTaskTypesAvailable.Insert(taskType);
			}
		}

		if (!scenarioFrameworkHeader.m_aDebugAreas.IsEmpty())
		{
			m_aDebugAreas.Clear();
			foreach (SCR_ScenarioFrameworkDebugArea debugArea : scenarioFrameworkHeader.m_aDebugAreas)
			{
				m_aDebugAreas.Insert(debugArea);
			}
		}

		if (scenarioFrameworkHeader.m_iDynamicDespawn != -1)
		{
			if (scenarioFrameworkHeader.m_iDynamicDespawn == 0)
				m_bDynamicDespawn = false;
			else
				m_bDynamicDespawn = true;
		}

		if (scenarioFrameworkHeader.m_iUpdateRate != -1)
			m_iUpdateRate = scenarioFrameworkHeader.m_iUpdateRate;
	}

	//------------------------------------------------------------------------------------------------
	//! Registers debug menus for ScenarioFramework in DiagMenu.
	override event protected void OnInit()
	{
		super.OnInit();

		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_MENU, "ScenarioFramework", "");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_TASKS, "", "Tasks", "ScenarioFramework");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_REGISTERED_AREAS, "", "Registered Areas", "ScenarioFramework");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_DEBUG_AREAS, "", "Debug Areas", "ScenarioFramework");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_LAYER_INSPECTOR, "", "Layer Inspector", "ScenarioFramework");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_ACTION_INSPECTOR, "", "Action Inspector", "ScenarioFramework");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_LOGIC_INSPECTOR, "", "Logic Inspector", "ScenarioFramework");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_PLUGIN_INSPECTOR, "", "Plugin Inspector", "ScenarioFramework");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_CONDITION_INSPECTOR, "", "Condition Inspector", "ScenarioFramework");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SCENARIO_FRAMEWORK_DEBUG_ACTIONS, "", "Debug Actions", "ScenarioFramework");

		m_FactionManager = GetGame().GetFactionManager();
		SCR_Task.GetOnTaskStateChanged().Remove(OnTaskUpdate);
		SCR_Task.GetOnTaskStateChanged().Insert(OnTaskUpdate);

		m_TaskSystem = SCR_TaskSystem.GetInstance();
		if (m_TaskSystem)
		{
			m_TaskSystem.GetOnTaskAdded().Remove(OnTaskAdded);
			m_TaskSystem.GetOnTaskAdded().Insert(OnTaskAdded);
		}

		m_bIsWorldLoadInit = true;

		Init();

		m_bIsWorldLoadInit = false;
		m_bIsSaveGameLoad = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes scenario framework system
	//! \return The return value represents whether the initialization process was successful or not.
	bool Init()
	{
		if (!IsMaster())
			return false;

		GetOnAllAreasInitiated().Remove(PrepareDynamicDespawn);
		GetOnAllAreasInitiated().Insert(PrepareDynamicDespawn);

		if (!m_bDebugInit)
		{
			LoadComponentSettings();
			LoadHeaderSettings();
		}
		else
		{
			m_bDebugInit = false;
		}

		if (m_aDebugAreas.IsEmpty())
		{
			foreach (SCR_ScenarioFrameworkArea area : m_aAreas)
			{
				if (area.GetDynamicDespawnEnabled())
					continue;

				area.Init();
			}
		}
		else
		{
			SCR_ScenarioFrameworkArea area;
			SCR_ScenarioFrameworkLayerTask layerTask;
			SCR_ScenarioFrameworkSlotTask slotTask;
			foreach (SCR_ScenarioFrameworkDebugArea debugArea : m_aDebugAreas)
			{
				area = debugArea.GetForcedArea();
				if (!area)
					continue;

				if (!m_aAreasTasksToSpawn.Contains(area.GetName()))
					m_aAreasTasksToSpawn.Insert(area.GetName());

				if (!area.GetDynamicDespawnEnabled())
					area.Init();

				layerTask = debugArea.GetForcedLayerTask();
				if (layerTask && !m_aLayersTaskToSpawn.Contains(layerTask.GetName()))
					m_aLayersTaskToSpawn.Insert(layerTask.GetName());

				slotTask = debugArea.GetForcedSlotTask();
				if (!slotTask || (slotTask && m_aSlotsTaskToSpawn.Contains(slotTask.GetName())))
				{
					m_aSlotsTaskToSpawn.Insert("0");
				}
				else
				{
					m_aSlotsTaskToSpawn.Insert(slotTask.GetName());
				}
			}

			IEntity entity;
			foreach (string coreArea : m_aCoreAreas)
			{
				if (SCR_StringHelper.IsEmptyOrWhiteSpace(coreArea))
					continue;

				entity = GetGame().GetWorld().FindEntityByName(coreArea);
				if (!entity)
					continue;

				area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
				if (area && !area.GetDynamicDespawnEnabled())
					area.Init()
			}
		}

		//if someone registered for the event, then call it
		if (m_OnAllAreasInitiated)
			m_OnAllAreasInitiated.Invoke();

		PostInit();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Generates tasks for debug shape layers on player connect, if any.
	void PostInit()
	{
		GenerateTasks();

		if (m_aDebugShapesLayers.IsEmpty())
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_BaseGameMode scriptedGameMode = SCR_BaseGameMode.Cast(gameMode);
		if (!scriptedGameMode)
			return;

		scriptedGameMode.GetOnPlayerConnected().Remove(OnPlayerConnected);
		scriptedGameMode.GetOnPlayerConnected().Insert(OnPlayerConnected);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerConnected(int playerID)
	{
		SyncDebugShapes(playerID);

		if (m_mVariableMap.IsEmpty())
			return;

		foreach (string key, string value : m_mVariableMap)
		{
			Rpc(RpcDo_CreateVariableValue, key, value, playerID);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Synchronizes debug shapes for player with ID, manages layer visibility and position.
	//! \param[in] playerID Player ID represents the unique identifier for the player controlling the action in the method.
	void SyncDebugShapes(int playerID)
	{
		int count = m_aDebugShapesLayers.Count();
		for (int i = 0; i < count; i++)
		{
			Tuple3<EntityID, bool, float> debugLayer = m_aDebugShapesLayers[i];
			Rpc(RpcDo_ManageLayerDebugShape, playerID, debugLayer.param1, debugLayer.param2, debugLayer.param3);
		}
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//! Spawns random tasks in an area, removing duplicates and ensuring task type balance.
	//! \param[in] countSafe CountSafe represents the number of tasks already spawned in the scenario, used for limiting the maximum number of tasks that can be
	void SpawnRandomTask(int countSafe = 0)
	{
		if (countSafe >= m_iMaxNumberOfTasks)
			return;

		// In case of more max tasks to be spawned but with less task types, we need to refill it again
		if (m_aESFTaskTypeForRandomization.IsEmpty())
			m_aESFTaskTypeForRandomization.Copy(m_aESFTaskTypesAvailable);

		if (m_aLayerTasksForRandomization.IsEmpty())
			return;

		SCR_ScenarioFrameworkLayerTask layerTask = m_aLayerTasksForRandomization.GetRandomElement();
		if (!layerTask)
			return;

		// This filters out task types that were already spawned
		SCR_ESFTaskType taskType = layerTask.GetTaskType();
		if (!m_aESFTaskTypeForRandomization.Contains(taskType))
		{
			for (int i = m_aLayerTasksForRandomization.Count() - 1; i >= 0; i--)
			{
				if (m_aLayerTasksForRandomization[i].GetTaskType() == taskType)
					m_aLayerTasksForRandomization.Remove(i);
			}

			if (m_aESFTaskTypesAvailable.Contains(taskType))
				SpawnRandomTask(countSafe + 1);
		}
		m_aESFTaskTypeForRandomization.RemoveItem(taskType);

		//Spawning and setting necessary states
		SCR_ScenarioFrameworkArea area = layerTask.GetParentArea();
		m_aLayerTasksToBeInitialized.Insert(layerTask);
		layerTask.SetParentLayer(layerTask.GetParentLayer());
		layerTask.Init(area, SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT);
		area.SetLayerTask(layerTask);
		area.SetAreaSelected(true);
		m_aSelectedAreas.Insert(area);

		// Removing all Layer Tasks that have the same Task Type
		for (int i = m_aLayerTasksForRandomization.Count() - 1; i >= 0; i--)
		{
			if (m_aLayerTasksForRandomization[i].GetTaskType() == taskType)
				m_aLayerTasksForRandomization.Remove(i);
		}

		// Removing all Layer Tasks that are from the same Area of randomly selected Layer Task
		for (int i = m_aLayerTasksForRandomization.Count() - 1; i >= 0; i--)
		{
			if (m_aLayerTasksForRandomization[i].GetParentArea() == area)
				m_aLayerTasksForRandomization.Remove(i);
		}

		Print(string.Format("ScenarioFramework: Creating area %1 with Layer Task %2", area.GetOwner().GetName(), layerTask.GetOwner().GetName()), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Main function responsible for selecting available tasks and spawning the areas related to the tasks
	void GenerateTasks()
	{
		if (m_aTaskTypesAvailable.IsEmpty())
		{
			Print("ScenarioFramework: Available tasks are empty, no new tasks will be generated.", LogLevel.NORMAL);
			return;
		}

		if (m_aAreas.IsEmpty())
		{
			Print("ScenarioFramework: There are no Areas to generate tasks from", LogLevel.NORMAL);
			return;
		}

		Print("ScenarioFramework: ---------------------- Generating tasks -------------------", LogLevel.NORMAL);

		if (!m_aLayersTaskToSpawn.IsEmpty())
		{
			for (int i = 0; i < m_aLayersTaskToSpawn.Count(); i++)
			{
				GenerateSingleTask(i);
			}

			AfterLayerTasksInit();
			Print("ScenarioFramework: ---------------------- Generation of tasks completed -------------------", LogLevel.NORMAL);

			//If counts are not the same, we want randomization to occur
			if (m_aAreasTasksToSpawn.Count() == m_aLayersTaskToSpawn.Count())
				return;
		}

		//Fetching all Layer Tasks from Areas
		array<SCR_ScenarioFrameworkLayerTask> layerTasksToRandomize = {};
		array<SCR_ScenarioFrameworkLayerTask> layerTasks = {};
		if (m_aDebugAreas.IsEmpty())
		{
			foreach (SCR_ScenarioFrameworkArea area : m_aAreas)
			{
				if (!area)
					continue;

				area.GetAllLayerTasks(layerTasks);
				layerTasksToRandomize.InsertAll(layerTasks);
			}
		}
		else
		{
			SCR_ScenarioFrameworkArea forcedArea;
			SCR_ScenarioFrameworkArea forcedLayerTask;
			foreach (SCR_ScenarioFrameworkDebugArea debugArea : m_aDebugAreas)
			{
				forcedArea = debugArea.GetForcedArea();
				if (!forcedArea)
					continue;

				//If debug area has LayerTask set, we don't want to put it for randomization
				if (debugArea.GetForcedLayerTask())
					continue;

				forcedArea.GetAllLayerTasks(layerTasks);
				layerTasksToRandomize.InsertAll(layerTasks);
			}
		}

		//Fetching available Task Types for generation based on type
		foreach (SCR_ScenarioFrameworkTaskType taskTypeClass : m_aTaskTypesAvailable)
		{
			m_aESFTaskTypesAvailable.Insert(taskTypeClass.GetTaskType());
		}

		//Removing Layer Tasks that don't have Task Type set in Available Task Types
		for (int i = layerTasksToRandomize.Count() - 1; i >= 0; i--)
		{
			if (!m_aESFTaskTypesAvailable.Contains(layerTasksToRandomize[i].GetTaskType()))
				layerTasksToRandomize.Remove(i);
		}

		//Removing Layer Tasks that don't have activation type set to ON_TASKS_INIT
		for (int i = layerTasksToRandomize.Count() - 1; i >= 0; i--)
		{
			if (layerTasksToRandomize[i].GetActivationType() != SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT)
				layerTasksToRandomize.Remove(i);
		}

		// Cleans unused task types
		for (int i = m_aESFTaskTypesAvailable.Count() - 1; i >= 0; i--)
		{
			bool invalidType = true;
			foreach (SCR_ScenarioFrameworkLayerTask layerTask : m_aLayerTasksForRandomization)
			{
				if (layerTask.GetTaskType() == m_aESFTaskTypesAvailable[i])
				{
					invalidType = false;
					break;
				}

				if (invalidType)
					m_aESFTaskTypesAvailable.Remove(m_aESFTaskTypesAvailable[i])
			}
		}

		//Creating a copy so we can work with these in loops
		m_aESFTaskTypeForRandomization.Copy(m_aESFTaskTypesAvailable);
		m_aLayerTasksForRandomization.Copy(layerTasksToRandomize);

		//Spawning desired number of tasks
		for (int i = 0; i < m_iMaxNumberOfTasks; i++)
		{
			//Layers Tasks are shrinked down and after certain number of generations, we need to refill it
			if (m_aLayerTasksForRandomization.IsEmpty())
			{
				m_aLayerTasksForRandomization.Copy(layerTasksToRandomize);

				foreach (SCR_ScenarioFrameworkArea selectedArea : m_aSelectedAreas)
				{
					for (int j = m_aLayerTasksForRandomization.Count() - 1; j >= 0; j--)
					{
						if (m_aLayerTasksForRandomization[j].GetParentArea() == selectedArea)
						{
							m_aLayerTasksForRandomization.Remove(j);
							continue;
						}
					}
				}
			}

			SpawnRandomTask(m_aLayerTasksToBeInitialized.Count());
		}

		if (m_aLayerTasksToBeInitialized.Count() < m_iMaxNumberOfTasks)
			Print(string.Format("ScenarioFramework: Available areas do not have any other tasks to generate. Only %1 out of %2 was generated", m_aLayerTasksToBeInitialized, m_iMaxNumberOfTasks), LogLevel.NORMAL);

		AfterLayerTasksInit();
		Print("ScenarioFramework: ---------------------- Generation of tasks completed -------------------", LogLevel.NORMAL);
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//! Create a new global variable at the scenario
	void CreateVariableValue(string key, string value)
	{
		Rpc(RpcDo_CreateVariableValue, key, value, -2);
		RpcDo_CreateVariableValue(key, value, -2);
	}

	//------------------------------------------------------------------------------------------------
	//! Create a new global variable at the scenario
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_CreateVariableValue(string key, string value, int playerID)
	{
		// -2 is used for runtime creations. JIP uses actual playerID
		if (playerID > -2 && SCR_PlayerController.GetLocalPlayerId() != playerID)
			return;

		string notUsed;
		if (!GetVariable(key, notUsed))
			m_mVariableMap.Insert(key, value);
	}

	//------------------------------------------------------------------------------------------------
	//! Set a value to global variable at the scenario
	void SetVariableValue(string key, string value)
	{
		Rpc(RpcDo_SetVariableValue, key, value);
		RpcDo_SetVariableValue(key, value);
	}

	//------------------------------------------------------------------------------------------------
	//! Set a value to global variable at the scenario
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_SetVariableValue(string key, string value)
	{
		string notUsed;
		if (GetVariable(key, notUsed))
			m_mVariableMap.Set(key, value);
		else
			RpcDo_CreateVariableValue(key, value, -2);
	}

	//------------------------------------------------------------------------------------------------
	//! Get value of given variable
	bool GetVariable(string key, out string value)
	{
		if (key.IsEmpty())
		{
			Print(string.Format("Variable %1 is not set in this scenario", key), LogLevel.NORMAL);
			return false;
		}

		return m_mVariableMap.Find(key, value);
	}

	//------------------------------------------------------------------------------------------------
	//! Processes voice line enum and string, assigns index if match found.
	//! \param[in] targetEnum TargetEnum is an enum type representing combat operations in the game, used for indexing voice lines based on the given string.
	//! \param[in] targetString Represents a string value associated with an enum in the SCR_ECombatOps_Everon_Tasks enum
	void ProcessVoicelineEnumAndString(typename targetEnum, string targetString)
	{
		targetEnum = SCR_ECombatOps_Everon_Tasks;
		array<string> stringValues = {};
		SCR_Enum.GetEnumNames(targetEnum, stringValues);

		int index = stringValues.Find(targetString);
		if (index != -1)
			m_aIntroVoicelineIndexes.Insert(index)
	}

	//------------------------------------------------------------------------------------------------
	//! Executes AfterTasksInitActions after all Layer Tasks are finished spawning
	void AfterLayerTasksInit()
	{
		foreach (SCR_ScenarioFrameworkLayerTask layerTask : m_aLayerTasksToBeInitialized)
		{
			if (!layerTask)
				continue;

			SCR_ScenarioFrameworkArea parentArea = layerTask.GetParentArea();
			if (!parentArea)
				continue;

			if (m_bDynamicDespawn && (layerTask.m_bDynamicDespawn || parentArea.m_bDynamicDespawn) && !layerTask.GetDynamicDespawnExcluded())
				layerTask.DynamicDespawn(null);
		}

		foreach (SCR_ScenarioFrameworkActionBase afterTasksInitActions : m_aAfterTasksInitActions)
		{
			afterTasksInitActions.Init(m_aAreas.GetRandomElement().GetOwner());
		}
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//! Generates an area with a layer and slot tasks, if valid indices provided.
	//! \param[in] index Index represents the specific task configuration for slot, layer, and area in the scenario.
	void GenerateSingleTask(int index)
	{
		if (!m_aAreasTasksToSpawn.IsIndexValid(index) || !m_aLayersTaskToSpawn.IsIndexValid(index))
			return;

		string targetLayer = m_aLayersTaskToSpawn[index];
		string targetArea = m_aAreasTasksToSpawn[index];

		IEntity layerEntity = GetGame().GetWorld().FindEntityByName(targetLayer);
		if (!layerEntity)
			return;

		SCR_ScenarioFrameworkLayerTask taskComponent = SCR_ScenarioFrameworkLayerTask.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (!taskComponent)
			return;

		m_aLayerTasksToBeInitialized.Insert(taskComponent);

		IEntity areaEntity = GetGame().GetWorld().FindEntityByName(targetArea);
		if (!areaEntity)
			areaEntity = layerEntity.GetParent();

		if (!areaEntity)
			return;

		SCR_ScenarioFrameworkArea area = SCR_ScenarioFrameworkArea.Cast(areaEntity.FindComponent(SCR_ScenarioFrameworkArea));
		if (!area)
			return;

		string targetSlot;
		if (m_aSlotsTaskToSpawn.IsIndexValid(index))
			targetSlot = m_aSlotsTaskToSpawn[index];

		SCR_ScenarioFrameworkSlotTask slotComponent;
		if (targetSlot != "0" && !SCR_StringHelper.IsEmptyOrWhiteSpace(targetSlot))
		{
			IEntity slotEntity = GetGame().GetWorld().FindEntityByName(targetSlot);
			if (slotEntity)
				slotComponent = SCR_ScenarioFrameworkSlotTask.Cast(slotEntity.FindComponent(SCR_ScenarioFrameworkSlotTask));
		}

		area.SetAreaSelected(true);
		taskComponent.SetActivationType(SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT);

		area.Create(taskComponent, slotComponent);
		Print(string.Format("ScenarioFramework: Creating area %1", area.GetOwner().GetName()), LogLevel.NORMAL);
		Print("ScenarioFramework: ---------------------------------------------------------------", LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	//! Selects random area with suitable task type from list of areas.
	//! \param[in] eTaskType Task type parameter represents the type of task to be assigned in the selected area.
	//! \return a randomly selected area from the list of areas suitable for the given task type, or null if no suitable area is found
	SCR_ScenarioFrameworkArea SelectRandomAreaByTaskType(SCR_ESFTaskType eTaskType)
	{
		if (m_aAreas.IsEmpty())
			return null;

		SCR_ScenarioFrameworkArea selectedArea;
		array<SCR_ScenarioFrameworkArea> aAreasCopy = {};
		aAreasCopy.Copy(m_aAreas);
		for (int i = 0, count = m_aAreas.Count(); i < count; i++)
		{
			selectedArea = aAreasCopy.GetRandomElement();
			if (!selectedArea.GetIsAreaSelected() && selectedArea.GetIsTaskSuitableForArea(eTaskType))
			{
				selectedArea.SetAreaSelected(true);
				return selectedArea;
			}
			else
			{
				aAreasCopy.RemoveItem(selectedArea);
			}
		}
		return null;
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//! Get parent area the object is nested into
	//! \param[in] child
	SCR_ScenarioFrameworkArea GetParentArea(IEntity child)
	{
		if (!child)
			return null;

		SCR_ScenarioFrameworkArea layer;
		IEntity entity = child.GetParent();
		while (entity)
		{
			layer = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (layer)
				return layer;

			entity = entity.GetParent();
		}

		return layer;
	}

	//------------------------------------------------------------------------------------------------
	//! Prepares dynamic spawn/despawn for specific area (Intended for runtime usage)
	//! \param[in] area
	//! \param[in] staySpawned
	void PrepareAreaSpecificDynamicDespawn(SCR_ScenarioFrameworkArea area, bool staySpawned = false, int despawnRange = 0)
	{
		area.SetDynamicDespawnEnabled(true);
		area.SetDynamicDespawnRange(despawnRange);

		//If this method is called with staySpawned = false, area will be added to m_aDespawnedAreas and gets despawned
		if (!staySpawned)
		{
			m_aDespawnedAreas.Insert(new Tuple3<SCR_ScenarioFrameworkArea, vector, int>(area, area.GetOwner().GetOrigin(), (despawnRange * despawnRange)));
			area.DynamicDespawn(area);
		}
		else
		{
			m_aSpawnedAreas.Insert(new Tuple3<SCR_ScenarioFrameworkArea, vector, int>(area, area.GetOwner().GetOrigin(), (despawnRange * despawnRange)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Removes dynamic spawn/despawn for specific area (Intended for runtime usage)
	//! \param[in] area
	//! \param[in] staySpawned
	void RemoveAreaSpecificDynamicDespawn(SCR_ScenarioFrameworkArea area, bool staySpawned = false)
	{
		area.SetDynamicDespawnEnabled(false);

		//If this method is called with staySpawned = false, area will be despawned
		if (!staySpawned)
			area.DynamicDespawn(area);

		for (int i = m_aDespawnedAreas.Count() - 1; i >= 0; i--)
		{
			Tuple3<SCR_ScenarioFrameworkArea, vector, int> areaInfo = m_aDespawnedAreas[i];
			if (area == areaInfo.param1)
				m_aDespawnedAreas.Remove(i);
		}

		for (int i = m_aSpawnedAreas.Count() - 1; i >= 0; i--)
		{
			Tuple3<SCR_ScenarioFrameworkArea, vector, int> areaInfo = m_aSpawnedAreas[i];
			if (area == areaInfo.param1)
				m_aSpawnedAreas.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Prepares dynamic spawn/despawn
	void PrepareDynamicDespawn()
	{
		if (!m_bDynamicDespawn)
			return;

		GetOnAllAreasInitiated().Remove(PrepareDynamicDespawn);

		foreach (SCR_ScenarioFrameworkArea area : m_aAreas)
		{
			if (!area.GetDynamicDespawnEnabled())
				continue;

			int despawnRange = area.GetDynamicDespawnRange();
			m_aDespawnedAreas.Insert(new Tuple3<SCR_ScenarioFrameworkArea, vector, int>(area, area.GetOwner().GetOrigin(), (despawnRange * despawnRange)));

			area.DynamicDespawn(null);
		}

		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(CheckDistance, 1000 * m_iUpdateRate, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Refreshes array of player characters and checks which areas should spawn/despawn
	void CheckDistance()
	{
		m_aObservers.Clear();
		array<int> playerIds = {};
		PlayerManager playerManager = GetGame().GetPlayerManager();
		IEntity player;
		SCR_DamageManagerComponent damageManager;
		playerManager.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			player = playerManager.GetPlayerControlledEntity(playerId);
			if (!player)
				continue;

			damageManager = SCR_DamageManagerComponent.GetDamageManager(player);
			if (damageManager && damageManager.GetState() != EDamageState.DESTROYED)
				m_aObservers.Insert(player.GetOrigin());
		}

		DynamicSpawn();
		DynamicDespawn();
	}

	//------------------------------------------------------------------------------------------------
	//! Goes over despawned areas and checks whether or not said area should spawn
	void DynamicSpawn()
	{
		Tuple3<SCR_ScenarioFrameworkArea, vector, int> areaInfo;
		for (int i = m_aDespawnedAreas.Count() - 1; i >= 0; i--)
		{
			areaInfo = m_aDespawnedAreas[i];
			if (!areaInfo.param1)
				continue;

			foreach (vector observerPos : m_aObservers)
			{
				if (vector.DistanceSqXZ(observerPos, areaInfo.param2) < areaInfo.param3)
				{
					areaInfo.param1.DynamicReinit();
					m_aSpawnedAreas.Insert(areaInfo);
					m_aDespawnedAreas.Remove(i);
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Goes over spawned areas and checks whether or not said area should despawn
	void DynamicDespawn()
	{
		Tuple3<SCR_ScenarioFrameworkArea, vector, int> areaInfo;
		for (int i = m_aSpawnedAreas.Count() - 1; i >= 0; i--)
		{
			areaInfo = m_aSpawnedAreas[i];
			if (!areaInfo.param1)
				continue;

			bool observerInRange;
			foreach (vector observerPos : m_aObservers)
			{
				if (vector.DistanceSqXZ(observerPos, areaInfo.param2) < areaInfo.param3)
				{
					observerInRange = true;
					break;
				}
			}

			if (!observerInRange)
			{
				areaInfo.param1.DynamicDespawn(null);
				m_aDespawnedAreas.Insert(areaInfo);
				m_aSpawnedAreas.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Displays hint with title, subtitle, timeout, faction key, and player ID, then calls RpcDo
	//! \param[in] sTitle string for hint message displayed on screen.
	//! \param[in] sSubtitle is the secondary message displayed in hint.
	//! \param[in] timeOut represents the duration in milliseconds for which hint is displayed on screen.
	//! \param[in] factionKey represents the identifier for the faction in the game world.
	//! \param[in] playerID represents the unique identifier for the player receiving the hint.
	void ShowHint(string sTitle, string sSubtitle, int timeOut, FactionKey factionKey = "", int playerID = -1)
	{
		Rpc(RpcDo_ShowHint, sTitle, sSubtitle, timeOut, factionKey, playerID);
		RpcDo_ShowHint(sTitle, sSubtitle, timeOut, factionKey, playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Displays hint with title, subtitle, timeout, and fact/playerID checks.
	//! \param[in] sTitle is the title of the hint message displayed to players.
	//! \param[in] sSubtitle is the secondary message displayed in hint UI, it provides additional information related to the main title.
	//! \param[in] timeOut represents the duration in seconds for which hint is displayed on screen.
	//! \param[in] factionKey represents the identifier for the faction in the game, used to check if the local player is part of it
	//! \param[in] playerID represents the unique identifier for a player in the game.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_ShowHint(string sTitle, string sSubtitle, int timeOut, FactionKey factionKey, int playerID)
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(factionKey))
		{
			if (SCR_FactionManager.SGetLocalPlayerFaction() != GetGame().GetFactionManager().GetFactionByKey(factionKey))
				return;
		}

		if (playerID != -1)
		{
			PlayerController playerController = GetGame().GetPlayerController();
			if (!playerController)
				return;

			if (playerID != playerController.GetPlayerId())
				return;
		}

		SCR_HintUIInfo info = SCR_HintUIInfo.CreateInfo(sTitle, sSubtitle, timeOut, 0, 0, true);
		if (info)
			SCR_HintManagerComponent.ShowHint(info);
	}

	//------------------------------------------------------------------------------------------------
	//! Hide current hint
	//! \param[in] factionKey represents the identifier for the faction in the game world.
	//! \param[in] playerID represents the unique identifier for the player receiving the hint.
	void HideHint(FactionKey factionKey = "", int playerID = -1)
	{
		Rpc(RpcDo_HideHint, factionKey, playerID);
		RpcDo_HideHint(factionKey, playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Hide current hint.
	//! \param[in] factionKey represents the identifier for the faction in the game, used to check if the local player is part of it
	//! \param[in] playerID represents the unique identifier for a player in the game.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_HideHint(FactionKey factionKey, int playerID)
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(factionKey))
		{
			if (SCR_FactionManager.SGetLocalPlayerFaction() != GetGame().GetFactionManager().GetFactionByKey(factionKey))
				return;
		}

		if (playerID != -1)
		{
			PlayerController playerController = GetGame().GetPlayerController();
			if (!playerController)
				return;

			if (playerID != playerController.GetPlayerId())
				return;
		}

		SCR_HintManagerComponent.HideHint();
	}

	//------------------------------------------------------------------------------------------------
	//! Displays a pop-up message with title, subtitle, optional faction key, and optional player ID.
	//! \param[in] sTitle for pop-up message displayed on screen.
	//! \param[in] sSubtitle is the secondary message displayed in the pop-up message box.
	//! \param[in] factionKey represents the identifier for the faction in the game, used to specify which faction's message is being
	//! \param[in] playerID represents the unique identifier for the player receiving the pop-up message.
	void PopUpMessage(string sTitle, string sSubtitle, FactionKey factionKey = "", int playerID = -1, string titleParam1 = "", string titleParam2 = "", string subtitleParam1 = "", string subtitleParam2 = "")
	{
		Rpc(RpcDo_PopUpMessage, sTitle, sSubtitle, factionKey, playerID, titleParam1, titleParam2, subtitleParam1, subtitleParam2);
		RpcDo_PopUpMessage(sTitle, sSubtitle, factionKey, playerID, titleParam1, titleParam2, subtitleParam1, subtitleParam2);
	}

	//------------------------------------------------------------------------------------------------
	//! Displays pop-up message with title, subtitle, and checks if player is in specified faction or not.
	//! \param[in] sTitle for pop-up message displayed on screen.
	//! \param[in] sSubtitle is the message content displayed in the pop-up notification.
	//! \param[in] factionKey represents the identifier for a faction in the game, used to check if the local player is part of it
	//! \param[in] playerID represents the unique identifier for a player in the game.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_PopUpMessage(string sTitle, string sSubtitle, FactionKey factionKey, int playerID, string titleParam1, string titleParam2, string subtitleParam1, string subtitleParam2)
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(factionKey))
		{
			const FactionManager fm = GetGame().GetFactionManager();
			if (!fm || SCR_FactionManager.SGetLocalPlayerFaction() != fm.GetFactionByKey(factionKey))
				return;
		}

		if (playerID != -1)
		{
			PlayerController playerController = GetGame().GetPlayerController();
			if (!playerController)
				return;

			if (playerID != playerController.GetPlayerId())
				return;
		}

		SCR_PopUpNotification.GetInstance().PopupMsg(sTitle, text2: sSubtitle, param1: titleParam1, param2: titleParam2, text2param1: subtitleParam1, text2param2: subtitleParam2);
	}

	//------------------------------------------------------------------------------------------------
	//! Manages layer debug shape visibility and radius, updates event mask if draw is true, stores layer IDs for runtime in session
	//! \param[in] id Entity ID represents the unique identifier for an entity in the game world, used to find and manipulate specific objects in the simulation
	//! \param[in] draw represents whether to display debug shape for the layer during runtime.
	//! \param[in] radius represents the size of debug shape drawn around the layer entity in the game world.
	//! \param[in] runtime determines if debug shapes should be drawn during gameplay or not.
	void ManageLayerDebugShape(EntityID id, bool draw, float radius, bool runtime)
	{
		IEntity entity = GetGame().GetWorld().FindEntityByID(id);
		if (!entity)
			return;

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return;

		layer.m_bShowDebugShapesDuringRuntime = draw;
		layer.m_fDebugShapeRadius = radius;

		if (draw)
			layer.SetEventMask(layer.GetOwner(), EntityEvent.INIT | EntityEvent.FRAME);

		if (RplSession.Mode() != RplMode.Client)
			m_aDebugShapesLayers.Insert(new Tuple3<EntityID, bool, float>(id, draw, radius));

		/*if (runtime)
			Rpc(RpcDo_ManageLayerDebugShape, id, draw, radius); */
	}

	//------------------------------------------------------------------------------------------------
	//! Manages layer debug shape visibility and radius for specified player.
	//! \param[in] playerID Player ID represents the unique identifier for the player controlling the action in the method.
	//! \param[in] id Represents entity ID for scenario layer in the game world.
	//! \param[in] draw parameter represents whether to enable or disable debug shapes for the specified layer during runtime.
	//! \param[in] radius represents the size of debug shape drawn around the layer during runtime.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_ManageLayerDebugShape(int playerID, EntityID id, bool draw, float radius)
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		if (SCR_PlayerController.GetLocalPlayerId() != playerID)
			return;

		IEntity entity = GetGame().GetWorld().FindEntityByID(id);
		if (!entity)
			return;

		SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layer)
			return;

		layer.m_bShowDebugShapesDuringRuntime = draw;
		layer.m_fDebugShapeRadius = radius;

		if (draw)
			layer.SetEventMask(layer.GetOwner(), EntityEvent.INIT | EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	//! Displays a pop-up message with title, subtitle, optional faction key, and optional player ID.
	//! \param[in] playerId Player ID represents the unique identifier for the player.
	//! \param[in] actionOwnerEntId entity ID identifying entity that holds the action.
	//! \param[in] int layoutId layout ID identifying a specific UI layout
	//! \param[in] float fadeIn fade in time.
	//! \param[in] float fadeOut fade out time.
	//! \param[in] float time for how long layout will be visible.
	//! \param[in] float opacity of the layout.

	void ShowLayout(int playerId = -1, EntityID actionOwnerEntId = EntityID.INVALID, int layoutId = -1, float fadeIn = -1, float fadeOut = -1, float visibilityTime = -1, float opacity = -1)
	{
		Rpc(RpcDo_ShowLayout, playerId, actionOwnerEntId, layoutId, fadeIn, fadeOut, visibilityTime, opacity);
		RpcDo_ShowLayout(playerId, actionOwnerEntId, layoutId, fadeIn, fadeOut, visibilityTime, opacity);
	}

	//------------------------------------------------------------------------------------------------
	//! Displays a pop-up message with title, subtitle, optional faction key, and optional player ID.
	//! \param[in] playerId Player ID represents the unique identifier for the player.
	//! \param[in] actionOwnerEntId entity ID identifying entity that holds the action.
	//! \param[in] int layoutId layout ID identifying a specific UI layout
	//! \param[in] float fadeIn fade in time.
	//! \param[in] float fadeOut fade out time.
	//! \param[in] float time for how long layout will be visible.
	//! \param[in] float opacity of the layout.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_ShowLayout(int playerID, EntityID actionOwnerEntId, int layoutId, float fadeIn, float fadeOut, float visibilityTime, float opacity)
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		if (playerID == -1)
			return;

		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		if (playerID != playerController.GetPlayerId())
			return;

		IEntity actionOwnerEntity = GetGame().GetWorld().FindEntityByID(actionOwnerEntId);
		if (!actionOwnerEntity)
			return;

		SCR_ScenarioFrameworkLayerBase layerBase = SCR_ScenarioFrameworkLayerBase.Cast(actionOwnerEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!layerBase)
			return;

		SCR_ScenarioFrameworkActionShowLayout actionShowLayout = GetLayoutComponent(layerBase, layoutId);
		if (!actionShowLayout)
		{
			PrintFormat("ScenarioFramework Action: Action ShowLayout with ID %1 wasn't found at layer %2.", layoutId, layerBase, LogLevel.ERROR);
			return;
		}

		SCR_HUDManagerComponent hudManager = GetGame().GetHUDManager();
		Widget rootWidget = hudManager.CreateLayout(actionShowLayout.GetLayoutResource(), EHudLayers.OVERLAY);
		if (!rootWidget)
			return;

		Widget imgWidget = rootWidget.FindAnyWidget("Image");
		if (!imgWidget)
			imgWidget = ImageWidget.Cast(rootWidget);

		if (!imgWidget)
			return;

		imgWidget.SetOpacity(0);

		float animVal;
		if (fadeIn == 0)
			animVal = 1000;
		else
			animVal = 1 / fadeIn;

		AnimateWidget.Opacity(imgWidget, opacity, animVal);
		SCR_ScenarioFrameworkCallQueueSystem.GetCallQueueNonPausable().CallLater(FadeOut, (fadeIn + visibilityTime) * 1000, false, imgWidget, fadeOut);
	}

	//------------------------------------------------------------------------------------------------
	void FadeOut(Widget imgWidget, float fadeOut)
	{
		float animVal;
		if (fadeOut == 0)
			animVal = 1000;
		else
			animVal = 1 / fadeOut;

		AnimateWidget.Opacity(imgWidget, 0, animVal);
	}

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkActionShowLayout GetLayoutComponent(SCR_ScenarioFrameworkLayerBase layerBase, int layoutId)
	{
		SCR_ScenarioFrameworkActionShowLayout actionShowLayout;
		array<ref SCR_ScenarioFrameworkActionBase> activationActions = layerBase.GetActivationActions();

		foreach (SCR_ScenarioFrameworkActionBase action : activationActions)
		{
			actionShowLayout = SCR_ScenarioFrameworkActionShowLayout.Cast(action);
			if (actionShowLayout && actionShowLayout.m_iID == layoutId)
				return actionShowLayout;

			actionShowLayout = CheckSubActions(action, layoutId);
			if (actionShowLayout)
				return actionShowLayout;
		}

		array<ref SCR_ScenarioFrameworkPlugin> plugins = layerBase.GetSpawnedPlugins();
		foreach (SCR_ScenarioFrameworkPlugin plugin : plugins)
		{
			array<ref SCR_ScenarioFrameworkActionBase> pluginActions = plugin.GetActions();
			foreach (SCR_ScenarioFrameworkActionBase pluginAction : pluginActions)
			{
				actionShowLayout = SCR_ScenarioFrameworkActionShowLayout.Cast(pluginAction);
				if (actionShowLayout && actionShowLayout.m_iID == layoutId)
					return actionShowLayout;

				actionShowLayout = CheckSubActions(pluginAction, layoutId);
				if (actionShowLayout)
					return actionShowLayout;
			}
		}

		return actionShowLayout;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkActionShowLayout CheckSubActions(SCR_ScenarioFrameworkActionBase action, int layoutId)
	{
		SCR_ScenarioFrameworkActionShowLayout actionShowLayout;
		array<ref SCR_ScenarioFrameworkActionBase> subAtions = action.GetSubActions();
		if (!subAtions)
			return null;

		foreach (SCR_ScenarioFrameworkActionBase subAction : subAtions)
		{
			actionShowLayout = SCR_ScenarioFrameworkActionShowLayout.Cast(subAction);
			if (actionShowLayout && actionShowLayout.m_iID == layoutId)
				return actionShowLayout;

			actionShowLayout = CheckSubActions(subAction, layoutId);
		}

		return actionShowLayout;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsSaveGameLoading()
	{
		auto instance = SCR_ScenarioFrameworkSystem.GetInstance();
		return instance && instance.m_bIsSaveGameLoad;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsWorldLoadInit()
	{
		auto instance = SCR_ScenarioFrameworkSystem.GetInstance();
		return instance && instance.m_bIsWorldLoadInit;
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		float timeSlice = args.GetTimeSliceSeconds();

		m_fTimer += timeSlice;

		if (m_fTimer < m_fCheckInterval)
			return;

		m_fTimer = 0;

		s_CallQueuePausable.Tick(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] timeSlice interval for diagnostics update.
	override event protected void OnDiag(float timeSlice)
	{
		super.OnDiag(timeslice);

		SCR_ScenarioFrameworkDebug.OnDiag();
	}

	//------------------------------------------------------------------------------------------------
	//! Clears CallQueue after destruction of this system
	void ~SCR_ScenarioFrameworkSystem()
	{
		if (s_CallQueuePausable)
			s_CallQueuePausable.Clear();
	}
}
