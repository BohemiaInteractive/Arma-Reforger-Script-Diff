[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotClearAreaClass : SCR_ScenarioFrameworkSlotTaskClass
{
}

class SCR_ScenarioFrameworkSlotClearArea : SCR_ScenarioFrameworkSlotTask
{
	[Attribute(desc: "Ignored Faction Keys that won't be used for any calculations for this Slot Clear", category: "Asset")]
	protected ref array<FactionKey> m_aIgnoredFactionKeys;

	protected bool m_bPreviousTriggerState;

	//------------------------------------------------------------------------------------------------
	//! Initializes trigger entities, disables periodic queries, and sets init sequence done to false.
	override void FinishInit()
	{
		BaseGameTriggerEntity trigger = BaseGameTriggerEntity.Cast(m_Entity);
		if (trigger)
		{
			m_bPreviousTriggerState = trigger.IsPeriodicQueriesEnabled();
			trigger.EnablePeriodicQueries(false);

			SCR_FactionControlTriggerEntity factionTrigger = SCR_FactionControlTriggerEntity.Cast(trigger);
			if (factionTrigger && m_aIgnoredFactionKeys)
				factionTrigger.AddIgnoredFactionKeys(m_aIgnoredFactionKeys);

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
	//! Initializes plugins, actions, and triggers after parent area children spawned.
	//! \param[in] layer for which this is called.
	protected void AfterParentAreaChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}

		InitActivationActions();

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
