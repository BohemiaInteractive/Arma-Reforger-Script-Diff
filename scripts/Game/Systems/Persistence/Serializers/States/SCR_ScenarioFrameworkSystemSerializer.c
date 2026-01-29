//! Dummy state proxy to access scenario framework system instance
class SCR_ScenarioFrameworkSystemData : PersistentState
{
}

class SCR_ScenarioFrameworkSystemSerializer : ScriptedStateSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return SCR_ScenarioFrameworkSystemData;
	}

	//------------------------------------------------------------------------------------------------
	override ESerializeResult Serialize(notnull Managed instance, notnull BaseSerializationSaveContext context)
	{
		const SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return ESerializeResult.DEFAULT;

		if (scenarioFrameworkSystem.GetIsMatchOver())
		{
			context.WriteValue("version", 1);
			context.WriteValue("matchOver", true);
		}

		const EGameOverTypes gameOverType = scenarioFrameworkSystem.m_eGameOverType;

		// Area data
		array<ref SCR_ScenarioFrameworkAreaSave> areas();
		StoreAreaStates(GetSystem(), scenarioFrameworkSystem, areas);

		// After Tasks Init Actions
		array<ref SCR_ScenarioFrameworkActionSave> afterTasksInitActions();
		SCR_ScenarioFrameworkActionSave.ReadActions(GetSystem(), scenarioFrameworkSystem.m_aAfterTasksInitActions, afterTasksInitActions);

		if ((gameOverType == EGameOverTypes.COMBATPATROL_DRAW) &&
			areas.IsEmpty() &&
			afterTasksInitActions.IsEmpty() &&
			scenarioFrameworkSystem.m_mVariableMap.IsEmpty())
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteValueDefault("matchOver", false, false);
		context.WriteDefault(gameOverType, EGameOverTypes.COMBATPATROL_DRAW);

		if (!context.CanSeekMembers() || !areas.IsEmpty())
			context.Write(areas);

		if (!context.CanSeekMembers() || !afterTasksInitActions.IsEmpty())
			context.Write(afterTasksInitActions);

		if (!context.CanSeekMembers() || !scenarioFrameworkSystem.m_mVariableMap.IsEmpty())
			context.WriteValue("variables", scenarioFrameworkSystem.m_mVariableMap);

		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull BaseSerializationLoadContext context)
	{
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return true; // SF system no longer present, so we can ignore it.

		scenarioFrameworkSystem.m_bIsSaveGameLoad = true;

		int version;
		context.Read(version);

		bool matchOver;
		context.ReadDefault(matchOver, false);

		EGameOverTypes gameOverType;
		if (context.Read(gameOverType))
			scenarioFrameworkSystem.m_eGameOverType = gameOverType;

		array<ref SCR_ScenarioFrameworkAreaSave> areas();
		if (!context.Read(areas) && !context.CanSeekMembers())
			return false;

		LoadAreaStates(GetSystem(), scenarioFrameworkSystem, areas);

		array<ref SCR_ScenarioFrameworkActionSave> afterTasksInitActions();
		if (!context.Read(afterTasksInitActions) && !context.CanSeekMembers())
			return false;

		foreach (auto action : afterTasksInitActions)
		{
			action.Write(GetSystem(), scenarioFrameworkSystem, scenarioFrameworkSystem.m_aAfterTasksInitActions);
		}

		context.ReadValue("variables", scenarioFrameworkSystem.m_mVariableMap);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void StoreAreaStates(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, out array<ref SCR_ScenarioFrameworkAreaSave> saves)
	{
		for (int i = scenarioFrameworkSystem.m_aAreas.Count() - 1; i >= 0; i--)
		{
			SCR_ScenarioFrameworkAreaSave save();
			save.Read(persistence, scenarioFrameworkSystem.m_aAreas[i]);
			if (!save.IsDefault())
				saves.Insert(save);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadAreaStates(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, array<ref SCR_ScenarioFrameworkAreaSave> saves)
	{
		foreach (SCR_ScenarioFrameworkAreaSave areaSave : saves)
		{
			auto entity = GetGame().GetWorld().FindEntityByName(areaSave.m_sName);
			if (!entity)
				continue;

			auto area = SCR_ScenarioFrameworkArea.Cast(entity.FindComponent(SCR_ScenarioFrameworkArea));
			if (!area)
				continue;

			areaSave.Write(persistence, scenarioFrameworkSystem, area);
		}
	}
}

class SCR_ScenarioFrameworkLayerSave
{
	string											m_sName;
	bool 											m_bIsTerminated;
	int												m_iActivationType;
	int 											m_iRepeatedSpawnNumber;
	ref array<string>								m_aRandomlySpawnedChildren = {};
	ref array<ref SCR_ScenarioFrameworkLogicSave>	m_aLogics = {};
	ref array<ref SCR_ScenarioFrameworkLayerSave>	m_aChildLayers = {};
	ref array<ref SCR_ScenarioFrameworkActionSave>	m_aActivationActions = {};
	ref array<ref SCR_ScenarioFrameworkPluginSave>	m_aPlugins = {};

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkLayerSave GetLayerTypeSave(const SCR_ScenarioFrameworkLayerBase layer)
	{
		if (layer.IsInherited(SCR_ScenarioFrameworkArea))
			return new SCR_ScenarioFrameworkAreaSave();

		if (layer.IsInherited(SCR_ScenarioFrameworkLayerTask))
			return new SCR_ScenarioFrameworkLayerTaskSave();

		if (layer.IsInherited(SCR_ScenarioFrameworkLayerTaskDefend))
			return new SCR_ScenarioFrameworkLayerTaskDefendSave();

		if (layer.IsInherited(SCR_ScenarioFrameworkSlotBase))
			return new SCR_ScenarioFrameworkSlotSave();

		if (layer.IsInherited(SCR_ScenarioFrameworkSlotAI))
			return new SCR_ScenarioFrameworkSlotAISave();

		if (layer.IsInherited(SCR_ScenarioFrameworkSlotTask))
			return new SCR_ScenarioFrameworkSlotTaskSave();

		if (layer.IsInherited(SCR_ScenarioFrameworkSlotTaskAI))
			return new SCR_ScenarioFrameworkSlotTaskAISave();

		if (layer.IsInherited(SCR_ScenarioFrameworkSlotTrigger))
			return new SCR_ScenarioFrameworkSlotTriggerSave();

		return new SCR_ScenarioFrameworkLayerSave();
	}

	//------------------------------------------------------------------------------------------------
	void Read(PersistenceSystem persistence, SCR_ScenarioFrameworkLayerBase layer)
	{
		m_sName = layer.GetName();

		m_iActivationType = layer.GetActivationType();
		if (m_iActivationType == layer.m_eActivationTypeDefault)
			m_iActivationType = -1; // Default

		m_bIsTerminated = layer.GetIsTerminated();

		m_iRepeatedSpawnNumber = layer.GetRepeatedSpawnNumber();
		if (m_iRepeatedSpawnNumber == layer.m_iRepeatedSpawnNumberDefault)
			m_iRepeatedSpawnNumber = 0;

		// Logics
		array<SCR_ScenarioFrameworkLogic> logics();
		layer.GetLogics(logics);
		foreach (SCR_ScenarioFrameworkLogic logic : logics)
		{
			auto logicCounter = SCR_ScenarioFrameworkLogicCounter.Cast(logic);
			if (!logicCounter)
				continue;

			SCR_ScenarioFrameworkLogicCounterSave logicSave();
			logicSave.Read(persistence, logicCounter);
			if (!logicSave.IsDefault())
				m_aLogics.Insert(logicSave);
		}

		// Randomization
		if (layer.GetSpawnChildrenType() != SCR_EScenarioFrameworkSpawnChildrenType.ALL)
		{
			foreach (auto randomlayer : layer.GetRandomlySpawnedChildren())
			{
				m_aRandomlySpawnedChildren.Insert(randomlayer.GetName());
			}
		}

		// Children
		array<SCR_ScenarioFrameworkLayerBase> children();
		layer.GetChildren(children);
		foreach (auto childLayer : children)
		{
			auto childSave = GetLayerTypeSave(childLayer);
			childSave.Read(persistence, childLayer);
			if (!childSave.IsDefault())
				m_aChildLayers.Insert(childSave);
		}

		// Activation Actions
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, layer.GetActivationActions(), m_aActivationActions);

		//  Plugins
		auto plugins = layer.GetSpawnedPlugins();
		foreach (SCR_ScenarioFrameworkPlugin plugin : plugins)
		{
			auto pluginSave = SCR_ScenarioFrameworkPluginSave.GetSaveType(plugin);
			pluginSave.Read(persistence, layer, plugin);
			if (!pluginSave.IsDefault())
				m_aPlugins.Insert(pluginSave);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsDefault()
	{
		return
			m_aChildLayers.IsEmpty() &&
			m_aLogics.IsEmpty() &&
			m_aRandomlySpawnedChildren.IsEmpty() &&
			m_iActivationType == -1 &&
			m_bIsTerminated == false &&
			m_iRepeatedSpawnNumber == 0 &&
			m_aActivationActions.IsEmpty() &&
			m_aPlugins.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	void Write(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, SCR_ScenarioFrameworkLayerBase layer)
	{
		if (m_iRepeatedSpawnNumber != 0)
		{
			layer.SetEnableRepeatedSpawn(true);
			layer.SetRepeatedSpawnNumber(m_iRepeatedSpawnNumber);
		}

		if (m_iActivationType != -1)
			layer.SetActivationType(m_iActivationType);

		layer.SetIsTerminated(m_bIsTerminated);

		if (!m_aRandomlySpawnedChildren.IsEmpty())
			layer.SetRandomlySpawnedChildren(m_aRandomlySpawnedChildren);

		foreach (auto logic : m_aLogics)
		{
			logic.Write(persistence, scenarioFrameworkSystem);
		}

		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActivationActions, layer.GetActivationActions());

		foreach (auto plugin : m_aPlugins)
		{
			plugin.Write(persistence, scenarioFrameworkSystem, layer, layer.GetSpawnedPlugins());
		}

		BaseWorld world = GetGame().GetWorld();
		foreach (auto saveData : m_aChildLayers)
		{
			IEntity entity = world.FindEntityByName(saveData.m_sName);
			if (!entity)
				continue;

			auto childlayer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (childlayer)
				saveData.Write(persistence, scenarioFrameworkSystem, childlayer);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool SerializationSave(BaseSerializationSaveContext context)
	{
		context.Write(m_sName);

		if (!context.CanSeekMembers() || !m_aChildLayers.IsEmpty())
			context.Write(m_aChildLayers);

		if (!context.CanSeekMembers() || !m_aLogics.IsEmpty())
			context.Write(m_aLogics);

		if (!context.CanSeekMembers() || !m_aRandomlySpawnedChildren.IsEmpty())
			context.Write(m_aRandomlySpawnedChildren);

		context.WriteDefault(m_iActivationType, -1);
		context.WriteDefault(m_iRepeatedSpawnNumber, 0);
		context.WriteDefault(m_bIsTerminated, false);

		if (!context.CanSeekMembers() || !m_aActivationActions.IsEmpty())
			context.Write(m_aActivationActions);

		if (!context.CanSeekMembers() || !m_aPlugins.IsEmpty())
			context.Write(m_aPlugins);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool SerializationLoad(BaseSerializationLoadContext context)
	{
		context.Read(m_sName);

		if (!context.Read(m_aChildLayers) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aLogics) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aRandomlySpawnedChildren) && !context.CanSeekMembers())
			return false;

		context.ReadDefault(m_iActivationType, -1);
		context.ReadDefault(m_iRepeatedSpawnNumber, 0);
		context.ReadDefault(m_bIsTerminated, false);

		if (!context.Read(m_aActivationActions) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aPlugins) && !context.CanSeekMembers())
			return false;

		return true;
	}
}

class SCR_ScenarioFrameworkAreaSave : SCR_ScenarioFrameworkLayerSave
{
	bool m_bAreaSelected;
	UUID m_sTriggerId;
	string m_sItemDeliveryPointName;
	string m_sLayerTaskName;
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aTriggerActions = {};

	//------------------------------------------------------------------------------------------------
	override void Read(PersistenceSystem persistence, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Read(persistence, layer);

		auto area = SCR_ScenarioFrameworkArea.Cast(layer);
		m_bAreaSelected = area.GetIsAreaSelected();
		m_sTriggerId = persistence.GetId(area.GetTrigger());

		m_sItemDeliveryPointName = area.GetDeliveryPointName();
		if (area.GetLayerTask())
			m_sLayerTaskName = area.GetLayerTaskName();

		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, area.m_aTriggerActions, m_aTriggerActions);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return
			super.IsDefault() &&
			!m_bAreaSelected &&
			m_sTriggerId.IsNull() &&
			m_sItemDeliveryPointName.IsEmpty() &&
			m_sLayerTaskName.IsEmpty() &&
			m_aTriggerActions.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override void Write(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Write(persistence, scenarioFrameworkSystem, layer);

		auto area = SCR_ScenarioFrameworkArea.Cast(layer);

		if (m_bAreaSelected)
		{
			scenarioFrameworkSystem.m_aAreasTasksToSpawn.Insert(m_sName);
			scenarioFrameworkSystem.m_aLayersTaskToSpawn.Insert(m_sLayerTaskName);
		}

		if (!m_sTriggerId.IsNull())
		{
			Tuple1<SCR_ScenarioFrameworkArea> triggerContext(area);
			PersistenceWhenAvailableTask triggerTask(OnTriggerAvailable, triggerContext);
			persistence.WhenAvailable(m_sTriggerId, triggerTask);
		}

		if (m_sItemDeliveryPointName)
			area.StoreDeliveryPoint(m_sItemDeliveryPointName);

		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aTriggerActions, area.m_aTriggerActions);
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnTriggerAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto trigger = SCR_BaseTriggerEntity.Cast(instance);
		if (!trigger)
			return;

		auto ctx = Tuple1<SCR_ScenarioFrameworkArea>.Cast(context);
		if (ctx.param1)
			ctx.param1.m_Trigger = trigger;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_bAreaSelected, false);
		context.WriteDefault(m_sTriggerId, UUID.NULL_UUID);
		context.WriteDefault(m_sItemDeliveryPointName, string.Empty);
		context.WriteDefault(m_sLayerTaskName, string.Empty);

		if (!context.CanSeekMembers() || !m_aTriggerActions.IsEmpty())
			context.Write(m_aTriggerActions);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_bAreaSelected, false);
		context.ReadDefault(m_sTriggerId, UUID.NULL_UUID);
		context.ReadDefault(m_sItemDeliveryPointName, string.Empty);
		context.ReadDefault(m_sLayerTaskName, string.Empty);

		if (!context.Read(m_aTriggerActions) && !context.CanSeekMembers())
			return false;

		return true;
	}
}

