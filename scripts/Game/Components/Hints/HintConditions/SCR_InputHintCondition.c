[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_InputHintCondition : SCR_BaseHintCondition
{
	[Attribute()]
	protected ref array<string> m_aActionNames;
	
	//------------------------------------------------------------------------------------------------
	override protected void OnInitCondition(Managed owner)
	{
		InputManager inputManager = GetGame().GetInputManager();
		for (int i, count = m_aActionNames.Count(); i < count; i++)
		{
			inputManager.AddActionListener(m_aActionNames[i], EActionTrigger.DOWN, Activate);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitCondition(Managed owner)
	{
		InputManager inputManager = GetGame().GetInputManager();
		for (int i, count = m_aActionNames.Count(); i < count; i++)
		{
			inputManager.RemoveActionListener(m_aActionNames[i], EActionTrigger.DOWN, Activate);
		}
	}
}
