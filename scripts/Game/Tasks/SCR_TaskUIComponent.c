class SCR_TaskUIComponentClass : ScriptComponentClass
{
}

class SCR_TaskUIComponent : ScriptComponent
{
	protected SCR_Task m_Task;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_Task = SCR_Task.Cast(GetOwner());
		if (!m_Task)
			Print("SCR_TaskUIComponent: Missing Task Entity!", LogLevel.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
}