class SCR_ScenarioFrameworkLayerTaskSave : SCR_ScenarioFrameworkLayerSave
{
	private bool m_bTaskActive;
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aTriggerActionsOnFinish = {};
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnCreated = {};
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnFailed = {};
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnCancelled = {};
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnProgress = {};
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnAssigned = {};

	//------------------------------------------------------------------------------------------------
	override void Read(PersistenceSystem persistence, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Read(persistence, layer);

		auto layerTask = SCR_ScenarioFrameworkLayerTask.Cast(layer);
		m_bTaskActive = layerTask.m_Task != null;

		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, layerTask.m_aTriggerActionsOnFinish, m_aTriggerActionsOnFinish);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, layerTask.m_aActionsOnCreated, m_aActionsOnCreated);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, layerTask.m_aActionsOnFailed, m_aActionsOnFailed);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, layerTask.m_aActionsOnCancelled, m_aActionsOnCancelled);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, layerTask.m_aActionsOnProgress, m_aActionsOnProgress);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, layerTask.m_aActionsOnAssigned, m_aActionsOnAssigned);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return
			super.IsDefault() &&
			!m_bTaskActive &&
			m_aTriggerActionsOnFinish.IsEmpty() &&
			m_aActionsOnCreated.IsEmpty() &&
			m_aActionsOnFailed.IsEmpty() &&
			m_aActionsOnCancelled.IsEmpty() &&
			m_aActionsOnProgress.IsEmpty() &&
			m_aActionsOnAssigned.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override void Write(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Write(persistence, scenarioFrameworkSystem, layer);

		auto taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		auto task = SCR_ScenarioFrameworkTask.Cast(taskSystem.GetTaskFromTaskID(layer.GetName()));
		if (!task)
			return;

		auto layerTask = SCR_ScenarioFrameworkLayerTask.Cast(layer);
		layerTask.m_Task = task;
		task.SetLayerTask(layerTask);

		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aTriggerActionsOnFinish, layerTask.m_aTriggerActionsOnFinish);
		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnCreated, layerTask.m_aActionsOnCreated);
		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnFailed, layerTask.m_aActionsOnFailed);
		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnCancelled, layerTask.m_aActionsOnCancelled);
		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnProgress, layerTask.m_aActionsOnProgress);
		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnAssigned, layerTask.m_aActionsOnAssigned);
	}


	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		if (!context.CanSeekMembers() || !m_aTriggerActionsOnFinish.IsEmpty())
			context.Write(m_aTriggerActionsOnFinish);

		if (!context.CanSeekMembers() || !m_aActionsOnCreated.IsEmpty())
			context.Write(m_aActionsOnCreated);

		if (!context.CanSeekMembers() || !m_aActionsOnFailed.IsEmpty())
			context.Write(m_aActionsOnFailed);

		if (!context.CanSeekMembers() || !m_aActionsOnCancelled.IsEmpty())
			context.Write(m_aActionsOnCancelled);

		if (!context.CanSeekMembers() || !m_aActionsOnProgress.IsEmpty())
			context.Write(m_aActionsOnProgress);

		if (!context.CanSeekMembers() || !m_aActionsOnAssigned.IsEmpty())
			context.Write(m_aActionsOnAssigned);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		if (!context.Read(m_aTriggerActionsOnFinish) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aActionsOnCreated) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aActionsOnFailed) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aActionsOnCancelled) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aActionsOnProgress) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aActionsOnAssigned) && !context.CanSeekMembers())
			return false;

		return true;
	}
}

