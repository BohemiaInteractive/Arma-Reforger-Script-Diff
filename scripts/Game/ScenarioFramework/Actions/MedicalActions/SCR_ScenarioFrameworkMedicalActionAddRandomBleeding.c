[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionAddRandomBleeding : SCR_ScenarioFrameworkMedicalAction
{
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		m_DamageManager.AddRandomBleeding();
	}
}