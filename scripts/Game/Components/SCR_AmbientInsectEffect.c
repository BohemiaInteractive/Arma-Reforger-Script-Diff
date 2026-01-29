[BaseContainerProps(configRoot: true)]
class SCR_AmbientInsectsEffect
{
	protected SCR_AmbientSoundsComponent m_AmbientSoundsComponent;
	protected SCR_AmbientInsectsComponent m_AmbientInsectsComponent;
	protected SignalsManagerComponent m_LocalSignalsManager;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] worldTime
	//! \param[in] cameraPos
	//! \param[in] timeOfDay
	//! \param[in] rainIntensity
	// Called by SCR_AmbientInsectsComponent in UpdateSoundJob()
	void Update(float worldTime, vector cameraPos, float timeOfDay, float rainIntensity);

	//------------------------------------------------------------------------------------------------
	//! \param[in] ambientSoundsComponent
	//! \param[in] ambientInsectsComponent
	//! \param[in] signalsManagerComponent
	// Called by SCR_AmbientInsectsComponent in OnPostInit()
	void OnPostInit(SCR_AmbientSoundsComponent ambientSoundsComponent, SCR_AmbientInsectsComponent ambientInsectsComponent, SignalsManagerComponent signalsManagerComponent)
	{
		m_AmbientSoundsComponent = ambientSoundsComponent;
		m_AmbientInsectsComponent = ambientInsectsComponent;
		m_LocalSignalsManager = signalsManagerComponent;
	}
}