class SCR_ScenarioFrameworkLayerTaskDefendSave : SCR_ScenarioFrameworkLayerTaskSave
{
	float m_fSecondsRemaining = -1.0;

	//------------------------------------------------------------------------------------------------
	override void Read(PersistenceSystem persistence, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Read(persistence, layer);

		auto layerTaskDefend = SCR_ScenarioFrameworkLayerTaskDefend.Cast(layer);
		m_fSecondsRemaining = layerTaskDefend.GetSecondsRemaining();
		const float defendTime = layerTaskDefend.GetDefendTime();
		if (float.AlmostEqual(m_fSecondsRemaining, defendTime))
			m_fSecondsRemaining = -1.0; // Default tiime
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && float.AlmostEqual(m_fSecondsRemaining, -1.0);
	}

	//------------------------------------------------------------------------------------------------
	override void Write(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Write(persistence, scenarioFrameworkSystem, layer);

		auto layerTaskDefend = SCR_ScenarioFrameworkLayerTaskDefend.Cast(layer);
		layerTaskDefend.SetSecondsRemaining(m_fSecondsRemaining);
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_fSecondsRemaining, -1.0);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_fSecondsRemaining, -1.0);
		return true;
	}
}

class SCR_ScenarioFrameworkSlotSave : SCR_ScenarioFrameworkLayerSave
{
	ResourceName m_sRandomlySpawnedObject;
	UUID m_sSlotEntity;
	ref array<UUID> m_aMiscEntities = {};

