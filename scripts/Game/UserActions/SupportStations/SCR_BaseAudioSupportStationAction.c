class SCR_BaseAudioSupportStationAction : SCR_BaseUseSupportStationAction
{
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, desc: "Sound project (acp)", "acp")]
	protected ResourceName m_sSoundProject;
	
	[Attribute(desc: "Sound Effect name")]
	protected string m_sSoundEventName;
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] soundProject acp sound project for sound
	//! \param[out] soundEffectName Sound event name of sound
	//! \return True if valid Audio files were found
	bool GetSoundEffectProjectAndEvent(out ResourceName soundProject, out string soundEffectName)
	{
		if (!Resource.Load(m_sSoundProject).IsValid() || m_sSoundEventName.IsEmpty())
			return false;
		
		soundProject = m_sSoundProject;
		soundEffectName = m_sSoundEventName;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetSoundSource()
	{
		return GetOwner();
	}
}
