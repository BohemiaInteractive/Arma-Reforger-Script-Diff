class SCR_AIDecoTestIsEntityBurning : DecoratorTestScripted
{
	IEntity m_ControlledEntity;
	SCR_DamageManagerComponent m_DamageManager;
	
	protected override bool TestFunction(AIAgent agent, IEntity controlled)
	{
		if (!controlled)
			return false;
		
		if (!m_DamageManager || m_ControlledEntity !=controlled)
		{
			m_DamageManager = SCR_DamageManagerComponent.GetDamageManager(controlled);
			m_ControlledEntity = controlled;
		}	
		
		return SCR_AIVehicleUsability.VehicleIsOnFire(controlled, m_DamageManager);
	}
}
