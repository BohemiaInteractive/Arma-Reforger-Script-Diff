[EntityEditorProps(category: "GameScripted/ScenarioFramework/Layer", description: "")]
class SCR_ScenarioFrameworkLayerBaseClass : ScriptComponentClass
{
}

// SCR_ScenarioFrameworkLayerBase Invoker
void ScriptInvokerScenarioFrameworkLayerMethod(SCR_ScenarioFrameworkLayerBase layer);
typedef func ScriptInvokerScenarioFrameworkLayerMethod;
typedef ScriptInvokerBase<ScriptInvokerScenarioFrameworkLayerMethod> ScriptInvokerScenarioFrameworkLayer;

enum SCR_EScenarioFrameworkSpawnChildrenType
{
	ALL,
	RANDOM_ONE,
	RANDOM_MULTIPLE,
	RANDOM_BASED_ON_PLAYERS_COUNT
}

class SCR_ScenarioFrameworkLayerBase : ScriptComponent
{
	[Attribute(defvalue: "0", UIWidgets.ComboBox, desc: "Spawn all children, only random one or random multiple ones?", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkSpawnChildrenType), category: "Children")]
	SCR_EScenarioFrameworkSpawnChildrenType	m_SpawnChildren;

	[Attribute(desc: "Faction key that corresponds with the SCR_Faction set in FactionManager", category: "Asset")]
	FactionKey m_sFactionKey;

	[Attribute(defvalue: "100", desc: "If the RANDOM_MULTIPLE option is selected, what's the percentage? ", UIWidgets.Graph, "0 100 1", category: "Children")]
	int m_iRandomPercent;

	[Attribute(desc: "When enabled, it will repeatedly spawn childern according to other parameters set", category: "Children")]
	bool m_bEnableRepeatedSpawn;

	[Attribute(defvalue: "-1", desc: "If Repeated Spawn is enabled, how many times can children be spawned? If set to -1, it is unlimited", params: "-1 inf 1", category: "Children")]
	int m_iRepeatedSpawnNumber;

	[Attribute(defvalue: "-1", UIWidgets.Slider, desc: "If Repeated Spawn is enabled, how frequently it will spawn next wave of children? Value -1 means disabled, thus children won't be spawned by the elapsed time.", params: "-1 86400 1", category: "Children")]
	float m_fRepeatedSpawnTimer;

	[Attribute(desc: "Show the debug shapes during runtime", category: "Debug")]
	bool m_bShowDebugShapesDuringRuntime;
	
	[Attribute(desc: "Show the debug shapes in Workbench", category: "Debug")]
	bool m_bShowDebugShapesInWorkbench;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(SCR_ScenarioFrameworkEActivationType), category: "Activation")]
	SCR_ScenarioFrameworkEActivationType m_eActivationType;

	[Attribute(desc: "Conditions that will be checked upon init and based on the result it will let this to finish init or not", category: "Activation")]
	ref array<ref SCR_ScenarioFrameworkActivationConditionBase> m_aActivationConditions;

	[Attribute(defvalue: SCR_EScenarioFrameworkLogicOperators.AND.ToString(), UIWidgets.ComboBox, "Which Boolean Logic will be used for Activation Conditions", "", enums: SCR_EScenarioFrameworkLogicOperatorHelper.GetParamInfo(), category: "Activation")]
	SCR_EScenarioFrameworkLogicOperators m_eActivationConditionLogic;

	[Attribute(desc: "Actions that will be activated on initalization.", category: "OnInit")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActivationActions;

	[Attribute(desc: "Should the dynamic Spawn/Despawn based on distance from observer cameras be enabled?", category: "Activation")]
	bool m_bDynamicDespawn;

	[Attribute(defvalue: "750", params: "0 inf", desc: "How close at least one observer camera must be in order to trigger spawn", category: "Activation")]
	int m_iDynamicDespawnRange;	
	
	[Attribute(desc: "", category: "Activation")]
	bool m_bExcludeFromDynamicDespawn;

	[Attribute(UIWidgets.Auto, category: "Plugins")]
	ref array<ref SCR_ScenarioFrameworkPlugin> m_aPlugins;

	ref array<SCR_ScenarioFrameworkLayerBase> m_aChildren = {};
	ref array<SCR_ScenarioFrameworkLayerBase> m_aRandomlySpawnedChildren = {};
	ref array<SCR_ScenarioFrameworkLogic> m_aLogic = {};

	ref ScriptInvokerBase<ScriptInvokerScenarioFrameworkLayerMethod> m_OnAllChildrenSpawned;
	ref array<IEntity> m_aSpawnedEntities = {};
	IEntity	m_Entity;
	SCR_ScenarioFrameworkArea m_Area;
	SCR_ScenarioFrameworkLayerBase m_ParentLayer;
	float m_fDebugShapeRadius = 0.25;
	WorldTimestamp m_fRepeatSpawnTimeStart;
	WorldTimestamp	m_fRepeatSpawnTimeEnd;
	int	m_iDebugShapeColor = ARGB(32, 255, 255, 255);
	int m_iCurrentlySpawnedChildren;
	int m_iSupposedSpawnedChildren;
	bool m_bInitiated;
	bool m_bDynamicallyDespawned;
	bool m_bIsTerminated; //Marks if this was terminated - either by death or deletion

	//Default values we need to store
	int m_iRepeatedSpawnNumberDefault = m_iRepeatedSpawnNumber;
	SCR_ScenarioFrameworkEActivationType m_eActivationTypeDefault = m_eActivationType;

	static const int SPAWN_DELAY = 200;

	//------------------------------------------------------------------------------------------------
	//! \return the name of the owner entity.
	string GetName()
	{
		return GetOwner().GetName();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity Sets the entity reference for further manipulation.
	void SetEntity(IEntity entity)
	{
		m_Entity = entity;
	}

	//------------------------------------------------------------------------------------------------
	//! \return whether the layer is terminated or not.
	bool GetIsTerminated()
	{
		return m_bIsTerminated;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] state termination of this layer.
	void SetIsTerminated(bool state)
	{
		m_bIsTerminated = state;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] randomlySpawnedChildren A list of child entities to randomly spawn
	void SetRandomlySpawnedChildren(array<string> randomlySpawnedChildren)
	{
		IEntity entity;
		SCR_ScenarioFrameworkLayerBase layer;
		m_aRandomlySpawnedChildren.Clear();
		foreach (string child : randomlySpawnedChildren)
		{
			entity = GetGame().GetWorld().FindEntityByName(child);
			if (!entity)
				continue;

			layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
				continue;

			m_aRandomlySpawnedChildren.Insert(layer);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Adds child to list if not already present, ensuring uniqueness.
	//! \param[in] child Adds child object to list of randomly spawned children if it's not already in the list.
	void AddRandomlySpawnedChild(SCR_ScenarioFrameworkLayerBase child)
	{
		if (child && !m_aRandomlySpawnedChildren.Contains(child))
			m_aRandomlySpawnedChildren.Insert(child);
	}

	//------------------------------------------------------------------------------------------------
	//! \return Randomly spawned children layers
	array<SCR_ScenarioFrameworkLayerBase> GetRandomlySpawnedChildren()
	{
		return m_aRandomlySpawnedChildren;
	}

	//------------------------------------------------------------------------------------------------
	//! Counts players in specified faction.
	//! \param[in] factionName FactionName is the key identifier for a faction in the game, used to determine player count within that faction.
	//! \return count of players in specified faction.
	int GetPlayersCount(FactionKey factionName = "")
	{
		if (factionName.IsEmpty())
			return GetGame().GetPlayerManager().GetPlayerCount();

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return -1;

		int iCnt = 0;
		array<int> aPlayerIDs = {};
		SCR_PlayerController playerController;
		GetGame().GetPlayerManager().GetPlayers(aPlayerIDs);
		foreach (int iPlayerID : aPlayerIDs)
		{
			playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(iPlayerID));
			if (!playerController)
				continue;

			if (playerController.GetLocalControlledEntityFaction() == factionManager.GetFactionByKey(factionName))
				iCnt++;
		}
		return iCnt;
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	//! Determines maximum players for a mission based on its mode, returns 4 if mission header is invalid.
	//! \param[in] factionName Faction name is the identifier for the faction in the game mode, used to determine the maximum number of players allowed in
	//! \return Maximum players allowed for the current game mode based on the specified faction.
	int GetMaxPlayersForGameMode(FactionKey factionName = "")
	{
		//TODO: separate players by faction (attackers / defenders)
		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());

		if (!header)
			return 4;	//TODO: make a constant

		return header.m_iPlayerCount;
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//! \return parent area of the entity or null if not found.
	SCR_ScenarioFrameworkArea GetParentArea()
	{
		if (m_Area)
			return m_Area;

		IEntity entity = GetOwner().GetParent();
		while (entity)
		{
			m_Area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (m_Area)
				return m_Area;

			entity = entity.GetParent();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Gathers all layers from child entities and their siblings, adding them to the provided array.
	//! \param[out] layers Represents all layers in this layer hierarchy.
	void GetAllLayers(out notnull array<SCR_ScenarioFrameworkLayerBase> layers)
	{
		layers.Clear();
		array<SCR_ScenarioFrameworkLayerBase> aSiblingLayers = {};
		SCR_ScenarioFrameworkLayerBase layerBase;
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			layerBase = SCR_ScenarioFrameworkLayerBase.Cast(child.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layerBase)
			{
				layers.Insert(layerBase);

				layerBase.GetAllLayers(aSiblingLayers);
				layers.InsertAll(aSiblingLayers);
			}

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Gathers all layers from child entities and their siblings, adding them to the provided array, filtered by activation type
	//! \param[out] layers Represents all layers in this layer hierarchy.
	//! \param[in] activationType that will filter output layers.
	void GetAllLayers(out notnull array<SCR_ScenarioFrameworkLayerBase> layers, SCR_ScenarioFrameworkEActivationType activationType)
	{
		layers.Clear();
		array<SCR_ScenarioFrameworkLayerBase> aSiblingLayers = {};
		SCR_ScenarioFrameworkLayerBase layerBase;
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			layerBase = SCR_ScenarioFrameworkLayerBase.Cast(child.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layerBase)
			{
				if (layerBase.m_eActivationType == activationType)
					layers.Insert(layerBase);

				layerBase.GetAllLayers(aSiblingLayers, activationType);
				layers.InsertAll(aSiblingLayers);
			}

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Gathers all layer tasks from child entities and their siblings, adding them to the provided array.
	//! \param[out] layerTasks Represents all tasks associated with layers in the scenario hierarchy.
	void GetAllLayerTasks(out notnull array<SCR_ScenarioFrameworkLayerTask> layerTasks)
	{
		layerTasks.Clear();
		array<SCR_ScenarioFrameworkLayerTask> aSiblingLayerTasks = {};
		SCR_ScenarioFrameworkLayerTask layerTask;
		SCR_ScenarioFrameworkLayerBase layerBase;
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			layerBase = SCR_ScenarioFrameworkLayerBase.Cast(child.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layerBase)
			{
				layerTask = SCR_ScenarioFrameworkLayerTask.Cast(layerBase);
				if (layerTask)
					layerTasks.Insert(layerTask);

				layerBase.GetAllLayerTasks(aSiblingLayerTasks);
				layerTasks.InsertAll(aSiblingLayerTasks);
			}

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Gets all slot tasks from all layers in the hierarchy of the owner entity, including sibling layers' tasks.
	//! \param[out] slotTasks Represents all slot tasks from the current entity and its siblings in the hierarchy.
	void GetAllSlotTasks(out notnull array<SCR_ScenarioFrameworkSlotTask> slotTasks)
	{
		slotTasks.Clear();
		array<SCR_ScenarioFrameworkSlotTask> aSiblingSlotTasks = {};
		SCR_ScenarioFrameworkSlotTask slotTask;
		SCR_ScenarioFrameworkLayerBase layerBase;
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			layerBase = SCR_ScenarioFrameworkLayerBase.Cast(child.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layerBase)
			{
				slotTask = SCR_ScenarioFrameworkSlotTask.Cast(layerBase);
				if (slotTask)
					slotTasks.Insert(slotTask);

				layerBase.GetAllSlotTasks(aSiblingSlotTasks);
				slotTasks.InsertAll(aSiblingSlotTasks);
			}

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return the ScenarioFrameworkLayerTask that this layer is nested into
	SCR_ScenarioFrameworkLayerTask GetLayerTask()
	{
		SCR_ScenarioFrameworkLayerTask layer;
		IEntity entity = GetOwner().GetParent();
		while (entity)
		{
			layer = SCR_ScenarioFrameworkLayerTask.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerTask));
			if (layer)
				return layer;

			entity = entity.GetParent();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieves slot task from layer child entities.
	//! \param[in] aLayers is an array of ScenarioFrameworkLayerBase objects representing layers in the scenario.
	//! \return the first SCR_ScenarioFrameworkSlotTask component found in the given layers or their children.
	SCR_ScenarioFrameworkSlotTask GetSlotTask(array<SCR_ScenarioFrameworkLayerBase> aLayers)
	{
		SCR_ScenarioFrameworkSlotTask slotTask;
		foreach (SCR_ScenarioFrameworkLayerBase layer : aLayers)
		{
			IEntity child = layer.GetOwner();
			slotTask = SCR_ScenarioFrameworkSlotTask.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotTask));
			if (slotTask)
				return slotTask;

			child = GetOwner().GetChildren();
			while (child)
			{
				slotTask = SCR_ScenarioFrameworkSlotTask.Cast(child.FindComponent(SCR_ScenarioFrameworkSlotTask));
				if (slotTask)
					return slotTask;

				child = child.GetSibling();
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] factionKey Sets the faction key for the layer.
	protected void SetFactionKey(FactionKey factionKey)
	{
		m_sFactionKey = factionKey;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Faction key representing the faction this layer has assigned.
	protected FactionKey GetFactionKey()
	{
		return m_sFactionKey;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] layer base that will be set as parentLayer.
	void SetParentLayer(SCR_ScenarioFrameworkLayerBase parentLayer)
	{
		m_ParentLayer = parentLayer;
	}

	//------------------------------------------------------------------------------------------------
	//! Climbs the hierarchy tree to find a defined FactionKey.
	//! Only reccomended to use in Workbench for object preview. When the Game is running, directly reference m_sFactionKey or GetFactionKey().
	protected FactionKey GetParentFactionKeyRecursive()
	{
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(GetFactionKey()))
		{
			// Resolve Alias
			SCR_FactionAliasComponent factionAliasComponent = SCR_FactionAliasComponent.GetFactionAliasComponentForWB();
			if (!factionAliasComponent)
				return GetFactionKey();   // If the mission creator didn't define SCR_FactionAliasComponent, then alias resolution is not needed.

			return factionAliasComponent.ResolveFactionAlias(GetFactionKey());
		}

		IEntity parentEntity = GetOwner().GetParent();
		if (!parentEntity)
			return "";

		SCR_ScenarioFrameworkLayerBase parentLayer = SCR_ScenarioFrameworkLayerBase.Cast(parentEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!parentLayer)
			return "";

		FactionKey factionKey = parentLayer.GetParentFactionKeyRecursive();
		return factionKey;

	}

	//------------------------------------------------------------------------------------------------
	//! \return parent layer entity or null if no parent layer exists.
	SCR_ScenarioFrameworkLayerBase GetParentLayer()
	{
		if (m_ParentLayer)
			return m_ParentLayer;

		IEntity entity = GetOwner().GetParent();
		if (!entity)
			return null;

		m_ParentLayer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		return m_ParentLayer;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Spawn children type for scenario spawning.
	SCR_EScenarioFrameworkSpawnChildrenType GetSpawnChildrenType()
	{
		return m_SpawnChildren;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Represents whether repeated spawning is enabled or not.
	bool GetEnableRepeatedSpawn()
	{
		return m_bEnableRepeatedSpawn;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] value Enables or disables repeated spawning of units for this layer.
	void SetEnableRepeatedSpawn(bool value)
	{
		m_bEnableRepeatedSpawn = value;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Represents the activation type.
	SCR_ScenarioFrameworkEActivationType GetActivationType()
	{
		return m_eActivationType;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] activationType Sets the activation type
	void SetActivationType(SCR_ScenarioFrameworkEActivationType activationType)
	{
		m_eActivationType = activationType;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Indicates if the layer has been initiated.
	bool GetIsInitiated()
	{
		return m_bInitiated;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Indicates whether dynamic despawn is enabled or not.
	bool GetDynamicDespawnEnabled()
	{
		return m_bDynamicDespawn;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] enabled Enables or disables dynamic despawning.
	void SetDynamicDespawnEnabled(bool enabled)
	{
		m_bDynamicDespawn = enabled;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Represents whether the layer is excluded from dynamic despawning.
	bool GetDynamicDespawnExcluded()
	{
		return m_bExcludeFromDynamicDespawn;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Represents dynamic despawn range.
	int GetDynamicDespawnRange()
	{
		return m_iDynamicDespawnRange;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] range Represents distance in meters for dynamic despawning.
	void SetDynamicDespawnRange(int range)
	{
		m_iDynamicDespawnRange = range;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] excluded Excludes layer from dynamic despawning.
	void SetDynamicDespawnExcluded(bool excluded)
	{
		m_bExcludeFromDynamicDespawn = excluded;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Array of spawned entities.
	array<IEntity> GetSpawnedEntities()
	{
		return m_aSpawnedEntities;
	}

	//------------------------------------------------------------------------------------------------
	//! \return an array of child entities in the scenario framework layer.
	array<SCR_ScenarioFrameworkLayerBase> GetChildrenEntities()
	{
		return m_aChildren;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Randomly selected child object from the list of children.
	SCR_ScenarioFrameworkLayerBase GetRandomChildren()
	{
		if (m_aChildren.IsEmpty())
			return null;
		
		array<SCR_ScenarioFrameworkLayerBase> suitableChildren = {};
		foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
		{
			if (child.GetDynamicDespawnEnabled())
				continue;
			
			suitableChildren.Insert(child);
		}

		return suitableChildren.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! Reverses and inserts children layers into an array.
	//! \param[out] children Returns an array of child entities with SCR_ScenarioFrameworkLayerBase component, reversed and then inserted into the main
	void GetChildren(out array<SCR_ScenarioFrameworkLayerBase> children)
	{
		children = {};
		map<string, SCR_ScenarioFrameworkLayerBase> childrenReversed = new map<string, SCR_ScenarioFrameworkLayerBase>;
		array<string> childrenStringsReversed = {};
		SCR_ScenarioFrameworkLayerBase layerBase;
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			layerBase = SCR_ScenarioFrameworkLayerBase.Cast(child.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layerBase)
			{
				childrenReversed.Insert(layerBase.GetName(), layerBase);
				childrenStringsReversed.Insert(layerBase.GetName());
			}

			child = child.GetSibling();
		}

		childrenStringsReversed.Sort();
		foreach (string name : childrenStringsReversed)
		{
			layerBase = childrenReversed.Get(name);

			if (layerBase)
				children.Insert(layerBase);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Reverses and inserts children layers into an array, filtered by activation type
	//! \param[out] children Returns an array of child entities with SCR_ScenarioFrameworkLayerBase component, reversed and then inserted into the main
	void GetChildren(out array<SCR_ScenarioFrameworkLayerBase> children, SCR_ScenarioFrameworkEActivationType activationType)
	{
		children = {};
		map<string, SCR_ScenarioFrameworkLayerBase> childrenReversed = new map<string, SCR_ScenarioFrameworkLayerBase>;
		array<string> childrenStringsReversed = {};
		SCR_ScenarioFrameworkLayerBase layerBase;
		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			layerBase = SCR_ScenarioFrameworkLayerBase.Cast(child.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layerBase)
			{
				childrenReversed.Insert(layerBase.GetName(), layerBase);
				childrenStringsReversed.Insert(layerBase.GetName());
			}

			child = child.GetSibling();
		}

		childrenStringsReversed.Sort();
		foreach (string name : childrenStringsReversed)
		{
			layerBase = childrenReversed.Get(name);

			if (layerBase && layerBase.m_eActivationType == activationType)
				children.Insert(layerBase);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieves all child scenario framework logic entities from owner entity and adds them to logics array if not already present.
	//! \param[out] logics Retrieves all child scenario framework logic entities from owner entity and adds them to logics array if not already present.
	void GetLogics(out array<SCR_ScenarioFrameworkLogic> logics)
	{
		IEntity child = GetOwner().GetChildren();
		SCR_ScenarioFrameworkLogic logic;
		while (child)
		{
			logic = SCR_ScenarioFrameworkLogic.Cast(child);
			if (logic && !logics.Contains(logic))
				logics.Insert(logic);

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return Array of spawned logic.
	array<SCR_ScenarioFrameworkLogic> GetSpawnedLogics()
	{
		return m_aLogic;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Array of plugins.
	array<ref SCR_ScenarioFrameworkPlugin> GetSpawnedPlugins()
	{
		return m_aPlugins;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Repetition count for spawning an object.
	int GetRepeatedSpawnNumber()
	{
		return m_iRepeatedSpawnNumber;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] number Represents the number of repeated spawns for an entity in the game.
	void SetRepeatedSpawnNumber(int number)
	{
		m_iRepeatedSpawnNumber = number;
	}

	//------------------------------------------------------------------------------------------------
	//! Repetitive spawning with timer.
	protected void RepeatedSpawn()
	{
		if (!m_bEnableRepeatedSpawn || (m_iRepeatedSpawnNumber != -1 && m_iRepeatedSpawnNumber <= 0))
			return;

		//This calls the RepeatedSpawnCalled with set delay and is set in a way that it
		//Can be both queued that way or called manually from different place (pseudo-looped CallLater)
		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(RepeatedSpawnCalled, 1000 * m_fRepeatedSpawnTimer);
	}

	//------------------------------------------------------------------------------------------------
	//! Repetitive spawning logic with countdown and condition checks.
	protected void RepeatedSpawnCalled()
	{
		if (m_iRepeatedSpawnNumber != -1)
			m_iRepeatedSpawnNumber--;

		SpawnChildren(true);

		if (!m_bEnableRepeatedSpawn || (m_iRepeatedSpawnNumber != -1 && m_iRepeatedSpawnNumber <= 0))
			return;

		RepeatedSpawn();
	}

	//------------------------------------------------------------------------------------------------
	//! \return a ScriptInvokerBase object representing the invoker triggered when all children have spawned in ScenarioFrameworkLayer.
	ScriptInvokerScenarioFrameworkLayer GetOnAllChildrenSpawned()
	{
		if (!m_OnAllChildrenSpawned)
			m_OnAllChildrenSpawned = new ScriptInvokerBase<ScriptInvokerScenarioFrameworkLayerMethod>();

		return m_OnAllChildrenSpawned;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns all children and triggers invoker on completion.
	void InvokeAllChildrenSpawned()
	{
		if (m_OnAllChildrenSpawned)
			m_OnAllChildrenSpawned.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	//! Calculates supposed spawned children based on activation type, termination, initiation, and conditions.
	//! \param[in] previouslyRandomized indicates whether children layers were spawned randomly previously.
	void CalculateSupposedSpawnedChildren(bool previouslyRandomized = false)
	{
		m_iSupposedSpawnedChildren = 0;
		if (previouslyRandomized)
		{
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aRandomlySpawnedChildren)
			{
				if (child.GetDynamicDespawnEnabled())
					continue;
				
				int activationType = child.GetActivationType();
				if (activationType == SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION || activationType == SCR_ScenarioFrameworkEActivationType.ON_AREA_TRIGGER_ACTIVATION
					|| activationType == SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT)
					continue;

				if (child.GetIsTerminated())
					continue;

				if (!child.InitActivationConditions())
					continue;

				if (child.GetIsInitiated())
					m_iCurrentlySpawnedChildren++;

				m_iSupposedSpawnedChildren++;
			}
		}
		else
		{
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
			{
				if (child.GetDynamicDespawnEnabled())
					continue;
				
				int activationType = child.GetActivationType();
				if (activationType == SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION || activationType == SCR_ScenarioFrameworkEActivationType.ON_AREA_TRIGGER_ACTIVATION
					|| activationType == SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT)
					continue;

				if (child.GetIsTerminated())
					continue;

				if (!child.InitActivationConditions())
					continue;

				if (child.GetIsInitiated())
					m_iCurrentlySpawnedChildren++;

				m_iSupposedSpawnedChildren++;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if all children layers have spawned or spawns one randomly if specified.
	//! \param[in] layer for which this check is invoked.
	void CheckAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer = null)
	{
		if (!layer && layer.GetDynamicDespawnEnabled())
			return;
		
		if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.ALL)
		{
			m_iCurrentlySpawnedChildren = 0;
			CalculateSupposedSpawnedChildren();

			if (m_iCurrentlySpawnedChildren == m_iSupposedSpawnedChildren)
				InvokeAllChildrenSpawned();
		}
		else if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_ONE)
		{
			InvokeAllChildrenSpawned();
		}
		else
		{
			m_iCurrentlySpawnedChildren = 0;
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
			{
				if (child.GetDynamicDespawnEnabled())
					continue;
				
				int activationType = child.GetActivationType();
				if (activationType == SCR_ScenarioFrameworkEActivationType.ON_TRIGGER_ACTIVATION || activationType == SCR_ScenarioFrameworkEActivationType.ON_AREA_TRIGGER_ACTIVATION)
					continue;

				if (child.GetIsTerminated())
					continue;

				if (!child.InitActivationConditions())
					continue;

				if (child.GetIsInitiated())
					m_iCurrentlySpawnedChildren++;
			}

			if (m_iCurrentlySpawnedChildren == m_iSupposedSpawnedChildren)
				InvokeAllChildrenSpawned();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns children based on scenario settings, either all at once or randomly.
	//! \param[in] previouslyRandomized bool indicates whether children were spawned randomly before.
	void SpawnChildren(bool previouslyRandomized = false)
	{
		if (m_aChildren.IsEmpty())
		{
			InvokeAllChildrenSpawned();
			return;
		}

		if (!previouslyRandomized && !m_aRandomlySpawnedChildren.IsEmpty())
			previouslyRandomized = true;

		const bool isWorldLoadInit = SCR_ScenarioFrameworkSystem.IsWorldLoadInit();
		
		if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.ALL)
		{
			int slotCount;
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
			{
				if (child.GetDynamicDespawnEnabled())
				{
					if (m_aChildren.Count() == 1)
						InvokeAllChildrenSpawned();	
					
					continue;
				}
				
				if (SCR_ScenarioFrameworkSlotBase.Cast(child))
				{
					if (isWorldLoadInit || SCR_ScenarioFrameworkSlotMarker.Cast(child) || SCR_ScenarioFrameworkSlotWaypoint.Cast(child))
					{
						InitChild(child);	
					}
					else
					{
						SCR_ScenarioFrameworkCallQueueSystem.GetCallQueueNonPausable().CallLater(InitChild, SPAWN_DELAY * slotCount, false, child);
						slotCount++;
					}
				}
				else
				{
					InitChild(child);
				}
			}
		}
		else if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_ONE)
		{
			SpawnRandomOneChild(previouslyRandomized);
		}
		else
		{
			SpawnRandomMultipleChildren(previouslyRandomized);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns children with delay based on their index.
	void SpawnPreviouslyRandomizedChildren()
	{
		CalculateSupposedSpawnedChildren(true);
		
		const bool isWorldLoadInit = SCR_ScenarioFrameworkSystem.IsWorldLoadInit();
		foreach (int i, SCR_ScenarioFrameworkLayerBase child : m_aRandomlySpawnedChildren)
		{
			if (isWorldLoadInit)
			{
				InitChild(child);
				continue;
			}

			SCR_ScenarioFrameworkCallQueueSystem.GetCallQueueNonPausable().CallLater(InitChild, 200 * i, false, child);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns random child object if not previously randomized, else spawns previously randomized children.
	//! \param[in] previouslyRandomized indicates if children were spawned randomly before.
	void SpawnRandomOneChild(bool previouslyRandomized = false)
	{
		if (previouslyRandomized)
			SpawnPreviouslyRandomizedChildren();
		else
		{
			SCR_ScenarioFrameworkLayerBase child = GetRandomChildren();
			m_aRandomlySpawnedChildren.Insert(child);
			InitChild(child);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns random children based on player count, if previously randomized, uses previous children, otherwise selects from available children.
	//! \param[in] previouslyRandomized state is stored, used for spawning them again in the same positions.
	void SpawnRandomMultipleChildren(bool previouslyRandomized = false)
	{
		if (previouslyRandomized)
			SpawnPreviouslyRandomizedChildren();
		else
		{
			array<SCR_ScenarioFrameworkLayerBase> suitableChildren = {};
			foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
			{
				if (child.GetDynamicDespawnEnabled())
					continue;
				
				suitableChildren.Insert(child);
			}

			if (suitableChildren.IsEmpty())
				return;

			if (m_SpawnChildren == SCR_EScenarioFrameworkSpawnChildrenType.RANDOM_BASED_ON_PLAYERS_COUNT)
				m_iRandomPercent = Math.Ceil(GetPlayersCount() / GetMaxPlayersForGameMode() * 100);

			m_iSupposedSpawnedChildren = Math.Round(suitableChildren.Count() * 0.01 * m_iRandomPercent);
			SCR_ScenarioFrameworkLayerBase child;
			for (int i = 1; i <= m_iSupposedSpawnedChildren; i++)
			{
				child = suitableChildren.GetRandomElement();
				m_aRandomlySpawnedChildren.Insert(child);
				InitChild(child);
				suitableChildren.RemoveItem(child);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes child layer, sets parent layer, and initializes child with parent area and action type.
	//! \param[in] child that is to be initialized.
	void InitChild(SCR_ScenarioFrameworkLayerBase child)
	{
		if (!child)
			return;

		child.SetParentLayer(this);
		child.Init(GetParentArea(), SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
	}

	//------------------------------------------------------------------------------------------------
	//! \return Spawned entity.
	IEntity GetSpawnedEntity()
	{
		return m_Entity;
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes all logic components.
	protected void ActivateLogic()
	{
		GetLogics(m_aLogic);
		foreach (SCR_ScenarioFrameworkLogic logic : m_aLogic)
		{
			logic.Init();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Restores default settings, clears children, removes spawned entities, optionally reinitializes after restoration.
	//! \param[in] includeChildren Restores default settings for this entity and its children if includeChildren is true.
	//! \param[in] reinitAfterRestoration Restores entity to default state, optionally reinitializes after restoration.
	//! \param[in] affectRandomization determines whether to clear all randomly spawned children entities after restoring default settings.
	void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false, bool affectRandomization = true)
	{
		m_Entity = null;
		m_iRepeatedSpawnNumber = m_iRepeatedSpawnNumberDefault;
		m_eActivationType = m_eActivationTypeDefault;
		m_bInitiated = false;
		m_bDynamicallyDespawned = false;
		m_bIsTerminated = false;

		RestoreActionsToDefault();

		if (includeChildren)
		{
			if (m_aChildren.IsEmpty())
				GetChildren(m_aChildren);

			foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
			{
				child.RestoreToDefault(includeChildren, false);
			}
		}

		m_aChildren.Clear();
		if (affectRandomization)
			m_aRandomlySpawnedChildren.Clear();

		foreach (IEntity entity : m_aSpawnedEntities)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}

		m_aSpawnedEntities.Clear();

		foreach (SCR_ScenarioFrameworkLogic logic : m_aLogic)
		{
			logic.RestoreToDefault();
		}

		if (reinitAfterRestoration)
			Init(m_Area);
	}

	//------------------------------------------------------------------------------------------------
	//! Dynamically despawns this layer.
	//! \param[in] layer for which this is called.
	void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
		GetOnAllChildrenSpawned().Remove(DynamicDespawn);
		if (!m_bInitiated)
		{
			GetOnAllChildrenSpawned().Insert(DynamicDespawn);
			return;
		}

		if (m_bExcludeFromDynamicDespawn)
			return;

		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		m_aChildren.RemoveItem(null);
		foreach (SCR_ScenarioFrameworkLayerBase child : m_aChildren)
		{
			// Check if the child has its own Dynamic Despawn in place that is managed by the parent area and not the parent layer
			if (!child.m_bDynamicDespawn)
				child.DynamicDespawn(this);
		}

		m_aChildren.Clear();

		foreach (IEntity entity : m_aSpawnedEntities)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}

		m_aSpawnedEntities.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Reinitializes this layer.
	void DynamicReinit()
	{
		Init(GetParentArea(), SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
	}

	//------------------------------------------------------------------------------------------------
	//! Initialization check if already happened.
	//! \return whether initialization has already happened.
	bool InitAlreadyHappened()
	{
		return m_bInitiated;
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes parent layer.
	//! \return true if parent layer is already initialized, otherwise initializes it and returns true if successful.
	bool InitParentLayer()
	{
		if (m_ParentLayer)
			return true;

		m_ParentLayer = GetParentLayer();
		return (m_ParentLayer != null);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if layer is not terminated, spawns children if parent exists, returns false if terminated or children not spawnned
	//! \return true if not terminated, otherwise false.
	bool InitNotTerminated()
	{
		if (!m_bIsTerminated)
			return true;

		if (m_ParentLayer)
			m_ParentLayer.CheckAllChildrenSpawned(this);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if object is not dynamically despawned, then verifies if activation type matches, spawns all children if
	//! \param[in] activation Activation type determines if scenario object should spawn or despawn based on conditions.
	//! \return true if the object can be despawned and activated with given type, false otherwise.
	bool InitDynDespawnAndActivation(SCR_ScenarioFrameworkEActivationType activation)
	{
		if (!m_bDynamicallyDespawned && activation != m_eActivationType)
		{
			if (m_ParentLayer)
				m_ParentLayer.CheckAllChildrenSpawned(this);

			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks activation conditions for scenario layer, sets condition status based on logic operator, checks parent layer if needed.
	//! \return whether all activation conditions for this scenario layer have been initialized successfully.
	bool InitActivationConditions(bool calledFromInit = false)
	{
		if (m_aActivationConditions.IsEmpty())
			return true;

		IEntity owner = GetOwner();
		bool conditionStatus = SCR_ScenarioFrameworkActivationConditionBase.EvaluateEmptyOrConditions(m_eActivationConditionLogic, m_aActivationConditions, owner);
		if (!conditionStatus && m_ParentLayer && calledFromInit)
			m_ParentLayer.CheckAllChildrenSpawned(this);

		return conditionStatus;
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes area for scenario framework layer
	//! \param[in] area sets parent area if provided or gets parent area if not provided
	//! \return true if area is successfully initialized, false otherwise.
	bool InitArea(SCR_ScenarioFrameworkArea area)
	{
		if (area)
		{
			m_Area = area;
			return true;
		}

		m_Area = GetParentArea();
		if (!m_Area)
		{
			if (m_ParentLayer)
				m_ParentLayer.CheckAllChildrenSpawned(this);

			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Handles inheritance of faction settings from parents
	bool InitFactionSettings()
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sFactionKey))
		{
			// Try Inherit
			if (!m_ParentLayer)
				return true;

			FactionKey parentFactionKey = m_ParentLayer.GetFactionKey();
			if (!SCR_StringHelper.IsEmptyOrWhiteSpace(parentFactionKey))
				SetFactionKey(parentFactionKey);
			return true;
		}

		// Resolve Alias
		SCR_FactionAliasComponent factionAliasComponent = SCR_FactionAliasComponent.Cast(GetGame().GetFactionManager().FindComponent(SCR_FactionAliasComponent));
		if (factionAliasComponent)
			SetFactionKey(factionAliasComponent.ResolveFactionAlias(GetFactionKey()));
		// Its not a mistake if the mission creator didn't define SCR_FactionAliasComponent. It just means that alias resolution is not needed.
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! For situations where some other logic is to be appended in these checks and is to be performed before FinishInit
	bool InitOtherThings()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! For situations where some other logic is needed to be performed before or after this Insert
	void FinishInitChildrenInsert()
	{
		GetOnAllChildrenSpawned().Insert(AfterAllChildrenSpawned);
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes children, retrieves them, and spawns them.
	void FinishInit()
	{
		FinishInitChildrenInsert();
		GetChildren(m_aChildren);
		SpawnChildren();
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes scenario framework layer
	//! \param[in] area that this layer is nested into
	//! \param[in] activation Activation type for scenario framework activation.
	void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		if (InitAlreadyHappened())
			return;

		if (!InitParentLayer())
			return;

		if (!InitFactionSettings())
			return;

		if (!InitNotTerminated())
			return;

		if (!InitDynDespawnAndActivation(activation))
			return;

		if (!InitActivationConditions(true))
			return;

		if (!InitArea(area))
			return;

		if (!InitOtherThings())
			return;

		FinishInit();
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes logic, plugins and actions.
	//! \param[in] layer for which this method is called.
	void AfterAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		m_bInitiated = true;

		ActivateLogic();
		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}

		InitActivationActions();

		if (m_ParentLayer)
			m_ParentLayer.CheckAllChildrenSpawned(this);

		GetOnAllChildrenSpawned().Remove(AfterAllChildrenSpawned);

		if (m_fRepeatedSpawnTimer >= 0)
			RepeatedSpawn();
	}

	//------------------------------------------------------------------------------------------------
	//! Draws debug shapes during runtime if enabled.
	//! \param[in] owner represents the owner entity of this layer.
	//! \param[in] timeSlice represents the time interval for which the method is called during each frame.
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);

		if (m_bShowDebugShapesDuringRuntime)
			DrawDebugShape(true);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] owner represents the entity being processed by the method, which is used for managing debug shapes during runtime based on certain conditions.
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (m_bShowDebugShapesDuringRuntime || m_fRepeatedSpawnTimer >= 0)
		{
			//TODO: deactivate once the slots are not needed (after entity was spawned)
			SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);

			if (SCR_Global.IsEditMode())
				return;

			BaseGameMode gameMode = GetGame().GetGameMode();
			if (!gameMode)
				return;

			SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
			if (!scenarioFrameworkSystem)
				return;

			scenarioFrameworkSystem.ManageLayerDebugShape(GetOwner().GetID(), m_bShowDebugShapesDuringRuntime, m_fDebugShapeRadius, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] fSize is the radius for debug shape visualization.
	void SetDebugShapeSize(float fSize)
	{
		m_fDebugShapeRadius = fSize;
	}

	//------------------------------------------------------------------------------------------------
	//! Draws debug shape if draw flag is true, creates sphere shape with specified color, flags, position, and radius.
	//! \param[in] draw debug shape if draw is true, otherwise does nothing.
	protected void DrawDebugShape(bool draw)
	{
		Shape dbgShape = null;
		if (!draw)
			return;

		dbgShape = Shape.CreateSphere(
			m_iDebugShapeColor,
			ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE,
			GetOwner().GetOrigin(),
			m_fDebugShapeRadius
		);
	}

	//------------------------------------------------------------------------------------------------
	protected void InitActivationActions()
	{
		const IEntity owner = GetOwner();

		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActivationActions)
		{
			activationAction.Init(owner);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RestoreActionsToDefault()
	{
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aActivationActions)
		{
			activationAction.RestoreToDefault();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_ScenarioFrameworkActionBase> GetActivationActions()
	{
		return m_aActivationActions;
	}

#ifdef WORKBENCH

	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntity owner, IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}

	//------------------------------------------------------------------------------------------------
	//! Draws debug shape based on m_bShowDebugShapesInWorkbench setting in Workbench after world update.
	//! \param[in] owner The owner represents the entity (object) calling the method.
	//! \param[in] timeSlice TimeSlice represents the time interval for which the method is called during each frame update.
	override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		DrawDebugShape(m_bShowDebugShapesInWorkbench);
	}

	//------------------------------------------------------------------------------------------------
	//! Renames all entities in the owner's children hierarchy.
	//! \param[in] owner Renames owner entity and its children in hierarchy.
	//! \param[in] src Source entity representing the parent object creating this scripted object.
	override void _WB_OnCreate(IEntity owner, IEntitySource src)
	{
		RenameOwnerEntity(owner);

		IEntity child = GetOwner().GetChildren();
		while (child)
		{
			RenameOwnerEntity(child);
			child = child.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Controls debug shape visibility in Workbench based on user input.
	//! \param[in] owner The owner represents the entity (object) in the game world that triggers the method when its key value changes.
	//! \param[in] src BaseContainer src represents input parameter containing key-value pairs related to the key changes in the Workbench.
	//! \param[in] key Controls whether debug shapes are drawn in Workbench.
	//! \param[in] ownerContainers Represents a list of containers related to the owner entity in the method.
	//! \param[in] parent Parent represents the entity that owns the key change event, used for context in the method.
	//! \return: Controls whether debug shapes are drawn in Workbench.
	override bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		if (key == "m_bShowDebugShapesInWorkbench")
			DrawDebugShape(m_bShowDebugShapesInWorkbench);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Rename entity owner with default name if not restoring undo/redo.
	//! \param[in] owner Represents the entity whose name is being changed by the method.
	void RenameOwnerEntity(IEntity owner)
	{
		GenericEntity genericEntity = GenericEntity.Cast(owner);

		WorldEditorAPI api = genericEntity._WB_GetEditorAPI();
		if (!api.UndoOrRedoIsRestoring())
			api.RenameEntity(api.EntityToSource(owner), api.GenerateDefaultEntityName(api.EntityToSource(owner)));
	}
#endif

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_ScenarioFrameworkLayerBase(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		if (m_bDynamicDespawn)
			m_fDebugShapeRadius = m_iDynamicDespawnRange;
		
		m_iDebugShapeColor = ARGB(32, 255, 255, 255);
#ifdef WORKBENCH
		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.OnWBKeyChanged(this);
		}
#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Removes object in edit mode or despawns it if not in edit mode.
	void ~SCR_ScenarioFrameworkLayerBase()
	{
		if (SCR_Global.IsEditMode())
			return;

		DynamicDespawn(this);
	}
}
