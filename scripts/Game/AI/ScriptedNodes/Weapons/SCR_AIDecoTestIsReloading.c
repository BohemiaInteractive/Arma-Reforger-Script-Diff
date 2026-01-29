class SCR_AIDecoTestIsReloading : DecoratorTestScripted
{
	SCR_CharacterControllerComponent m_CharacterController;
	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		IEntity entity = agent.GetControlledEntity();
		if (!entity)
			return false;
		
		if (!m_CharacterController)
		{
			m_CharacterController = SCR_CharacterControllerComponent.Cast(entity.FindComponent(SCR_CharacterControllerComponent));
			if (!m_CharacterController)
				return false;
		}
			
		return m_CharacterController.IsReloading();
	}
};