[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotPickClass : SCR_ScenarioFrameworkSlotTaskClass
{
}

class SCR_ScenarioFrameworkSlotPick : SCR_ScenarioFrameworkSlotTask
{
	//TODO: make title and description as Tuple2
	[Attribute(desc: "Name of the task in list of tasks (item picked up )", category: "Task")]		
	protected string 		m_sTaskTitleUpdated1;
	
	[Attribute(desc: "Description of the task (item picked up)", category: "Task",)]			//TODO: make config, memory
	protected string 		m_sTaskDescriptionUpdated1;
	
	[Attribute(desc: "Name of the task in list of tasks (item dropped)", category: "Task")]		
	protected string 		m_sTaskTitleUpdated2;
	
	[Attribute(desc: "Description of the task (item dropped)", category: "Task",)]
	protected string 		m_sTaskDescriptionUpdated2;
	
	//------------------------------------------------------------------------------------------------
	//! Task title getter with state-dependent titles.
	//! \param[in] iState iState represents the current state of the task, used for differentiating between various stages of the task's progression
	//! \return different task titles based on state.
	override string GetTaskTitle(int iState = 0) 
	{ 
		if (iState == 0)
			return super.GetTaskTitle();
		else if (iState == 1)
			return m_sTaskTitleUpdated1;	
		else if (iState == 2)
			return m_sTaskTitleUpdated2;
		else if (iState == 4)
			return super.GetTaskTitle();
		else if (iState == 5)
			return m_sTaskTitleUpdated1;
		else if (iState == 6)
			return m_sTaskTitleUpdated2; 

		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes dynamic despawning if not already initiated, excludes if marked for exclusion, sets dynamic despawn flag true
	//! \param[in] layer for which this is called.
	override void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
		if (!m_bInitiated || m_bExcludeFromDynamicDespawn)
			return;
		
		m_bDynamicallyDespawned = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns different descriptions based on state, defaulting to superclass description if no state-specific description is found.
	//! \param[in] iState Task state representation for description updates.
	//! \return different descriptions based on state parameter.
	override string GetTaskDescription(int iState = 0)	
	{ 
		if (iState == 0)
			return super.GetTaskDescription();
		else if (iState == 1)
			return m_sTaskDescriptionUpdated1;	
		else if (iState == 2)
			return m_sTaskDescriptionUpdated2;
		else if (iState == 4)
			return super.GetTaskDescription();
		else if (iState == 5)
			return m_sTaskDescriptionUpdated1;
		else if (iState == 6)
			return m_sTaskDescriptionUpdated2;

		return string.Empty;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Updates blacklist for garbage system when all children spawned in scenario layer.
	//! \param[in] layer Layer represents scenario framework layer, used for spawning entities in the game world.
	override void AfterAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		super.AfterAllChildrenSpawned(this);
		
		if (!m_Entity || m_bCanBeGarbageCollected)
			return;

		auto garbageSystem = SCR_GarbageSystem.GetByEntityWorld(m_Entity);
		if (garbageSystem)
			garbageSystem.UpdateBlacklist(m_Entity, true);
	}
}
