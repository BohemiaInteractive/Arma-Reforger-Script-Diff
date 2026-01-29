[BaseContainerProps()]
class SCR_CustomTriggerConditionsSpecificPrefabCount : SCR_CustomTriggerConditions
{
	[Attribute(desc: "Which Prefabs and if their children will be accounted for", category: "Trigger")]
	ref array<ref SCR_ScenarioFrameworkPrefabFilterCount> m_aPrefabFilter;

	//------------------------------------------------------------------------------------------------
	//! Initializes scenario trigger entity with prefab filters based on prefab filter count data.
	//! \param[in] entity Trigger representing prefab filter conditions for scenario event activation.
	override void Prepare(IEntity entity)
	{
		SCR_ScenarioFrameworkTriggerEntity trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(entity);
		if (!trigger)
			return;
		
		Resource resource;
		BaseContainer baseContainer;
		PrefabFilter prefabFilter;
		foreach (SCR_ScenarioFrameworkPrefabFilterCount prefabFilterCount : m_aPrefabFilter)
		{
			prefabFilter = new PrefabFilter();
			prefabFilter.SetPrefab(prefabFilterCount.m_sSpecificPrefabName);
			prefabFilter.SetCheckPrefabHierarchy(prefabFilterCount.m_bIncludeChildren);
			trigger.AddPrefabFilter(prefabFilter);

			resource = Resource.Load(prefabFilterCount.m_sSpecificPrefabName);
			if (!resource.IsValid())
				continue;

			prefabFilterCount.m_Resource = resource;
			prefabFilterCount.m_PrefabContainer = resource.GetResource().ToBaseContainer();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Checks how many times the specific prefab (Using the BaseContainer) is present inside the trigger and sets trigger conditions accordingly
	//! \param[in] entity
	override bool Init(IEntity entity)
	{
		SCR_ScenarioFrameworkTriggerEntity trigger = SCR_ScenarioFrameworkTriggerEntity.Cast(entity);
		if (!trigger)
			return true;
		
		bool triggerStatus;
		foreach (SCR_ScenarioFrameworkPrefabFilterCount prefabFilter : m_aPrefabFilter)
		{
			if (trigger.GetSpecificPrefabCountInsideTrigger(prefabFilter.m_PrefabContainer, prefabFilter.m_iPrefabCount, prefabFilter.m_bIncludeChildren) >= prefabFilter.m_iPrefabCount)
			{
				triggerStatus = true;
			}
			else
			{
				triggerStatus = false;
				break;
			}
		}

		return triggerStatus;
	}
}