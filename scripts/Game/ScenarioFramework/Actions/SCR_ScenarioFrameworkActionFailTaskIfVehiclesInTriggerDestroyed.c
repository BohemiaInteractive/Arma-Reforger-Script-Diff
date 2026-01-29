[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionFailTaskIfVehiclesInTriggerDestroyed : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Trigger Getter")]
	ref SCR_ScenarioFrameworkGet		m_Getter;

	[Attribute(desc: "Target Layer Task. If this is attached to the Layer Task you intend to work with, you can leave it empty")]
	string m_sTargetLayerTask;

	[Attribute(defvalue: "false", desc: "If set to true, it will fail the task only when player caused the destruction")]
	bool m_bCausedByPlayer;

	bool m_bAlreadyDestroyed;
	SCR_ScenarioFrameworkTask m_Task;
	ref array<IEntity> m_aTargetEntities = {};

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] object
	//! \param[out] trigger
	//! \return
	bool CanActivateTriggerVariant(IEntity object, out SCR_ScenarioFrameworkTriggerEntity trigger)
	{
		trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(object);
		if (m_iMaxNumberOfActivations != -1 && m_iNumberOfActivations >= m_iMaxNumberOfActivations)
		{
			if (trigger)
			{
				trigger.GetOnActivate().Remove(OnActivate);
				trigger.GetOnDeactivate().Remove(OnActivate);
			}

			return false;
		}

		m_iNumberOfActivations++;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] object
	//! \param[in] trigger
	void AddListener(IEntity object = null, SCR_ScenarioFrameworkTriggerEntity trigger = null)
	{
		if (!object)
			return;

		if (!trigger)
			return;

		SCR_ScenarioFrameworkLayerTask layerTask = SCR_ScenarioFrameworkLayerTask.Cast(object.FindComponent(SCR_ScenarioFrameworkLayerTask));
		if (!layerTask)
			return;

		m_Task = layerTask.GetTask();
		if (!m_Task)
			return;

		array<IEntity> entitesInside = {};
		trigger.GetEntitiesInside(entitesInside);
		if (entitesInside.IsEmpty())
			return;

		foreach (IEntity entity : entitesInside)
		{
			if (!Vehicle.Cast(entity))
				continue;

			SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(entity);
			if (objectDmgManager)
			{
				m_aTargetEntities.Insert(entity);
				objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || m_bAlreadyDestroyed)
			return;

		if (m_bCausedByPlayer)
		{
			bool destroyedByPlayer;
			foreach (IEntity entity : m_aTargetEntities)
			{
				SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(entity);
				if (!objectDmgManager)
					continue;

				if (objectDmgManager.GetState() == EDamageState.DESTROYED)
				{

					Instigator instigator = objectDmgManager.GetInstigator();
					if (!instigator)
						continue;

					InstigatorType instigatorType = instigator.GetInstigatorType();
					if (instigatorType && instigatorType == InstigatorType.INSTIGATOR_PLAYER)
					{
						destroyedByPlayer = true;
						break;
					}
				}
			}

			if (!destroyedByPlayer)
				return;
		}

		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		m_bAlreadyDestroyed = true;
		if (!m_Task)
			return;

		SCR_ScenarioFrameworkLayerTask layerTask = m_Task.GetLayerTask();
		if (!layerTask)
			return;

		layerTask.ProcessLayerTaskState(SCR_ETaskState.FAILED);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!m_sTargetLayerTask.IsEmpty())
		{
			IEntity entity = GetGame().GetWorld().FindEntityByName(m_sTargetLayerTask);
			if (entity)
				object = entity;
		}

		SCR_ScenarioFrameworkTriggerEntity trigger;
		if (!CanActivateTriggerVariant(object, trigger))
			return;

		if (trigger)
		{
			AddListener(object, trigger);
			return;
		}

		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Getter not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity entityFrom = entityWrapper.GetValue();
		if (!entityFrom)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;

		}

		trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(entityFrom);
		if (trigger)
			AddListener(object, trigger);
	}
}
