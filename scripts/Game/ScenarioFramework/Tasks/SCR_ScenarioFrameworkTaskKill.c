class SCR_TaskKillClass: SCR_ScenarioFrameworkTaskClass
{
};

class SCR_TaskKill : SCR_ScenarioFrameworkTask
{	
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state Checks if object is not destroyed, has asset and support entity, then removes self from damage state change event and finishes the task
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED || !m_Asset)
			return;
		
		SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Asset);
		if (objectDmgManager)
	 		objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);
				
		m_LayerTask.ProcessLayerTaskState(SCR_ETaskState.COMPLETED);			
	}
	
	//------------------------------------------------------------------------------------------------
	void OnGroupEmpty(AIGroup group)
	{
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(group);
		if (!aiGroup)
			return;
		
		aiGroup.GetOnEmpty().Remove(OnGroupEmpty);
		m_LayerTask.ProcessLayerTaskState(SCR_ETaskState.COMPLETED);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Rehooks task asset, gets damage manager, and subscribes to damage state changes.
	//! \param[in] object Rehooks task asset, gets damage manager, adds damage state change listener.
	override void HookTaskAsset(IEntity object)
	{
		if (!object)
			return;
		
		super.HookTaskAsset(object);
		
		SCR_AIGroup group = SCR_AIGroup.Cast(m_Asset);
		if (!group)
		{
			SCR_DamageManagerComponent objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(m_Asset);
			if (objectDmgManager)
				objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
		}
		else
		{
			group.GetOnEmpty().Insert(OnGroupEmpty);
		}	
	}
}
