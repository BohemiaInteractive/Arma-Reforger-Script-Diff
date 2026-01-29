[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionBase : BaseContainerObject
{
	[Attribute(desc: "If set to true, when this action gets activated, it will break the breakpoint in the Script Editor in CanActivate method from which you can step out to the OnActivate method and debug this specific action. This can be also set during runtime via Debug Menu > ScenarioFramework > Action Inspector")]
	bool m_bDebug;
	
	[Attribute(defvalue: "-1", uiwidget: UIWidgets.Graph, params: "-1 10000 1", desc: "How many times this action can be performed if this gets triggered? Value -1 for infinity")]
	int	m_iMaxNumberOfActivations;

	IEntity	m_Entity;
	int	m_iNumberOfActivations;

	//------------------------------------------------------------------------------------------------
	//! Initializes entity, sets OnActivate and OnDeactivate event handlers for SCR_BaseTriggerEntity.
	//! \param[in] entity that initialized this action.
	void Init(IEntity entity)
	{
		m_Entity = entity;
		
		if (!SCR_BaseTriggerEntity.Cast(entity))
		{
			OnActivate(entity);
			return;
		}

		ScriptInvoker pOnActivateInvoker = SCR_BaseTriggerEntity.Cast(entity).GetOnActivate();
		if (pOnActivateInvoker)
			pOnActivateInvoker.Insert(OnActivate);

		ScriptInvoker pOnDeactivateInvoker = SCR_BaseTriggerEntity.Cast(entity).GetOnDeactivate();
		if (pOnDeactivateInvoker)
			pOnDeactivateInvoker.Insert(OnActivate);		//registering OnDeactivate to OnActivate - we need both changes
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if action can be activated, handles maximum activation limit, increments activation count if possible
	//! \return true if action can be activated, false otherwise.
	bool CanActivate()
	{
		// Here you can step out to the OnActivate method and debug this specific action if m_bDebug attribute is set to true.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Action Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkActionBase.CanActivate] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		if (m_iMaxNumberOfActivations != -1 && m_iNumberOfActivations >= m_iMaxNumberOfActivations)
		{
			if (m_bDebug)
			{
				if (m_Entity)
					Print(string.Format("ScenarioFramework Action: Maximum number of activations reached for Action %1 attached on %2. Action won't do anything.", this, m_Entity.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Maximum number of activations reached for Action %1. Action won't do anything.", this), LogLevel.ERROR);
			}

			return false;
		}

		m_iNumberOfActivations++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Validates input entity for ScenarioFramework Action, checks if it's from layer or getter, returns true if valid
	//! \param[in] object Validates if input object is a valid entity for the action
	//! \param[in] getter Getter represents an optional parameter used to retrieve an entity from a scenario framework action.
	//! \param[out] entity that is valid for this action
	//! \return true if the input entity is valid, otherwise false.
	bool ValidateInputEntity(IEntity object, SCR_ScenarioFrameworkGet getter, out IEntity entity)
	{
		if (!getter && object)
		{
			SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				Print(string.Format("ScenarioFramework Action: Action %1 attached on %2 is not called from layer and won't do anything.", this, object.GetName()), LogLevel.ERROR);
				return false;
			}

			entity = layer.GetSpawnedEntity();
		}
		else
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(getter.Get());
			if (!entityWrapper)
			{
				if (object)
					Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
				else
					Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);

				return false;
			}

			entity = entityWrapper.GetValue();
		}

		if (!entity)
		{
			if (object)
				Print(string.Format("ScenarioFramework Action: Entity not found for Action %1 attached on %2.", this, object.GetName()), LogLevel.ERROR);
			else
				Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);

			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] object
	void OnActivate(IEntity object);
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] subActions
	array<ref SCR_ScenarioFrameworkActionBase> GetSubActions();
	
	//------------------------------------------------------------------------------------------------
	//! Restores default settings for action and resets number of activations.
	void RestoreToDefault()
	{
		m_iNumberOfActivations = 0;
		array<ref SCR_ScenarioFrameworkActionBase> subActions = GetSubActions();
		if (subActions && !subActions.IsEmpty())
		{
			foreach (SCR_ScenarioFrameworkActionBase subAction : subActions)
			{
				subAction.RestoreToDefault();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns objects from an array, checks for existence, activation type, and initializes them with specified activation type.
	//! \param[in] aObjectsNames An array of object names for spawning in the scenario.
	//! \param[in] eActivationType eActivationType represents the activation type for the objects being spawned in the scenario.
	void SpawnObjects(notnull array<string> aObjectsNames, SCR_ScenarioFrameworkEActivationType eActivationType)
	{
		IEntity object;
		SCR_ScenarioFrameworkLayerBase layer;

		foreach (string sObjectName : aObjectsNames)
		{
			object = GetGame().GetWorld().FindEntityByName(sObjectName);
			if (!object)
			{
				Print(string.Format("ScenarioFramework Action: Can't spawn object set in slot %1. Slot doesn't exist", sObjectName), LogLevel.ERROR);
				continue;
			}

			layer = SCR_ScenarioFrameworkLayerBase.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (!layer)
			{
				Print(string.Format("Can't spawn object %1 - the slot doesn't have SCR_ScenarioFrameworkLayerBase component", sObjectName), LogLevel.ERROR);
				continue;
			}
			
			if (layer.GetActivationType() != eActivationType)
			{
				Print(string.Format("Can't spawn object %1 - the slot has %2 activation type set instead of %3", sObjectName, layer.GetActivationType(), eActivationType), LogLevel.ERROR);
				continue;
			}

			layer.Init(null, eActivationType);
			layer.SetActivationType(SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
		}
	}
}