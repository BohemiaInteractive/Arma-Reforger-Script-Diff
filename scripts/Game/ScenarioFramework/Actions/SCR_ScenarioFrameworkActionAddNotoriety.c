[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAddNotoriety : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Added notoriety value (can be also used to substract)")]
	float m_fChange;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		SCR_ScenarioFrameworkNotorietyComponent notorietyComponent = SCR_ScenarioFrameworkNotorietyComponent.GetInstance();
		if (!notorietyComponent)
			return;
		
		notorietyComponent.AddNotoriety(m_fChange);
	}
}