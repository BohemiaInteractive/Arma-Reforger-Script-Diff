[EntityEditorProps(category: "GameScripted/ScenarioFramework", description: "")]
class SCR_ScenarioFrameworkAreaClass : SCR_ScenarioFrameworkLayerBaseClass
{
}

// Helper class for designer to specify what tasks will be available in this area
[BaseContainerProps()]
class SCR_ScenarioFrameworkTaskType
{
	[Attribute("Type of task", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ESFTaskType))]
	protected SCR_ESFTaskType m_eTypeOfTask;

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ESFTaskType GetTaskType()
	{
		return m_eTypeOfTask;
	}
}

//! Class is managing the area including the Slots (prefab Slot.et).
//! The Slots should be placed in its hierarchy.
//! GameModePatrolManager will select one instance of this type and will start to populate it based on the selected task
class SCR_ScenarioFrameworkArea : SCR_ScenarioFrameworkLayerBase
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, "Trigger for area", category: "Trigger")]
	ResourceName m_sTriggerResource;

	[Attribute(defvalue: "5.0", UIWidgets.Slider, params: "0 inf", desc: "Radius of the trigger area", category: "Trigger")]
	float m_fAreaRadius;

	[Attribute(defvalue: "1", UIWidgets.CheckBox, desc: "Activate the trigger once or everytime the activation condition is true?", category: "Trigger")]
	bool m_bOnce;

	[Attribute(desc: "Actions that will be activated when Trigger gets activated", category: "OnActivation")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aTriggerActions;

	SCR_BaseTriggerEntity m_Trigger;
	ref ScriptInvoker<SCR_ScenarioFrameworkArea, SCR_ScenarioFrameworkEActivationType>	m_OnTriggerActivated;
	ref ScriptInvoker m_OnAreaInit = new ScriptInvoker();
	bool m_bAreaSelected;
	SCR_Task m_Task;
	string m_sItemDeliveryPointName;
	SCR_ScenarioFrameworkLayerTask m_LayerTask;
	SCR_ScenarioFrameworkSlotTask m_SlotTask; 				//storing this one in order to get the task title and description

	ref array<ref Tuple3<SCR_ScenarioFrameworkLayerBase, vector, int>> m_aSpawnedLayers = {};
	ref array<ref Tuple3<SCR_ScenarioFrameworkLayerBase, vector, int>> m_aDespawnedLayers = {};

	//------------------------------------------------------------------------------------------------
	//! \return the current task assigned to the area.
	SCR_Task GetTask()
	{
		return m_Task;
	}

	//------------------------------------------------------------------------------------------------
	//! \return slot task associated with this scenario framework area.
	SCR_ScenarioFrameworkSlotTask GetSlotTask()
	{
		return m_SlotTask;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] slotTask associated with this scenario framework area.
	void SetSlotTask(SCR_ScenarioFrameworkSlotTask slotTask)
	{
		m_SlotTask = slotTask;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the layer task associated with this scenario framework area.
	override SCR_ScenarioFrameworkLayerTask GetLayerTask()
	{
		return m_LayerTask;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] layerTask associated with this scenario framework area.
	void SetLayerTask(SCR_ScenarioFrameworkLayerTask layerTask)
	{
		m_LayerTask = layerTask;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the name of the owner entity of the layer task.
	string GetLayerTaskName()
	{
		return m_LayerTask.GetOwner().GetName();
	}

	//------------------------------------------------------------------------------------------------
	//! \return the task type associated with the layer task.
	SCR_ESFTaskType GetLayerTaskType()
	{
		return m_LayerTask.GetTaskType();
	}

	//------------------------------------------------------------------------------------------------
	//! \return the name of the delivery point for an item.
	string GetDeliveryPointName()
	{
		return m_sItemDeliveryPointName;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets delivery point name for task delivery and triggers it with given name.
	//! \param[in] sDeliveryPointName Represents the name of the delivery point for an item in the mission.
	void StoreDeliveryPoint(string sDeliveryPointName)
	{
		m_sItemDeliveryPointName = sDeliveryPointName;
		if (!SCR_TaskDeliver.Cast(m_Task))
			return;

		SCR_TaskDeliver.Cast(m_Task).SetTriggerNameToDeliver(sDeliveryPointName);
	}

	//------------------------------------------------------------------------------------------------
	//! Store Task in area.
	//! \param[in] task
	void StoreTaskToArea(SCR_Task task)
	{
		m_Task = task;
	}

	//------------------------------------------------------------------------------------------------
	//! Randomly selects an available scenario layer from all available slots and initializes it.
	//! \return Randomly selected Layer Task
	SCR_ScenarioFrameworkLayerTask Create()
	{
		array<SCR_ScenarioFrameworkLayerBase> aSlotsOut = {};
		GetChildren(aSlotsOut);
		if (aSlotsOut.IsEmpty())
			return null;

		SCR_ScenarioFrameworkLayerBase layer = aSlotsOut.GetRandomElement();
		if (layer)
			layer.Init(this);

		return SCR_ScenarioFrameworkLayerTask.Cast(layer);
	}

	//------------------------------------------------------------------------------------------------
	//! Creates a suitable layer for a given task type, initializes it, and returns it.
	//! \param[in] eTaskType eTaskType represents the type of task for which suitable layers are searched in the method.
	//! \return a random suitable layer task for the given task type.
	SCR_ScenarioFrameworkLayerTask Create(SCR_ESFTaskType eTaskType)
	{
		array<SCR_ScenarioFrameworkLayerBase> aSlotsOut = {};
		GetSuitableLayersForTaskType(aSlotsOut, eTaskType);
		if (aSlotsOut.IsEmpty())
			return null;

		//there might be more layers in the area conforming to the task type (i.e. 2x the Truck task)
		m_LayerTask = SCR_ScenarioFrameworkLayerTask.Cast(aSlotsOut.GetRandomElement());
		if (m_LayerTask)
			m_LayerTask.Init(this, SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT);

		return m_LayerTask;
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes layer task with random spawned child slot task on tasks init.
	//! \param[in] layerTask LayerTask represents the parent task for spawning child tasks randomly in the scenario.
	//! \param[in] slotTask SlotTask represents a task within a layer, which can be added randomly to the layer by the method.
	void Create(SCR_ScenarioFrameworkLayerTask layerTask, SCR_ScenarioFrameworkSlotTask slotTask = null)
	{
		if (!layerTask)
			return;

		m_LayerTask = layerTask;
		m_LayerTask.AddRandomlySpawnedChild(slotTask);
		m_LayerTask.Init(this, SCR_ScenarioFrameworkEActivationType.ON_TASKS_INIT);
	}

	//------------------------------------------------------------------------------------------------
	//! \return Represents the trigger entity associated with this area.
	SCR_BaseTriggerEntity GetTrigger()
	{
		return m_Trigger;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns a trigger entity with specified resource, sets its radius, and attaches an activation event.
	protected void SetupTrigger()
	{
		if (!m_Trigger)
		{
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			GetOwner().GetWorldTransform(spawnParams.Transform);
			m_Trigger = SCR_BaseTriggerEntity.Cast(GetGame().SpawnEntityPrefabEx(m_sTriggerResource, false, params: spawnParams));
		}

		if (!m_Trigger)
			return;

		m_Trigger.SetSphereRadius(m_fAreaRadius);
		m_Trigger.GetOnActivate().Insert(OnAreaTriggerActivated);

		SCR_ScenarioFrameworkTriggerEntity sfTrigger = SCR_ScenarioFrameworkTriggerEntity.Cast(m_Trigger);
		if (sfTrigger)
			sfTrigger.SetOnce(m_bOnce);

		if (m_Trigger)
		{
			foreach (SCR_ScenarioFrameworkActionBase triggerAction : m_aTriggerActions)
			{
				triggerAction.Init(m_Trigger);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Moves task icon to area position.
	//! \param[in] task
	void MoveTaskIconToArea(notnull SCR_Task task)
	{
		task.SetOrigin(GetOwner().GetOrigin());

	}

	//------------------------------------------------------------------------------------------------
	//! Checks if given task type is suitable for current area by checking if it's in available task types list.
	//! \param[in] eTaskType Task type enum representing possible tasks in the area.
	//! \return true if the given task type is suitable for the current area, false otherwise.
	bool GetIsTaskSuitableForArea(SCR_ESFTaskType eTaskType)
	{
		array<SCR_ESFTaskType> aTaskTypes = {};
		GetAvailableTaskTypes(aTaskTypes);
		if (aTaskTypes.IsEmpty())
			return false;

		return aTaskTypes.Find(eTaskType) != -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Gets available task types from scenario layers and adds them to an array.
	//! \param[out] aTaskTypes The array of available task types for the scenario framework layer.
	void GetAvailableTaskTypes(out array<SCR_ESFTaskType> aTaskTypes)
	{
		array<SCR_ScenarioFrameworkLayerBase> aSlots = {};
		GetChildren(aSlots);
		SCR_ESFTaskType eType;
		SCR_ScenarioFrameworkLayerTask pos;
		foreach (SCR_ScenarioFrameworkLayerBase layer : aSlots)
		{
			pos = SCR_ScenarioFrameworkLayerTask.Cast(layer);
			if (!pos)
				continue;

			eType = pos.GetTaskType();
			if (aTaskTypes.Find(eType) == -1)
				aTaskTypes.Insert(eType);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Filters suitable layers for given task type, populates output array with matching layers.
	//! \param[out] aSlotsOut Array of suitable layers for given task type.
	//! \param[in] eTaskType eTaskType is the task type parameter used to filter suitable layers for the given task type in the scenario framework.
	void GetSuitableLayersForTaskType(out notnull array<SCR_ScenarioFrameworkLayerBase> aSlotsOut, SCR_ESFTaskType eTaskType)
	{
		SCR_ESFTaskType eType;
		array<SCR_ScenarioFrameworkLayerBase> aSlots = {};
		GetChildren(aSlots);
		SCR_ScenarioFrameworkLayerTask pos;
		foreach (SCR_ScenarioFrameworkLayerBase layer : aSlots)
		{
			pos = SCR_ScenarioFrameworkLayerTask.Cast(layer);
			if (!pos)
				continue;

			eType = pos.GetTaskType();
			if (eTaskType == eType)
				aSlotsOut.Insert(pos);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] bSet is a boolean parameter indicating whether to set or unset the area selection.
	void SetAreaSelected(bool bSet)
	{
		 m_bAreaSelected = bSet;
	}

	//------------------------------------------------------------------------------------------------
	//! \return whether an area is selected or not.
	bool GetIsAreaSelected()
	{
		return m_bAreaSelected;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entity Triggers area activation event for scenario framework, enabling periodic queries if not already done once.
	void OnAreaTriggerActivated(IEntity entity)
	{
		if (m_OnTriggerActivated)
		{
			m_OnTriggerActivated.Invoke(this, SCR_ScenarioFrameworkEActivationType.ON_AREA_TRIGGER_ACTIVATION, false);
			m_OnTriggerActivated.Clear();
			if (!m_bOnce)
				return;

			m_Trigger.EnablePeriodicQueries(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return a ScriptInvoker object for handling area trigger activation events.
	ScriptInvoker GetOnAreaTriggerActivated()
	{
		if (!m_OnTriggerActivated)
			m_OnTriggerActivated = new ScriptInvoker<SCR_ScenarioFrameworkArea, SCR_ScenarioFrameworkEActivationType>();

		return m_OnTriggerActivated;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the event handler for area initialization.
	ScriptInvoker GetOnAreaInit()
	{
		return m_OnAreaInit;
	}

	//------------------------------------------------------------------------------------------------
	//! Restores default settings, deselects area, nullifies triggers, resets item delivery point, and calls base class
	//! \param[in] includeChildren Restores default settings, optionally including children objects.
	//! \param[in] reinitAfterRestoration Restores object state, optionally reinitializes after restoration.
	//! \param[in] affectRandomization Affects randomization settings during restoration.
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false, bool affectRandomization = true)
	{
		m_bAreaSelected = false;
		m_Trigger = null;
		m_sItemDeliveryPointName = "";
		m_LayerTask = null;
		m_SlotTask = null;

		super.RestoreToDefault(includeChildren, reinitAfterRestoration, affectRandomization);
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes dynamic reinitialization by calling Init() method.
	override void DynamicReinit()
	{
		Init();
	}

	//------------------------------------------------------------------------------------------------
	//! Removes all children layers from the scenario framework layer and despawns itself if not excluded from dynamic despawning.
	//! \param[in] layer Removes layer from dynamic despawning, clears children layers.
	override void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
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
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes parent layer with true value.
	//! \return true if parent layer initialization is successful.
	override bool InitParentLayer()
	{
		// Areas do not have parents
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes area, spawns trigger if not already spawned, initializes trigger actions if trigger is present, and sets activation type
	//! \param[in] area parent
	//! \param[in] activation Activation type determines how scenario area is initiated, either on init or based on specific conditions.
	override void Init(SCR_ScenarioFrameworkArea area = null, SCR_ScenarioFrameworkEActivationType activation = SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT)
	{
		if (m_eActivationType != SCR_ScenarioFrameworkEActivationType.ON_INIT)
			PrintFormat("ScenarioFramework: Area %1 is set to %2 activation type, but area will always spawn on Init as default", GetOwner().GetName(), activation, LogLevel.WARNING);

		// Area is always spawned on the start
		super.Init(this, SCR_ScenarioFrameworkEActivationType.ON_INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes Trigger
	//! \param[in] layer for which this method is called.
	override void AfterAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		super.AfterAllChildrenSpawned(layer);

		SetupTrigger();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prepares dynamic spawn/despawn for specific layer (Intended for runtime usage)
	//! \param[in] layer
	//! \param[in] staySpawned
	void PrepareLayerSpecificDynamicDespawn(SCR_ScenarioFrameworkLayerBase layer, bool staySpawned = false, int despawnRange = 0)
	{
		layer.SetDynamicDespawnEnabled(true);
		layer.SetDynamicDespawnRange(despawnRange);
		
		if (m_aDespawnedLayers.IsEmpty() && m_aSpawnedLayers.IsEmpty())
			PrepareDynamicDespawn();

		//If this method is called with staySpawned = false, layer will be added to m_aDespawnedLayers and gets despawned
		if (!staySpawned)
		{
			m_aDespawnedLayers.Insert(new Tuple3<SCR_ScenarioFrameworkLayerBase, vector, int>(layer, layer.GetOwner().GetOrigin(), (despawnRange * despawnRange)));
			layer.DynamicDespawn(layer);
		}
		else
		{
			m_aSpawnedLayers.Insert(new Tuple3<SCR_ScenarioFrameworkLayerBase, vector, int>(layer, layer.GetOwner().GetOrigin(), (despawnRange * despawnRange)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Removes dynamic spawn/despawn for specific layer (Intended for runtime usage)
	//! \param[in] layer
	//! \param[in] staySpawned
	void RemoveLayerSpecificDynamicDespawn(SCR_ScenarioFrameworkLayerBase layer, bool staySpawned = false)
	{
		layer.SetDynamicDespawnEnabled(false);
		
		//If this method is called with staySpawned = false, layer will be despawned
		if (!staySpawned)
			layer.DynamicDespawn(layer);

		for (int i = m_aDespawnedLayers.Count() - 1; i >= 0; i--)
		{
			Tuple3<SCR_ScenarioFrameworkLayerBase, vector, int> layerInfo = m_aDespawnedLayers[i];
			if (layer == layerInfo.param1)
				m_aDespawnedLayers.Remove(i);
		}

		for (int i = m_aSpawnedLayers.Count() - 1; i >= 0; i--)
		{
			Tuple3<SCR_ScenarioFrameworkLayerBase, vector, int> layerInfo = m_aSpawnedLayers[i];
			if (layer == layerInfo.param1)
				m_aSpawnedLayers.Remove(i);
		}
		
		if (m_aDespawnedLayers.IsEmpty() && m_aSpawnedLayers.IsEmpty())
			RemoveDynamicDespawn();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Prepares dynamic spawn/despawn for layers
	void PrepareDynamicDespawn()
	{
		if (!m_bDynamicDespawn)
			return;
		
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;
		
		ExecuteDynamicDespawn(scenarioFrameworkSystem);
		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(ExecuteDynamicDespawn, 1000 * scenarioFrameworkSystem.m_iUpdateRate, true, scenarioFrameworkSystem);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Removes dynamic spawn/despawn for layers
	void RemoveDynamicDespawn()
	{
		if (!m_bDynamicDespawn)
			return;
		
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;
		
		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().Remove(ExecuteDynamicDespawn);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Executes dynamic spawn/despawn for layers
	void ExecuteDynamicDespawn(SCR_ScenarioFrameworkSystem scenarioFrameworkSystem)
	{
		if (!scenarioFrameworkSystem)
			return;
		
		DynamicSpawnLayer(scenarioFrameworkSystem.m_aObservers);
		DynamicDespawnLayer(scenarioFrameworkSystem.m_aObservers);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Goes over despawned layers and checks whether or not said area should spawn
	void DynamicSpawnLayer(array<vector> observers)
	{
		Tuple3<SCR_ScenarioFrameworkLayerBase, vector, int> layerInfo;
		for (int i = m_aDespawnedLayers.Count() - 1; i >= 0; i--)
		{
			layerInfo = m_aDespawnedLayers[i];
			if (!layerInfo.param1)
			{
				continue;
			}
			
			foreach (vector observerPos : observers)
			{
				if (vector.DistanceSqXZ(observerPos, layerInfo.param2) < layerInfo.param3)
				{
					layerInfo.param1.DynamicReinit();
					m_aSpawnedLayers.Insert(layerInfo);
					m_aDespawnedLayers.Remove(i);
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Goes over spawned layers and checks whether or not said area should despawn
	void DynamicDespawnLayer(array<vector> observers)
	{
		Tuple3<SCR_ScenarioFrameworkLayerBase, vector, int> layerInfo;
		for (int i = m_aSpawnedLayers.Count() - 1; i >= 0; i--)
		{
			layerInfo = m_aSpawnedLayers[i];
			if (!layerInfo.param1)
				continue;
			
			bool observerInRange;
			foreach (vector observerPos : observers)
			{
				if (vector.DistanceSqXZ(observerPos, layerInfo.param2) < layerInfo.param3)
				{
					observerInRange = true;
					break;
				}
			}

			if (!observerInRange)
			{
				layerInfo.param1.DynamicDespawn(null);
				m_aDespawnedLayers.Insert(layerInfo);
				m_aSpawnedLayers.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes area registration with ScenarioFrameworkSystem when game mode is valid.
	//! \param[in] owner The owner represents the entity that initializes this script on its EOnInit event.
	override void EOnInit(IEntity owner)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (scenarioFrameworkSystem)
			scenarioFrameworkSystem.RegisterArea(this);

		array<SCR_ScenarioFrameworkLayerBase> children = {};
		GetAllLayers(children);

		foreach (SCR_ScenarioFrameworkLayerBase child : children)
		{
			if (child.GetActivationType() == SCR_ScenarioFrameworkEActivationType.ON_INIT)
			{
				if (scenarioFrameworkSystem && scenarioFrameworkSystem.IsMaster())
					child.Init(this, SCR_ScenarioFrameworkEActivationType.ON_INIT);

				child.SetActivationType(SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
			}
			
			if (!child.GetDynamicDespawnEnabled())
				continue;
			
			int despawnRange = child.GetDynamicDespawnRange();
			m_aDespawnedLayers.Insert(new Tuple3<SCR_ScenarioFrameworkLayerBase, vector, int>(child, child.GetOwner().GetOrigin(), (despawnRange * despawnRange)));
		}
		
		if (!m_aDespawnedLayers.IsEmpty())
			PrepareDynamicDespawn();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] owner The owner represents the entity being initialized in the method.
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);

		super.OnPostInit(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Draws debug shapes for debugging
	//! \param[in] draw debug shape for the object if draw parameter is true, otherwise does not draw anything.
	override protected void DrawDebugShape(bool draw)
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

		if (m_sTriggerResource.IsEmpty())
			return;

		Shape triggerdbgShape = null;
		triggerdbgShape = Shape.CreateSphere(
										ARGB(100, 0x99, 0x10, 0xF2),
										ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE | ShapeFlags.ONCE | ShapeFlags.NOOUTLINE,
										GetOwner().GetOrigin(),
										m_fAreaRadius
								);
	}
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_ScenarioFrameworkArea(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_fDebugShapeRadius = m_iDynamicDespawnRange;
#ifdef WORKBENCH
		m_iDebugShapeColor = ARGB(32, 0x99, 0xF3, 0x12);
#endif
	}
}
