[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAudioSystemPauseResume : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "If true, audio categories will be paused, otherwise they will be resumed")]
	bool m_bPause;
	
	[Attribute(desc: "SFX")]
	bool m_bSFX;
	
	[Attribute(desc: "Music")]
	bool m_bMusic;
	
	[Attribute(desc: "Voice Chat")]
	bool m_bVoiceChat;
	
	[Attribute(desc: "Dialog")]
	bool m_bDialog;
	
	[Attribute(desc: "UI")]
	bool m_bUI;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		int mastersMask;
		
		//Eww
		if (m_bSFX)
			mastersMask |= (1 << AudioSystem.SFX);
		
		if (m_bMusic)
			mastersMask |= (1 << AudioSystem.Music);
		
		if (m_bVoiceChat)
			mastersMask |= (1 << AudioSystem.VoiceChat);
		
		if (m_bDialog)
			mastersMask |= (1 << AudioSystem.Dialog);
		
		if (m_bUI)
			mastersMask |= (1 << AudioSystem.UI);
		
		if (m_bPause)
			AudioSystem.Pause(mastersMask);
		else
			AudioSystem.Resume(mastersMask);
	}
}