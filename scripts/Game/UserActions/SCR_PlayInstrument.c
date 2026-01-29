//------------------------------------------------------------------------------------------------
class SCR_PlayInstrument : ScriptedUserAction
{
	[Attribute("", UIWidgets.Auto)]
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;
	
	[Attribute("0", UIWidgets.ComboBox, "Instrument type", "", ParamEnumArray.FromEnum(SCR_EInstrumentType) )]
	protected SCR_EInstrumentType m_eInstrumentType;
		
	protected AudioHandle m_AudioHandle = AudioHandle.Invalid;
	
	protected static ref ScriptInvokerInt2 s_onInstrumentPlayed;
	protected SCR_DamageManagerComponent m_DamageComponent;
	
	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		
		m_DamageComponent = SCR_DamageManagerComponent.GetDamageManager(pOwnerEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if( m_DamageComponent && m_DamageComponent.IsDestroyed() )
			return false;
		
		return super.CanBeShownScript(user);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionStart(IEntity pUserEntity) 
	{
		if (s_onInstrumentPlayed)
			s_onInstrumentPlayed.Invoke(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity), m_eInstrumentType);

		if (!m_AudioSourceConfiguration || !m_AudioSourceConfiguration.IsValid())
			return;
		
		const IEntity owner = GetOwner();		
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(owner.GetWorld());
		if (!soundManager)
			return;
				
		SCR_AudioSource audioSource = soundManager.CreateAudioSource(owner, m_AudioSourceConfiguration);
		if (!audioSource)
			return;
		
		AudioSystem.TerminateSound(m_AudioHandle);
		
		soundManager.PlayAudioSource(audioSource);			
		m_AudioHandle = audioSource.m_AudioHandle;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{		
		AudioSystem.TerminateSound(m_AudioHandle);
		
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(pOwnerEntity.GetWorld());
		if (!soundManager)
			return;
				
		if (!m_AudioSourceConfiguration || m_AudioSourceConfiguration.m_sSoundProject == string.Empty)
			return;
		
		// Create stop sound audioSourceConfiguration
		SCR_AudioSourceConfiguration audioSourceConfiguration = new SCR_AudioSourceConfiguration;
		audioSourceConfiguration.m_sSoundProject = m_AudioSourceConfiguration.m_sSoundProject;
		audioSourceConfiguration.m_sSoundEventName = SCR_SoundEvent.SOUND_STOP_PLAYING;
		audioSourceConfiguration.m_eFlags = m_AudioSourceConfiguration.m_eFlags;
		audioSourceConfiguration.m_vOffset = m_AudioSourceConfiguration.m_vOffset;
						
		SCR_AudioSource audioSource = soundManager.CreateAudioSource(pOwnerEntity, audioSourceConfiguration);
		if (!audioSource)
			return;
							
		soundManager.PlayAudioSource(audioSource);			
		m_AudioHandle = audioSource.m_AudioHandle;
	}
		
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerInt2 GetOnInstrumentPlayed()
	{
		if (!s_onInstrumentPlayed)
			s_onInstrumentPlayed = new ScriptInvokerInt2();
		
		return s_onInstrumentPlayed;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_PlayInstrument()
	{
		AudioSystem.TerminateSoundFadeOut(m_AudioHandle, false, 0);
	}
};

enum SCR_EInstrumentType
{
	PIANO,
	ORGAN,
	GUITAR
}