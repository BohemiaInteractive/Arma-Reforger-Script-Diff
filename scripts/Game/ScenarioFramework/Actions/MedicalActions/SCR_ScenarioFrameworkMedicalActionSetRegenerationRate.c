[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetRegenerationRate : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character regeneration rate multiplier", params: "0 5 0.001")]
	float m_fRegeneration;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		m_DamageManager.SetRegenScale(m_fRegeneration, true);
	}
}