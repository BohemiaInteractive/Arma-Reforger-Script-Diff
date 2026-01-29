class SCR_InteriorRequestCallback : InteriorRequestCallback
{
	protected ref SCR_AudioSource m_AudioSource;
	
	//------------------------------------------------------------------------------------------------	
	override void OnCompleted(float interior, float roomSize)
	{
		if (!SCR_Enum.HasFlag(m_AudioSource.m_eFlags, EAudioSourceFlag.Terminated))
		{			
			m_AudioSource.SetSignalValue(SCR_AudioSource.INTERIOR_SIGNAL_NAME, interior);
			m_AudioSource.SetSignalValue(SCR_AudioSource.ROOM_SIZE_SIGNAL_NAME, roomSize);
			m_AudioSource.Play();
		}
		
		m_AudioSource.m_InteriorRequestCallback = null;
		m_AudioSource = null;
	}
	
	//------------------------------------------------------------------------------------------------	
	void SCR_InteriorRequestCallback(SCR_AudioSource audioSource)
	{
		m_AudioSource = audioSource;
	}
}