	//------------------------------------------------------------------------------------------------
	override void Read(PersistenceSystem persistence, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Read(persistence, layer);

		auto slot = SCR_ScenarioFrameworkSlotBase.Cast(layer);
		m_sRandomlySpawnedObject = slot.GetRandomlySpawnedObject();
		auto spawnedEntity = slot.GetSpawnedEntity();
		m_sSlotEntity = persistence.GetId(spawnedEntity);
		foreach (auto entity : slot.m_aSpawnedEntities)
		{
			if (entity == spawnedEntity)
				continue;

			const UUID id = persistence.GetId(entity);
			if (!id.IsNull())
				m_aMiscEntities.Insert(id);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return
			super.IsDefault() &&
			m_sRandomlySpawnedObject.IsEmpty() &&
			m_sSlotEntity.IsNull() &&
			m_aMiscEntities.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override void Write(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Write(persistence, scenarioFrameworkSystem, layer);

		auto slot = SCR_ScenarioFrameworkSlotBase.Cast(layer);

		if (m_sRandomlySpawnedObject)
			slot.SetRandomlySpawnedObject(m_sRandomlySpawnedObject);

		if (!m_sSlotEntity.IsNull())
		{
			Tuple2<SCR_ScenarioFrameworkSlotBase, bool> entityContext(slot, true);
			PersistenceWhenAvailableTask slotEntityTask(OnEntityAvailable, entityContext);
			persistence.WhenAvailable(m_sSlotEntity, slotEntityTask);
		}

		foreach (auto miscEntity : m_aMiscEntities)
		{
			Tuple2<SCR_ScenarioFrameworkSlotBase, bool> entityContext(slot, false);
			PersistenceWhenAvailableTask slotEntityTask(OnEntityAvailable, entityContext);
			persistence.WhenAvailable(miscEntity, slotEntityTask);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnEntityAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto entity = IEntity.Cast(instance);
		if (!entity)
			return;

		auto entityContext = Tuple2<SCR_ScenarioFrameworkSlotBase, bool>.Cast(context);
		if (!entityContext.param1)
			return;

		if (entityContext.param2)
		{
			entityContext.param1.SetEntity(entity);

			auto slotWP = SCR_ScenarioFrameworkSlotWaypoint.Cast(entityContext.param1);
			if (slotWP && slotWP.m_Waypoint)
			{
				slotWP.m_Waypoint.m_SlotWaypoint = slotWP;
				slotWP.m_Waypoint.SetupWaypoint(entity);
			}
		}
		else
		{
			entityContext.param1.m_aSpawnedEntities.Insert(entity);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_sRandomlySpawnedObject, ResourceName.Empty);
		context.WriteDefault(m_sSlotEntity, UUID.NULL_UUID);

		if (!context.CanSeekMembers() || !m_aMiscEntities.IsEmpty())
			context.Write(m_aMiscEntities);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_sRandomlySpawnedObject, ResourceName.Empty);
		context.ReadDefault(m_sSlotEntity, UUID.NULL_UUID);

		if (!context.Read(m_aMiscEntities) && !context.CanSeekMembers())
			return false;

		return true;
	}
}

class SCR_ScenarioFrameworkSlotAISave : SCR_ScenarioFrameworkLayerSave
{
	UUID m_sGroupId;

	//------------------------------------------------------------------------------------------------
	override void Read(PersistenceSystem persistence, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Read(persistence, layer);

		auto slot = SCR_ScenarioFrameworkSlotAI.Cast(layer);
		m_sGroupId = persistence.GetId(slot.m_AIGroup);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && m_sGroupId.IsNull();
	}

	//------------------------------------------------------------------------------------------------
	override void Write(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Write(persistence, scenarioFrameworkSystem, layer);

		if (m_sGroupId.IsNull())
			return;

		auto slot = SCR_ScenarioFrameworkSlotAI.Cast(layer);
		Tuple1<SCR_ScenarioFrameworkSlotAI> groupContext(slot);
		PersistenceWhenAvailableTask slotGroupTask(OnGroupAvailable, groupContext);
		persistence.WhenAvailable(m_sGroupId, slotGroupTask);
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnGroupAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto group = SCR_AIGroup.Cast(instance);
		if (!group)
			return;

		auto groupContext = Tuple1<SCR_ScenarioFrameworkSlotAI>.Cast(context);
		if (!groupContext.param1)
			return;

		groupContext.param1.m_AIGroup = group;
		if (group.GetCurrentWaypoint())
			groupContext.param1.m_bWaypointsInitialized = true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_sGroupId, UUID.NULL_UUID);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_sGroupId, UUID.NULL_UUID);
		return true;
	}
}

class SCR_ScenarioFrameworkSlotTaskSave : SCR_ScenarioFrameworkSlotSave
{
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnFinished = {};
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnCreated = {};
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnFailed = {};
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnCancelled = {};
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnProgress = {};
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnAssigned = {};

	//------------------------------------------------------------------------------------------------
	override void Read(PersistenceSystem persistence, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Read(persistence, layer);

		auto slot = SCR_ScenarioFrameworkSlotTask.Cast(layer);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, slot.m_aActionsOnFinished, m_aActionsOnFinished);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, slot.m_aActionsOnCreated, m_aActionsOnCreated);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, slot.m_aActionsOnFailed, m_aActionsOnFailed);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, slot.m_aActionsOnCancelled, m_aActionsOnCancelled);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, slot.m_aActionsOnProgress, m_aActionsOnProgress);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, slot.m_aActionsOnAssigned, m_aActionsOnAssigned);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() &&
			m_aActionsOnFinished.IsEmpty() &&
			m_aActionsOnCreated.IsEmpty() &&
			m_aActionsOnFailed.IsEmpty() &&
			m_aActionsOnCancelled.IsEmpty() &&
			m_aActionsOnProgress.IsEmpty() &&
			m_aActionsOnAssigned.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override void Write(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Write(persistence, scenarioFrameworkSystem, layer);

		auto slot = SCR_ScenarioFrameworkSlotTask.Cast(layer);
		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnFinished, slot.m_aActionsOnFinished);
		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnCreated, slot.m_aActionsOnCreated);
		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnFailed, slot.m_aActionsOnFailed);
		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnCancelled, slot.m_aActionsOnCancelled);
		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnProgress, slot.m_aActionsOnProgress);
		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnAssigned, slot.m_aActionsOnAssigned);
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		if (!context.CanSeekMembers() || !m_aActionsOnFinished.IsEmpty())
			context.Write(m_aActionsOnFinished);

		if (!context.CanSeekMembers() || !m_aActionsOnCreated.IsEmpty())
			context.Write(m_aActionsOnCreated);

		if (!context.CanSeekMembers() || !m_aActionsOnFailed.IsEmpty())
			context.Write(m_aActionsOnFailed);

		if (!context.CanSeekMembers() || !m_aActionsOnCancelled.IsEmpty())
			context.Write(m_aActionsOnCancelled);

		if (!context.CanSeekMembers() || !m_aActionsOnProgress.IsEmpty())
			context.Write(m_aActionsOnProgress);

		if (!context.CanSeekMembers() || !m_aActionsOnAssigned.IsEmpty())
			context.Write(m_aActionsOnAssigned);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		if (!context.Read(m_aActionsOnFinished) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aActionsOnCreated) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aActionsOnFailed) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aActionsOnCancelled) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aActionsOnProgress) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aActionsOnAssigned) && !context.CanSeekMembers())
			return false;

		return true;
	}
}

