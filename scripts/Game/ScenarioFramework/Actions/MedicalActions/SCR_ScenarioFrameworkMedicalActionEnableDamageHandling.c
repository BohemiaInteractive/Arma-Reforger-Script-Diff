[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionEnableDamageHandling : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(desc: "Enable Damage Handling")]
	bool m_bEnableDamageHandling;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		m_DamageManager.EnableDamageHandling(m_bEnableDamageHandling);
	}
}