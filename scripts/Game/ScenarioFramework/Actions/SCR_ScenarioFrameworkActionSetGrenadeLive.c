[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetGrenadeLive : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Grenade entity to set live.")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "If enabled, grenade will be affected by physics")]
	bool m_bEnableSimulation;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;
		
		BaseTriggerComponent triggerComp = BaseTriggerComponent.Cast(entity.FindComponent(BaseTriggerComponent));
		if (!triggerComp)
			return;
		
		//Delayed call, so grenade is set "live" only after it is trully prepared.
		SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(triggerComp.SetLive, 1000, false);
		
		if (!m_bEnableSimulation)
			return;
		
		GrenadeMoveComponent projectileComponent = GrenadeMoveComponent.Cast(entity.FindComponent(GrenadeMoveComponent));
		if (projectileComponent)
			projectileComponent.EnableSimulation(entity);
	}
}