class SCR_ScenarioFrameworkSlotTaskAISave : SCR_ScenarioFrameworkLayerSave
{
	UUID m_sGroupId;

	//------------------------------------------------------------------------------------------------
	override void Read(PersistenceSystem persistence, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Read(persistence, layer);

		auto slot = SCR_ScenarioFrameworkSlotTaskAI.Cast(layer);
		m_sGroupId = persistence.GetId(slot.m_AIGroup);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && m_sGroupId.IsNull();
	}

	//------------------------------------------------------------------------------------------------
	override void Write(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Write(persistence, scenarioFrameworkSystem, layer);

		if (m_sGroupId.IsNull())
			return;

		auto slot = SCR_ScenarioFrameworkSlotTaskAI.Cast(layer);
		Tuple1<SCR_ScenarioFrameworkSlotTaskAI> groupContext(slot);
		PersistenceWhenAvailableTask slotGroupTask(OnGroupAvailable, groupContext);
		persistence.WhenAvailable(m_sGroupId, slotGroupTask);
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnGroupAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto group = SCR_AIGroup.Cast(instance);
		if (!group)
			return;

		auto groupContext = Tuple1<SCR_ScenarioFrameworkSlotTaskAI>.Cast(context);
		if (!groupContext.param1)
			return;

		groupContext.param1.m_AIGroup = group;
		if (group.GetCurrentWaypoint())
			groupContext.param1.m_bWaypointsInitialized = true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_sGroupId, UUID.NULL_UUID);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_sGroupId, UUID.NULL_UUID);
		return true;
	}
}

class SCR_ScenarioFrameworkSlotTriggerSave : SCR_ScenarioFrameworkSlotSave
{
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aTriggerActions = {};

	//------------------------------------------------------------------------------------------------
	override void Read(PersistenceSystem persistence, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Read(persistence, layer);

		auto slot = SCR_ScenarioFrameworkSlotTrigger.Cast(layer);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, slot.m_aTriggerActions, m_aTriggerActions);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && m_aTriggerActions.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override void Write(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem, SCR_ScenarioFrameworkLayerBase layer)
	{
		super.Write(persistence, scenarioFrameworkSystem, layer);

		auto slot = SCR_ScenarioFrameworkSlotTrigger.Cast(layer);
		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aTriggerActions, slot.m_aTriggerActions);
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		if (!context.CanSeekMembers() || !m_aTriggerActions.IsEmpty())
			context.Write(m_aTriggerActions);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		if (!context.Read(m_aTriggerActions) && !context.CanSeekMembers())
			return false;

		return true;
	}
}

class SCR_ScenarioFrameworkLogicSave
{
	string m_sName;
	bool m_bIsTerminated;
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActions = {};

	//------------------------------------------------------------------------------------------------
	void Read(PersistenceSystem persistence, SCR_ScenarioFrameworkLogic logic)
	{
		m_sName = logic.GetName();
		m_bIsTerminated = logic.GetIsTerminated();
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, logic.m_aActions, m_aActions);
	}

	//------------------------------------------------------------------------------------------------
	bool IsDefault()
	{
		return !m_bIsTerminated && m_aActions.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkLogic Write(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem)
	{
		if (m_sName.IsEmpty())
			return null;

		auto logic = SCR_ScenarioFrameworkLogic.Cast(GetGame().GetWorld().FindEntityByName(m_sName));
		if (logic && m_bIsTerminated)
			logic.SetIsTerminated(true);

		SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActions, logic.m_aActions);

		return logic;
	}

	//------------------------------------------------------------------------------------------------
	bool SerializationSave(BaseSerializationSaveContext context)
	{
		context.Write(m_sName);
		context.WriteDefault(m_bIsTerminated, false);

		if (!context.CanSeekMembers() || !m_aActions.IsEmpty())
			context.Write(m_aActions);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool SerializationLoad(BaseSerializationLoadContext context)
	{
		context.Read(m_sName);
		context.ReadDefault(m_bIsTerminated, false);

		if (!context.Read(m_aActions) && !context.CanSeekMembers())
			return false;

		return true;
	}
}

class SCR_ScenarioFrameworkLogicCounterSave : SCR_ScenarioFrameworkLogicSave
{
	int m_iCounterValue;

	//------------------------------------------------------------------------------------------------
	override void Read(PersistenceSystem persistence, SCR_ScenarioFrameworkLogic logic)
	{
		m_iCounterValue = SCR_ScenarioFrameworkLogicCounter.Cast(logic).GetCounterValue();
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && m_iCounterValue == 0;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkLogic Write(PersistenceSystem persistence, SCR_ScenarioFrameworkSystem scenarioFrameworkSystem)
	{
		auto logic = super.Write(persistence, scenarioFrameworkSystem);
		auto counter = SCR_ScenarioFrameworkLogicCounter.Cast(logic);
		if (counter)
			counter.SetCounterValue(m_iCounterValue);

		return logic;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_iCounterValue, 0);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_iCounterValue, 0);
		return true;
	}
}

class SCR_ScenarioFrameworkActionSave
{
	string m_sStoreName;
	int m_iNumberOfActivations;
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aSubActions = {};

	//------------------------------------------------------------------------------------------------
	static SCR_ScenarioFrameworkActionSave GetSaveType(const SCR_ScenarioFrameworkActionBase action)
	{
		if (action.IsInherited(SCR_ScenarioFrameworkActionWaitAndExecute))
			return new SCR_ScenarioFrameworkActionWaitAndExecuteSave();

		if (action.IsInherited(SCR_ScenarioFrameworkActionLimiter))
			return new SCR_ScenarioFrameworkActionLimiterSave();

		if (action.IsInherited(SCR_ScenarioFrameworkActionOnUserActionEvent))
			return new SCR_ScenarioFrameworkActionOnUserActionEventSave();

		return new SCR_ScenarioFrameworkActionSave();
	}

	//------------------------------------------------------------------------------------------------
	static void ReadActions(
		PersistenceSystem persistence,
		notnull array<ref SCR_ScenarioFrameworkActionBase> sourceActions,
		notnull array<ref SCR_ScenarioFrameworkActionSave> savesholder)
	{
		foreach (SCR_ScenarioFrameworkActionBase action : sourceActions)
		{
			auto actionSave = SCR_ScenarioFrameworkActionSave.GetSaveType(action);
			actionSave.Read(persistence, action);
			if (!actionSave.IsDefault())
				savesholder.Insert(actionSave);
		}
	}

