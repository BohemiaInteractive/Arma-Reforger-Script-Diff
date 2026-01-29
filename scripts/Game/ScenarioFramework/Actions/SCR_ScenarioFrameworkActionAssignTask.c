[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAssignTask : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Which task to work with - Use GetTask")]
	ref SCR_ScenarioFrameworkGetTask m_TaskGetter;
	
	[Attribute(desc: "Getter for player, or array of players, to assign task.")];
	ref SCR_ScenarioFrameworkGet m_PlayerGetter;
	
	[Attribute(defvalue: "1", desc: "Assign Task")]
	bool m_bAssignTask;
	
	[Attribute(defvalue: "1", desc: "Force Assign")]
	bool m_bForceAssign;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		//First get task
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_TaskGetter.Get());
		if (!entityWrapper)
		{
			PrintFormat("ScenarioFramework Action: Issue with Task Getter detected for Action %1.", this, level:LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkTask task = SCR_ScenarioFrameworkTask.Cast(entityWrapper.GetValue());
		if (!task)
		{
			if (object)
					PrintFormat("ScenarioFramework Action: Task not found for Action %1 attached on %2.", this, object.GetName(), level:LogLevel.ERROR);
				else
					PrintFormat("ScenarioFramework Action: Task not found for Action %1.", this, level:LogLevel.ERROR);

			return;
		}
		
		//Get player, or array of players to assign task
		entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_PlayerGetter.Get());
		if (!entityWrapper)
		{
			SCR_ScenarioFrameworkParam<array<IEntity>> entityArrayWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_PlayerGetter.Get());
			if (!entityArrayWrapper)
			{
				Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
				return;
			}
				
			array<IEntity> entities = {};
			entities = entityArrayWrapper.GetValue();
				
			if (entities.IsEmpty())
			{
				Print(string.Format("ScenarioFramework Action: Array Getter for %1 has no elements to work with.", this), LogLevel.ERROR);
				return;
			}
				
			foreach (IEntity entity : entities)
			{
				ProcessPlayer(entity, task);
			}
				
			return;
		}
			
		IEntity entity = entityWrapper.GetValue();
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		ProcessPlayer(entity, task);
	}
	
	//------------------------------------------------------------------------------------------------
	void ProcessPlayer(notnull IEntity playerEnt, notnull SCR_ScenarioFrameworkTask task)
	{
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;
		
		SCR_TaskExecutor executor = SCR_TaskExecutor.FromEntity(playerEnt);
		if (!executor)
			return;
		
		if (m_bAssignTask)
			taskSystem.AssignTask(task, executor, m_bForceAssign);
		else
			taskSystem.UnassignTask(task, executor);
	}
}