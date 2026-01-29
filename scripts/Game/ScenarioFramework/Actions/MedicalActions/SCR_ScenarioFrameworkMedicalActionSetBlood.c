[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetBlood : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "6000", uiwidget: UIWidgets.Slider, desc: "Character blood hit zone value", params: "0 6000 0.001")]
	float m_fBloodValue;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		HitZone bloodHitZone = m_DamageManager.GetBloodHitZone();
		if (!bloodHitZone)
		{
			Print(string.Format("ScenarioFramework Action: Blood Hit Zone not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		bloodHitZone.SetHealth(m_fBloodValue);
	}
}