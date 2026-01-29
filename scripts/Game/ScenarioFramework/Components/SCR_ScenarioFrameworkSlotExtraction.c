[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotExtractionClass : SCR_ScenarioFrameworkSlotTaskClass
{
}

class SCR_ScenarioFrameworkSlotExtraction : SCR_ScenarioFrameworkSlotTask
{
	//------------------------------------------------------------------------------------------------
	//! Initializes scenario trigger entity, sets its owner faction, then calls base class FinishInit method.
	override void FinishInit()
	{
		SCR_ScenarioFrameworkTriggerEntity trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(m_Entity);
		if (trigger)
			trigger.SetOwnerFaction(m_sFactionKey);
		
		super.FinishInit();
	}
}
