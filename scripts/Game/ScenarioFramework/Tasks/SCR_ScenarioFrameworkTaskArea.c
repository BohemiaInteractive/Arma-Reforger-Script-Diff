class SCR_ScenarioFrameworkTaskAreaClass: SCR_ScenarioFrameworkTaskClass
{
};

class SCR_ScenarioFrameworkTaskArea : SCR_ScenarioFrameworkTask
{	
	protected SCR_BaseTriggerEntity 		m_Trigger;
	
	//------------------------------------------------------------------------------------------------
	//! Registers trigger after rehooking task asset.
	//! \param[in] object that should be a Trigger
	override void HookTaskAsset(IEntity object)
	{
		if (!object)
			return;
		
		super.HookTaskAsset(object);
		
		RegisterTrigger();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Registers trigger entity for task area, handles missing trigger or support entity errors.
	void RegisterTrigger()
	{
		m_Trigger = SCR_BaseTriggerEntity.Cast(m_Asset);
		if (!m_Trigger)
		{
			m_LayerTask.ProcessLayerTaskState(SCR_ETaskState.CANCELLED);
	 		return;
		}
		m_Trigger.GetOnActivate().Remove(OnTriggerActivated);
		m_Trigger.GetOnActivate().Insert(OnTriggerActivated);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Finishes current task for support entity on trigger activation.
	void OnTriggerActivated()
	{
		m_Trigger.GetOnActivate().Remove(OnTriggerActivated);
		m_LayerTask.ProcessLayerTaskState(SCR_ETaskState.COMPLETED);
	}	
}
