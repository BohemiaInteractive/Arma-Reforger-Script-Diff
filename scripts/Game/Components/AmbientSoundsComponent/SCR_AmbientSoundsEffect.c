[BaseContainerProps(configRoot: true)]
class SCR_AmbientSoundsEffect
{
	protected SCR_AmbientSoundsComponent m_AmbientSoundsComponent;
	protected SignalsManagerComponent m_LocalSignalsManager;
	
	//------------------------------------------------------------------------------------------------
	//!
	SCR_AmbientSoundsComponent GetAmbientSoundsComponent()
	{
		return m_AmbientSoundsComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] worldTime
	//! \param[in] cameraPos
	// Called by SCR_AmbientSoundComponent in UpdateSoundJob()
	void Update(float worldTime, vector cameraPos)
	{
	}
	
	//------------------------------------------------------------------------------------------------	
	//!
	//! \param[in] ambientSoundsComponent
	//! \param[in] signalsManagerComponent
	// Called by SCR_AmbientSoundComponent in OnPostInit()
	void OnPostInit(SCR_AmbientSoundsComponent ambientSoundsComponent, SignalsManagerComponent signalsManagerComponent)
	{
		m_AmbientSoundsComponent = ambientSoundsComponent;
		m_LocalSignalsManager = signalsManagerComponent;
	}
	
	//------------------------------------------------------------------------------------------------	
	// Called by SCR_AmbientSoundComponent in OnInit()
	void OnInit()
	{
	}
	
#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] worldTime
	void UpdateDebug(float worldTime)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ReloadConfig()
	{
	}
#endif
}
