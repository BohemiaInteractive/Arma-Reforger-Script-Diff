[EntityEditorProps(category: "GameScripted/ScenarioFramework/Layer", description: "")]
class SCR_ScenarioFrameworkLayerTaskDeliverClass : SCR_ScenarioFrameworkLayerTaskClass
{
}

class SCR_ScenarioFrameworkLayerTaskDeliver : SCR_ScenarioFrameworkLayerTask
{	
	[Attribute(desc: "Name of the task in list of tasks after item is possessed.", category: "Task")]		
	protected string 		m_sTaskTitleUpdated;
	
	[Attribute(desc: "Description of the task after item is possessed", category: "Task")]			//TODO: make config, memory
	protected string 		m_sTaskDescriptionUpdated;
	
	[Attribute(defvalue: "30", desc: "Delay in seconds for Intel map marker when it is dropped on the ground", params: "0 6000 1", category: "Task")]
	protected int 			m_iIntelMapMarkerUpdateDelay;
	
	protected IEntity		m_DeliverPoint;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] entity Sets delivery point entity.
	void SetDeliveryPointEntity(IEntity entity)
	{
		m_DeliverPoint = entity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Delivery point entity representing a location for delivery in the game world.
	IEntity GetDeliveryPointEntity()
	{
		return m_DeliverPoint;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Delay in seconds between intel map marker updates.
	int GetIntelMapMarkerUpdateDelay()
	{
		return m_iIntelMapMarkerUpdateDelay;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Restores default values, nullifies deliver point, calls base method.
	//! \param[in] includeChildren Restores default settings, optionally including children objects.
	//! \param[in] reinitAfterRestoration Restores object state after default settings, optionally reinitializes after restoration.
	//! \param[in] affectRandomization Affects randomization during default restoration.
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false, bool affectRandomization = true)
	{
		m_DeliverPoint = null;
		
		super.RestoreToDefault(includeChildren, reinitAfterRestoration, affectRandomization);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets up delivery trigger for task at deliver point.
	override void SetupTask()
	{
		super.SetupTask();
		
		if (!m_Task || !m_DeliverPoint)
			return;
		
		SCR_TaskDeliver.Cast(m_Task).SetDeliveryTrigger(SCR_BaseTriggerEntity.Cast(m_DeliverPoint));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates task title and description if provided.
	void UpdateTaskTitleAndDescription()
	{
		if (!m_sTaskTitleUpdated.IsEmpty()) 
			m_Task.SetTaskName(m_sTaskTitleUpdated);
		if (!m_sTaskDescriptionUpdated.IsEmpty()) 
			m_Task.SetTaskDescription(m_sTaskDescriptionUpdated);
	}
}
