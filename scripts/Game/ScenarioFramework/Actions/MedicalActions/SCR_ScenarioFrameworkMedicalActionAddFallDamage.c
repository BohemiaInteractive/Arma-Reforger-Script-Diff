[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionAddFallDamage : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(desc: "Amount of fall damage to add.", uiwidget: UIWidgets.Slider, defvalue: "20", params: "0 200 1")]
	float m_fFallDamage;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		m_DamageManager.HandleAnimatedFallDamage(m_fFallDamage);
	}
}