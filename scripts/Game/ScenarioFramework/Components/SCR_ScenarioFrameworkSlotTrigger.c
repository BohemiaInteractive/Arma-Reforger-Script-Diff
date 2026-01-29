[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotTriggerClass : SCR_ScenarioFrameworkSlotBaseClass
{
}

class SCR_ScenarioFrameworkSlotTrigger : SCR_ScenarioFrameworkSlotBase
{
	[Attribute(desc: "Actions that will be performed after trigger conditions are true and the trigger itself activates (not the slot itself)", category: "OnActivation")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aTriggerActions;

	protected bool m_bPreviousTriggerState;

	//------------------------------------------------------------------------------------------------
	//! Restores default settings for trigger actions, then calls base class method.
	//! \param[in] includeChildren Restores default settings for all child objects, if includeChildren is true.
	//! \param[in] reinitAfterRestoration Resets object state after restoration.
	//! \param[in] affectRandomization Affects randomization state during restoration.
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false, bool affectRandomization = true)
	{
		foreach (SCR_ScenarioFrameworkActionBase activationAction : m_aTriggerActions)
		{
			activationAction.RestoreToDefault();
		}

		super.RestoreToDefault(includeChildren, reinitAfterRestoration, affectRandomization);
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes trigger entities, disables periodic queries, and sets init sequence done to false.
	override void FinishInit()
	{
		BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(m_Entity);
		if (trigger)
		{
			m_bPreviousTriggerState = trigger.IsPeriodicQueriesEnabled();
			trigger.EnablePeriodicQueries(false);

			SCR_ScenarioFrameworkTriggerEntity frameworkTrigger = SCR_ScenarioFrameworkTriggerEntity.Cast(trigger);
			if (frameworkTrigger)
				frameworkTrigger.SetInitSequenceDone(false);
		}

		super.FinishInit();
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes scenario layer, checks parent layer, sets area, and removes self from onAllChildrenSpawned list.
	//! \param[in] layer for which this is called.
	override void AfterAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		m_bInitiated = true;

		if (m_ParentLayer)
			m_ParentLayer.CheckAllChildrenSpawned(this);

		if (!m_Area)
			m_Area = GetParentArea();

		if (m_Area)
		{
			m_Area.GetOnAllChildrenSpawned().Insert(AfterParentAreaChildrenSpawned);
			m_Area.CheckAllChildrenSpawned(this);
		}

		GetOnAllChildrenSpawned().Remove(AfterAllChildrenSpawned);
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes plugins, actions, and triggers, checks for repeated spawn timer, removes event handler, enables periodic queries,
	//! \param[in] layer Initializes plugins, actions, and triggers for scenario framework layer after parent area children spawned.
	protected void AfterParentAreaChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}

		InitActivationActions();

		foreach (SCR_ScenarioFrameworkActionBase triggerAction : m_aTriggerActions)
		{
			triggerAction.Init(m_Entity);
		}

		if (m_fRepeatedSpawnTimer >= 0)
			RepeatedSpawn();

		if (m_Area)
			m_Area.GetOnAllChildrenSpawned().Remove(AfterParentAreaChildrenSpawned);

		if (m_Entity)
		{
			BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(m_Entity);
			if (trigger)
			{
				trigger.EnablePeriodicQueries(m_bPreviousTriggerState);

				SCR_ScenarioFrameworkTriggerEntity frameworkTrigger = SCR_ScenarioFrameworkTriggerEntity.Cast(trigger);
				if (frameworkTrigger)
					frameworkTrigger.SetInitSequenceDone(true);
			}
		}
	}
}
