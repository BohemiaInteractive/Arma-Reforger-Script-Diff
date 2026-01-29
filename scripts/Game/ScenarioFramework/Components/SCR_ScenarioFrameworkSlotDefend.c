[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotDefendClass : SCR_ScenarioFrameworkSlotTaskAIClass
{
}

class SCR_ScenarioFrameworkSlotDefend : SCR_ScenarioFrameworkSlotTaskAI
{
	//------------------------------------------------------------------------------------------------
	//! Dynamically despawns this layer.
	//! \param[in] layer Layer represents the scenario framework layer where dynamic despawning occurs.
	override void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
		GetOnAllChildrenSpawned().Remove(DynamicDespawn);
		if (!m_Entity && !SCR_StringHelper.IsEmptyOrWhiteSpace(m_sObjectToSpawn))
		{
			GetOnAllChildrenSpawned().Insert(DynamicDespawn);
			return;
		}
		
		if (!m_bInitiated || m_bExcludeFromDynamicDespawn)
			return;
		
		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		if (m_Entity)
			m_vPosition = m_Entity.GetOrigin();
		
		m_aSpawnedEntities.RemoveItem(null);
		foreach (IEntity entity : m_aSpawnedEntities)
		{
			m_vPosition = entity.GetOrigin();
			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}
		
		m_aSpawnedEntities.Clear();
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_ScenarioFrameworkSlotDefend(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
#ifdef WORKBENCH		
		m_iDebugShapeColor = ARGB(100, 0x00, 0x10, 0xFF);
#endif		
	}
}
