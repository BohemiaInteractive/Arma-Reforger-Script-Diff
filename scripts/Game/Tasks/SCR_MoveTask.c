[EntityEditorProps(category: "GameScripted/Tasks", description: "Move task.", color: "0 0 255 255")]
class SCR_MoveTaskClass: SCR_AttachableTaskClass
{
}

class SCR_MoveTask : SCR_AttachableTask
{	
	[Attribute("5")]
	protected float m_fMaxDistance;
	
	//------------------------------------------------------------------------------------------------
	void PeriodicalCheck()
	{		
		if (!SCR_TaskSystem.GetInstance())
			return;
		
		array<ref SCR_TaskExecutor> assignees = GetTaskAssignees();		
		
		SCR_TaskExecutorPlayer playerExecutor;
		IEntity controlledEntity;
		foreach (SCR_TaskExecutor assignee : assignees)
		{
			playerExecutor = SCR_TaskExecutorPlayer.Cast(assignee);
			if (!playerExecutor)
				continue;
			
			int id = playerExecutor.GetPlayerID();
			controlledEntity = SCR_PossessingManagerComponent.GetPlayerMainEntity(id);
			if (!controlledEntity)
				continue;
			
			float distance = vector.Distance(controlledEntity.GetOrigin(), GetOrigin());			
			if (distance < m_fMaxDistance)
			{
				SetTaskState(SCR_ETaskState.COMPLETED);
				return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetTaskCompletionType(EEditorTaskCompletionType newTaskCompletionType)
	{
		if (m_iTaskCompletionType == newTaskCompletionType)
			return;
		
		super.SetTaskCompletionType(newTaskCompletionType);
		
		if (Replication.IsClient())
			return;
		
		if (m_iTaskCompletionType == EEditorTaskCompletionType.AUTOMATIC)
			SetEventMask(EntityEvent.FRAME);
		else 
			ClearEventMask(EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		PeriodicalCheck();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MoveTask(IEntitySource src, IEntity parent)
	{
		if (SCR_Global.IsEditMode(this))
			return;
		
		if (Replication.IsClient())
			return;
		
		if (m_iTaskCompletionType == EEditorTaskCompletionType.AUTOMATIC)
			SetEventMask(EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_MoveTask()
	{
		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
			return;
		
		if (Replication.IsClient())
			return;
		
		if (m_iTaskCompletionType == EEditorTaskCompletionType.AUTOMATIC)
			ClearEventMask(EntityEvent.FRAME);
	}

}