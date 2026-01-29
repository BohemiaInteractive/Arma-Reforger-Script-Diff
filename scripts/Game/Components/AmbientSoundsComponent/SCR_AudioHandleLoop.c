[BaseContainerProps()]
class SCR_AudioHandleLoop
{
	//! Sound audio handle
	AudioHandle m_AudioHandle = AudioHandle.Invalid;

	//! Sound position
	vector m_aMat[4];

	//! Sound event name
	string m_sSoundEvent;
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_AudioHandleLoop()
	{
		AudioSystem.TerminateSound(m_AudioHandle);
	}
}
