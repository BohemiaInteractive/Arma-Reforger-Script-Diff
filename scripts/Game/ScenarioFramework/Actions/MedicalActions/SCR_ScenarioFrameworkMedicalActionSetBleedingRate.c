[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetBleedingRate : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character bleeding rate multiplier", params: "0 5 0.001")]
	float m_fBleedingRate;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		m_DamageManager.SetBleedingScale(m_fBleedingRate, true);
	}
}