//------------------------------------------------------------------------------------------------
class SCR_FlushToilet : ScriptedUserAction
{	
	[Attribute("", UIWidgets.Auto)]
	ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;
		
	private AudioHandle m_AudioHandle = AudioHandle.Invalid;
	
	protected static ref ScriptInvokerInt s_onToiletFlushed;
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{
		if (s_onToiletFlushed)
			s_onToiletFlushed.Invoke(GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(pUserEntity));
		
		if (!m_AudioSourceConfiguration || !m_AudioSourceConfiguration.IsValid())
			return;
		
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(pOwnerEntity.GetWorld());
		if (!soundManager)
			return;
				
		SCR_AudioSource audioSource = soundManager.CreateAudioSource(pOwnerEntity, m_AudioSourceConfiguration);
		if (!audioSource)
			return;
				
		soundManager.PlayAudioSource(audioSource);
	
		m_AudioHandle = audioSource.m_AudioHandle;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerInt GetOnToiletFlushed()
	{
		if (!s_onToiletFlushed)
			s_onToiletFlushed = new ScriptInvokerInt();
		
		return s_onToiletFlushed;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "#AR-UserAction_FlushToilet";
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{		
		return AudioSystem.IsSoundPlayed(m_AudioHandle);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_FlushToilet()
	{
		AudioSystem.TerminateSoundFadeOut(m_AudioHandle, false, 0);
	}
};