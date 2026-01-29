[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetResilience : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, desc: "Character resilience hit zone value", params: "0 100 0.001")]
	float m_fResilienceValue;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		HitZone resilienceHitZone = m_DamageManager.GetResilienceHitZone();
		if (!resilienceHitZone)
		{
			Print(string.Format("ScenarioFramework Action: Resilience Hit Zone not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		resilienceHitZone.SetHealth(m_fResilienceValue);
	}
}