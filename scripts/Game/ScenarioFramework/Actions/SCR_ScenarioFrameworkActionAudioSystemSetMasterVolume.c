[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAudioSystemSetMasterVolume : SCR_ScenarioFrameworkActionBase
{
	[Attribute(defvalue: "0", desc: "ID", params: "0 inf")]
	int m_iID;
	
	[Attribute(defvalue: "0", desc: "Volume", params: "0 inf")]
	int m_iVolume;
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		AudioSystem.SetMasterVolume(m_iID,m_iVolume)
	}
}