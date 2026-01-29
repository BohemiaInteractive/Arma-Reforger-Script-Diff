[EntityEditorProps(category: "GameScripted/Tasks", description: "")]
class SCR_TriggerTaskClass : SCR_AttachableTaskClass
{
}

class SCR_TriggerTask : SCR_AttachableTask
{
	[Attribute(desc: "When enabled, task will fail upon activation instead of being completed.", category: "Trigger Task")]
	protected bool m_bToFail;
	
	[Attribute(desc: "When enabled, task will change state when its trigger is deactivated instead of activated.", category: "Trigger Task")]
	protected bool m_bOnTriggerDeactivate;
	
	protected SCR_BaseFactionTriggerEntity m_Trigger;
	
	//------------------------------------------------------------------------------------------------
	protected void OnTriggerActivate()
	{
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;
		
		if (m_iTaskCompletionType != EEditorTaskCompletionType.AUTOMATIC)
			return;
		
		if (!m_bOnTriggerDeactivate)
		{
			if (m_bToFail)
				taskSystem.SetTaskState(this, SCR_ETaskState.FAILED);
			else
				taskSystem.SetTaskState(this, SCR_ETaskState.COMPLETED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTriggerDeactivate()
	{
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;
		
		if (m_iTaskCompletionType != EEditorTaskCompletionType.AUTOMATIC)
			return;
		
		if (m_bOnTriggerDeactivate)
		{
			if (m_bToFail)
				taskSystem.SetTaskState(this, SCR_ETaskState.FAILED);
			else
				taskSystem.SetTaskState(this, SCR_ETaskState.COMPLETED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void AddOwnerFactionKey(FactionKey factionKey)
	{
		super.AddOwnerFactionKey(factionKey);
		
		if (m_Trigger)
			m_Trigger.AddOwnerFaction(factionKey);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (Replication.IsServer())
		{
			IEntity child = GetChildren();
			while (child)
			{
				m_Trigger = SCR_BaseFactionTriggerEntity.Cast(child);
				if (m_Trigger)
					break;
				else
					child = child.GetSibling();
			}
			
			if (m_Trigger)
			{
				m_Trigger.GetOnActivate().Insert(OnTriggerActivate);
				m_Trigger.GetOnDeactivate().Insert(OnTriggerDeactivate);
			}
			else if (!SCR_Global.IsEditMode(this))
			{
				Print("SCR_TriggerTask is missing a child of type SCR_BaseFactionTriggerEntity!", LogLevel.ERROR);
			}
		}
	}
}