//! A scripted action class having optional logic for playing audio as well as checking if the faction is valid
class SCR_BaseAudioScriptedUserAction : SCR_ScriptedUserAction
{
	[Attribute(desc: "Sound effect played when action is exectuted")]
	protected string m_sActionSoundEffectEventName;
	
	[Attribute(desc: "File that contains the sound effect. Only used as backup if SFX was not played by the SoundComponent on the entity", params: "acp")]
	protected ResourceName m_sActionSoundEffectFile;
	
	//================================================== PERFORM ==================================================\\
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
 	{
		PlayActionSoundEffect(pOwnerEntity);
	}
	
	//================================================== AUDIO ==================================================\\
	protected void PlayActionSoundEffect(IEntity pOwnerEntity)
	{
		if (!pOwnerEntity)
			return;
		
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sActionSoundEffectEventName))
			return;
		
		//~ Get sound component
		SoundComponent soundComponent = SoundComponent.Cast(pOwnerEntity.FindComponent(SoundComponent));
		
		//~ Has sound component. Play sound with that offset
		if (soundComponent)
		{			
			//~ Check if sound was played. If true simply play it and return
			if (soundComponent.GetEventIndex(m_sActionSoundEffectEventName) != -1)
			{
				soundComponent.SoundEventOffset(m_sActionSoundEffectEventName, GetLocalPositionAction());
				return;
			}
			else 
			{
				Print(string.Format("'SCR_BaseAudioScriptedUserAction': Trying to play sound for '%1' but sound either sound event '%2' or more likely sound file needed is not included on the SoundComponent! SCR_SoundManagerModule is used instead (if any in the world) but that means position of sound is not updated if entity moves while sound is playing.", pOwnerEntity, m_sActionSoundEffectEventName, m_sActionSoundEffectFile), LogLevel.WARNING);
			}				
		}
		
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sActionSoundEffectFile))
			return;
		
		//~ No sound manager
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(pOwnerEntity.GetWorld());
		if (!soundManager)
			return;
				
		//~ Create new config
		SCR_AudioSourceConfiguration audioConfig = new SCR_AudioSourceConfiguration();
		audioConfig.m_sSoundProject = m_sActionSoundEffectFile;
		audioConfig.m_sSoundEventName = m_sActionSoundEffectEventName;
		audioConfig.m_eFlags = SCR_Enum.RemoveFlag(audioConfig.m_eFlags, EAudioSourceConfigurationFlag.Static);
	
		//~  Play sound
		SCR_AudioSource audioSource = soundManager.CreateAudioSource(pOwnerEntity, audioConfig, GetWorldPositionAction());
		if (!audioSource)
			return;

		soundManager.PlayAudioSource(audioSource);
	}
};