	//------------------------------------------------------------------------------------------------
	static void WriteActions(
		PersistenceSystem persistence,
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem,
		notnull array<ref SCR_ScenarioFrameworkActionSave> savesholder,
		notnull array<ref SCR_ScenarioFrameworkActionBase> sourceActions)
	{
		foreach (auto action : savesholder)
		{
			action.Write(persistence, scenarioFrameworkSystem, sourceActions);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Read(const PersistenceSystem persistence, const SCR_ScenarioFrameworkActionBase action)
	{
		m_sStoreName = action.GetStoreName();

		auto subActions = action.GetSubActions();
		if (subActions)
			ReadActions(persistence, subActions, m_aSubActions);

		if (action.m_iMaxNumberOfActivations == -1)
			return; // Consider default

		m_iNumberOfActivations = action.m_iNumberOfActivations;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDefault()
	{
		return m_iNumberOfActivations == 0 && m_aSubActions.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkActionBase Write(
		PersistenceSystem persistence,
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem,
		array<ref SCR_ScenarioFrameworkActionBase> actions)
	{
		foreach (SCR_ScenarioFrameworkActionBase action : actions)
		{
			if (action.GetStoreName() == m_sStoreName)
			{
				action.m_iNumberOfActivations = m_iNumberOfActivations;

				auto subActions = action.GetSubActions();
				if (subActions)
					WriteActions(persistence, scenarioFrameworkSystem, m_aSubActions, subActions);

				return action;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	bool SerializationSave(BaseSerializationSaveContext context)
	{
		context.Write(m_sStoreName);
		context.WriteDefault(m_iNumberOfActivations, 0);

		if (!context.CanSeekMembers() || !m_aSubActions.IsEmpty())
			context.Write(m_aSubActions);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool SerializationLoad(BaseSerializationLoadContext context)
	{
		context.Read(m_sStoreName);
		context.ReadDefault(m_iNumberOfActivations, 0);

		if (!context.Read(m_aSubActions) && !context.CanSeekMembers())
			return false;

		return true;
	}
}

class SCR_ScenarioFrameworkActionWaitAndExecuteSave : SCR_ScenarioFrameworkActionSave
{
	int m_iTimeRemaining = -1;
	string m_EntityId;

	//------------------------------------------------------------------------------------------------
	override void Read(const PersistenceSystem persistence, const SCR_ScenarioFrameworkActionBase action)
	{
		super.Read(persistence, action);

		SCR_ScenarioFrameworkActionWaitAndExecute waitAndExecuteAction = SCR_ScenarioFrameworkActionWaitAndExecute.Cast(action);
		m_iTimeRemaining = SCR_ScenarioFrameworkSystem.GetCallQueuePausable().GetRemainingTime(waitAndExecuteAction.ExecuteActions);
		if (!waitAndExecuteAction.m_Entity)
			return;

		const UUID persistentId = persistence.GetId(waitAndExecuteAction.m_Entity);
		if (!persistentId.IsNull())
		{
			m_EntityId = persistentId;
			return;
		}

		m_EntityId = waitAndExecuteAction.m_Entity.GetName();
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && m_iTimeRemaining == -1;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkActionBase Write(
		PersistenceSystem persistence,
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem,
		array<ref SCR_ScenarioFrameworkActionBase> actions)
	{
		SCR_ScenarioFrameworkActionWaitAndExecute waitAndExecuteAction = SCR_ScenarioFrameworkActionWaitAndExecute.Cast(super.Write(persistence, scenarioFrameworkSystem, actions));
		if (waitAndExecuteAction)
		{
			if (UUID.IsUUID(m_EntityId))
			{
				Tuple2<SCR_ScenarioFrameworkActionWaitAndExecute, int> actionContext(waitAndExecuteAction, m_iTimeRemaining);
				PersistenceWhenAvailableTask entityTask(OnEntityAvailable, actionContext);
				persistence.WhenAvailable(m_EntityId, entityTask);
			}
			else
			{
				IEntity object;
				if (m_EntityId) // There might be a name, for loaded entites or dynamically spawned who are not using persistence itself, but we still want to invoke for.
					object = persistence.GetWorld().FindEntityByName(m_EntityId);

				SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(waitAndExecuteAction.ExecuteActions, m_iTimeRemaining, false, object);
			}
		}
		return waitAndExecuteAction;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnEntityAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto entity = IEntity.Cast(instance);

		auto entityContext = Tuple2<SCR_ScenarioFrameworkActionWaitAndExecute, int>.Cast(context);
		if (!entityContext.param1)
			return;

		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(entityContext.param1.ExecuteActions, entityContext.param2, false, entity);
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_iTimeRemaining, -1);
		context.WriteDefault(m_EntityId, string.Empty);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_iTimeRemaining, -1);
		context.ReadDefault(m_EntityId, string.Empty);
		return true;
	}
}

class SCR_ScenarioFrameworkActionLimiterSave : SCR_ScenarioFrameworkActionSave
{
	float m_fLimitRemaining;

	//------------------------------------------------------------------------------------------------
	override void Read(const PersistenceSystem persistence, const SCR_ScenarioFrameworkActionBase action)
	{
		super.Read(persistence, action);

		SCR_ScenarioFrameworkActionLimiter limiterAction = SCR_ScenarioFrameworkActionLimiter.Cast(action);
		if (!limiterAction.m_LimitedUntil)
			return;

		const WorldTimestamp currentTimestamp = GetGame().GetWorld().GetTimestamp(); // 110
		m_fLimitRemaining = limiterAction.m_LimitedUntil.DiffMilliseconds(currentTimestamp) // 150 - 110 = 40
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && m_fLimitRemaining == 0.0;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkActionBase Write(
		PersistenceSystem persistence,
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem,
		array<ref SCR_ScenarioFrameworkActionBase> actions)
	{
		SCR_ScenarioFrameworkActionLimiter limiterAction = SCR_ScenarioFrameworkActionLimiter.Cast(super.Write(persistence, scenarioFrameworkSystem, actions));
		if (limiterAction)
		{
			limiterAction.m_LimitedUntil = GetGame().GetWorld().GetTimestamp().PlusMilliseconds(m_fLimitRemaining);
		}
		return limiterAction;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_fLimitRemaining, 0.0);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_fLimitRemaining, 0.0);
		return true;
	}
}

class SCR_ScenarioFrameworkActionOnUserActionEventSave : SCR_ScenarioFrameworkActionSave
{
	UUID m_sActionHolderId;
	ref array<UUID> m_aUserEntitiyIds = {};

	//------------------------------------------------------------------------------------------------
	override void Read(const PersistenceSystem persistence, const SCR_ScenarioFrameworkActionBase action)
	{
		super.Read(persistence, action);

		const SCR_ScenarioFrameworkActionOnUserActionEvent listenerAction = SCR_ScenarioFrameworkActionOnUserActionEvent.Cast(action);
		m_sActionHolderId = persistence.GetId(listenerAction.m_ActionHolder);
		if (!listenerAction.m_aUserEntities)
			return;

		foreach (IEntity user : listenerAction.m_aUserEntities)
		{
			const UUID id = persistence.GetId(user);
			if (!id.IsNull())
				m_aUserEntitiyIds.Insert(id);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && m_sActionHolderId.IsNull() && m_aUserEntitiyIds.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkActionBase Write(
		PersistenceSystem persistence,
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem,
		array<ref SCR_ScenarioFrameworkActionBase> actions)
	{
		auto listenerAction = SCR_ScenarioFrameworkActionOnUserActionEvent.Cast(super.Write(persistence, scenarioFrameworkSystem, actions));
		if (listenerAction)
		{
			if (!m_sActionHolderId.IsNull())
			{
				Tuple2<SCR_ScenarioFrameworkActionOnUserActionEvent, bool> actionContext(listenerAction, true);
				PersistenceWhenAvailableTask entityTask(OnEntityAvailable, actionContext);
				persistence.WhenAvailable(m_sActionHolderId, entityTask);
			}

			if (!m_aUserEntitiyIds.IsEmpty())
			{
				Tuple2<SCR_ScenarioFrameworkActionOnUserActionEvent, bool> actionContext(listenerAction, false);
				PersistenceWhenAvailableTask entityTask(OnEntityAvailable, actionContext);
				foreach (UUID user : m_aUserEntitiyIds)
				{
					persistence.WhenAvailable(user, entityTask);
				}
			}
		}
		return listenerAction;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnEntityAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		auto entity = IEntity.Cast(instance);

		auto entityContext = Tuple2<SCR_ScenarioFrameworkActionOnUserActionEvent, bool>.Cast(context);
		if (!entityContext.param1)
			return;

		if (!entityContext.param2)
		{
			entityContext.param1.m_aUserEntities.Insert(entity);
			return;
		}

		BaseActionsManagerComponent actionsManager = BaseActionsManagerComponent.Cast(entity.FindComponent(BaseActionsManagerComponent));
		if (!actionsManager)
			return;

		ScriptedUserAction scriptedUserAction = ScriptedUserAction.Cast(actionsManager.FindAction(entityContext.param1.m_iActionID));
		if (!scriptedUserAction)
			return;

		actionsManager.AddUserActionEventListener(scriptedUserAction, entityContext.param1.OnInvoked);
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		context.WriteDefault(m_sActionHolderId, UUID.NULL_UUID);

		if (!context.CanSeekMembers() || !m_aUserEntitiyIds.IsEmpty())
			context.Write(m_aUserEntitiyIds);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		context.ReadDefault(m_sActionHolderId, UUID.NULL_UUID);

		if (!context.Read(m_aUserEntitiyIds) && !context.CanSeekMembers())
			return false;

		return true;
	}
}

class SCR_ScenarioFrameworkPluginSave
{
	string m_sStoreName;

	//------------------------------------------------------------------------------------------------
	static SCR_ScenarioFrameworkPluginSave GetSaveType(const SCR_ScenarioFrameworkPlugin plugin)
	{
		if (plugin.IsInherited(SCR_ScenarioFrameworkPluginTrigger))
			return new SCR_ScenarioFrameworkPluginTriggerSave();

		if (plugin.IsInherited(SCR_ScenarioFrameworkPluginSpawnPoint))
			return new SCR_ScenarioFrameworkPluginSpawnPointSave();

		if (plugin.IsInherited(SCR_ScenarioFrameworkPluginOnInventoryChange))
			return new SCR_ScenarioFrameworkPluginOnInventoryChangeSave();

		if (plugin.IsInherited(SCR_ScenarioFrameworkPluginOnDestroyEvent))
			return new SCR_ScenarioFrameworkPluginOnDestroyEventSave();

		return new SCR_ScenarioFrameworkPluginSave();
	}

	//------------------------------------------------------------------------------------------------
	void Read(const PersistenceSystem persistence, const SCR_ScenarioFrameworkLayerBase layer, const SCR_ScenarioFrameworkPlugin plugin)
	{
		m_sStoreName = plugin.GetStoreName();
	}

	//------------------------------------------------------------------------------------------------
	bool IsDefault()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkPlugin Write(
		PersistenceSystem persistence,
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem,
		SCR_ScenarioFrameworkLayerBase layer,
		array<ref SCR_ScenarioFrameworkPlugin> plugins)
	{
		foreach (SCR_ScenarioFrameworkPlugin plugin : plugins)
		{
			if (plugin.GetStoreName() == m_sStoreName)
			{
				return plugin;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	bool SerializationSave(BaseSerializationSaveContext context)
	{
		context.Write(m_sStoreName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool SerializationLoad(BaseSerializationLoadContext context)
	{
		context.Read(m_sStoreName);
		return true;
	}
}

class SCR_ScenarioFrameworkPluginTriggerSave : SCR_ScenarioFrameworkPluginSave
{
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aEntityEnteredActions = {};
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aEntityLeftActions = {};
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aFinishedActions = {};

	//------------------------------------------------------------------------------------------------
	override void Read(const PersistenceSystem persistence, const SCR_ScenarioFrameworkLayerBase layer, const SCR_ScenarioFrameworkPlugin plugin)
	{
		super.Read(persistence, layer, plugin);

		const SCR_ScenarioFrameworkPluginTrigger triggerPlugin = SCR_ScenarioFrameworkPluginTrigger.Cast(plugin);

		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, triggerPlugin.m_aEntityEnteredActions, m_aEntityEnteredActions);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, triggerPlugin.m_aEntityLeftActions, m_aEntityLeftActions);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, triggerPlugin.m_aFinishedActions, m_aFinishedActions);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return
			super.IsDefault() &&
			m_aEntityEnteredActions.IsEmpty() &&
			m_aEntityLeftActions.IsEmpty() &&
			m_aFinishedActions.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkPlugin Write(
		PersistenceSystem persistence,
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem,
		SCR_ScenarioFrameworkLayerBase layer,
		array<ref SCR_ScenarioFrameworkPlugin> plugins)
	{
		SCR_ScenarioFrameworkPluginTrigger pluginTrigger = SCR_ScenarioFrameworkPluginTrigger.Cast(super.Write(persistence, scenarioFrameworkSystem, layer, plugins));
		if (pluginTrigger)
		{
			SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aEntityEnteredActions, pluginTrigger.m_aEntityEnteredActions);
			SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aEntityLeftActions, pluginTrigger.m_aEntityLeftActions);
			SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aFinishedActions, pluginTrigger.m_aFinishedActions);
		}
		return pluginTrigger;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		if (!context.CanSeekMembers() || !m_aEntityEnteredActions.IsEmpty())
			context.Write(m_aEntityEnteredActions);

		if (!context.CanSeekMembers() || !m_aEntityLeftActions.IsEmpty())
			context.Write(m_aEntityLeftActions);

		if (!context.CanSeekMembers() || !m_aFinishedActions.IsEmpty())
			context.Write(m_aFinishedActions);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		if (!context.Read(m_aEntityEnteredActions) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aEntityLeftActions) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aFinishedActions) && !context.CanSeekMembers())
			return false;

		return true;
	}
}

class SCR_ScenarioFrameworkPluginSpawnPointSave : SCR_ScenarioFrameworkPluginSave
{
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnSpawnPointUsed = {};

	//------------------------------------------------------------------------------------------------
	override void Read(const PersistenceSystem persistence, const SCR_ScenarioFrameworkLayerBase layer, const SCR_ScenarioFrameworkPlugin plugin)
	{
		super.Read(persistence, layer, plugin);

		const SCR_ScenarioFrameworkPluginSpawnPoint spawnpointPlugin = SCR_ScenarioFrameworkPluginSpawnPoint.Cast(plugin);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, spawnpointPlugin.m_aActionsOnSpawnPointUsed, m_aActionsOnSpawnPointUsed);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && m_aActionsOnSpawnPointUsed.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkPlugin Write(
		PersistenceSystem persistence,
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem,
		SCR_ScenarioFrameworkLayerBase layer,
		array<ref SCR_ScenarioFrameworkPlugin> plugins)
	{
		SCR_ScenarioFrameworkPluginSpawnPoint spawnpointPlugin = SCR_ScenarioFrameworkPluginSpawnPoint.Cast(super.Write(persistence, scenarioFrameworkSystem, layer, plugins));
		if (spawnpointPlugin)
		{
			SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnSpawnPointUsed, spawnpointPlugin.m_aActionsOnSpawnPointUsed);
		}
		return spawnpointPlugin;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		if (!context.CanSeekMembers() || !m_aActionsOnSpawnPointUsed.IsEmpty())
			context.Write(m_aActionsOnSpawnPointUsed);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		if (!context.Read(m_aActionsOnSpawnPointUsed) && !context.CanSeekMembers())
			return false;
		return true;
	}
}

class SCR_ScenarioFrameworkPluginOnInventoryChangeSave : SCR_ScenarioFrameworkPluginSave
{
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnItemAdded = {};
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnItemRemoved = {};

	//------------------------------------------------------------------------------------------------
	override void Read(const PersistenceSystem persistence, const SCR_ScenarioFrameworkLayerBase layer, const SCR_ScenarioFrameworkPlugin plugin)
	{
		super.Read(persistence, layer, plugin);

		const SCR_ScenarioFrameworkPluginOnInventoryChange inventoryChangePlugin = SCR_ScenarioFrameworkPluginOnInventoryChange.Cast(plugin);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, inventoryChangePlugin.m_aActionsOnItemAdded, m_aActionsOnItemAdded);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, inventoryChangePlugin.m_aActionsOnItemRemoved, m_aActionsOnItemRemoved);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return
			super.IsDefault() &&
			m_aActionsOnItemAdded.IsEmpty() &&
			m_aActionsOnItemRemoved.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkPlugin Write(
		PersistenceSystem persistence,
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem,
		SCR_ScenarioFrameworkLayerBase layer,
		array<ref SCR_ScenarioFrameworkPlugin> plugins)
	{
		SCR_ScenarioFrameworkPluginOnInventoryChange inventoryChangePlugin = SCR_ScenarioFrameworkPluginOnInventoryChange.Cast(super.Write(persistence, scenarioFrameworkSystem, layer, plugins));
		if (inventoryChangePlugin)
		{
			SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnItemAdded, inventoryChangePlugin.m_aActionsOnItemAdded);
			SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnItemRemoved, inventoryChangePlugin.m_aActionsOnItemRemoved);
		}
		return inventoryChangePlugin;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		if (!context.CanSeekMembers() || !m_aActionsOnItemAdded.IsEmpty())
			context.Write(m_aActionsOnItemAdded);

		if (!context.CanSeekMembers() || !m_aActionsOnItemRemoved.IsEmpty())
			context.Write(m_aActionsOnItemRemoved);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		if (!context.Read(m_aActionsOnItemAdded) && !context.CanSeekMembers())
			return false;

		if (!context.Read(m_aActionsOnItemRemoved) && !context.CanSeekMembers())
			return false;

		return true;
	}
}

class SCR_ScenarioFrameworkPluginOnDestroyEventSave : SCR_ScenarioFrameworkPluginSave
{
	ref array<ref SCR_ScenarioFrameworkActionSave> m_aActionsOnDestroy = {};

	//------------------------------------------------------------------------------------------------
	override void Read(const PersistenceSystem persistence, const SCR_ScenarioFrameworkLayerBase layer, const SCR_ScenarioFrameworkPlugin plugin)
	{
		super.Read(persistence, layer, plugin);

		const SCR_ScenarioFrameworkPluginOnDestroyEvent destroyEventPlugin = SCR_ScenarioFrameworkPluginOnDestroyEvent.Cast(plugin);
		SCR_ScenarioFrameworkActionSave.ReadActions(persistence, destroyEventPlugin.m_aActionsOnDestroy, m_aActionsOnDestroy);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsDefault()
	{
		return super.IsDefault() && m_aActionsOnDestroy.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkPlugin Write(
		PersistenceSystem persistence,
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem,
		SCR_ScenarioFrameworkLayerBase layer,
		array<ref SCR_ScenarioFrameworkPlugin> plugins)
	{
		SCR_ScenarioFrameworkPluginOnDestroyEvent destroyEventPlugin = SCR_ScenarioFrameworkPluginOnDestroyEvent.Cast(super.Write(persistence, scenarioFrameworkSystem, layer, plugins));
		if (destroyEventPlugin)
		{
			SCR_ScenarioFrameworkActionSave.WriteActions(persistence, scenarioFrameworkSystem, m_aActionsOnDestroy, destroyEventPlugin.m_aActionsOnDestroy);
		}
		return destroyEventPlugin;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationSave(BaseSerializationSaveContext context)
	{
		if (!super.SerializationSave(context))
			return false;

		if (!context.CanSeekMembers() || !m_aActionsOnDestroy.IsEmpty())
			context.Write(m_aActionsOnDestroy);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool SerializationLoad(BaseSerializationLoadContext context)
	{
		if (!super.SerializationLoad(context))
			return false;

		if (!context.Read(m_aActionsOnDestroy) && !context.CanSeekMembers())
			return false;
		return true;
	}
}
