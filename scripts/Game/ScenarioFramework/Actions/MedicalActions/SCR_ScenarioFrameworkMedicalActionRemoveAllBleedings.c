[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionRemoveAllBleedings : SCR_ScenarioFrameworkMedicalAction
{
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		m_DamageManager.RemoveAllBleedings();
	}
}