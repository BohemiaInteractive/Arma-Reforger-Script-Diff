class SCR_GameModeHealthSettingsClass : ScriptComponentClass
{
}

class SCR_GameModeHealthSettings : ScriptComponent
{	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character bleeding rate multiplier", params: "0 5 0.001", precision: 3, category: "Game Mode")]
	protected float m_fDOTScale;	
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character regeneration rate multiplier", params: "0 5 0.001", precision: 3, category: "Game Mode")]
	protected float m_fRegenScale;
	
	[Attribute(defvalue: "false", uiwidget: UIWidgets.CheckBox, desc: "Whether to allow unconsciousness even when waking up automatically is not guaranteed", category: "Game Mode")]
	protected bool m_bKillIndefinitelyUnconsciousPlayers;
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Whether unconsciousness is allowed", category: "Game Mode")]
	protected bool m_bPermitUnconsciousness;

	[Attribute(defvalue: "false", uiwidget: UIWidgets.CheckBox, desc: "Whether using VON during unconsciousness is allowed", category: "Game Mode")]
	protected bool m_bPermitUnconsciousVON;
	
	[Attribute(defvalue: "0.75", uiwidget: UIWidgets.Slider, desc: "How much will the character be slowed down when having tourniquetted leg", params: "0 1 0.001", precision: 3, category: "Game Mode")]
	protected float m_fTourniquettedLegMovementSlowdown;
	
	[Attribute(defvalue: "0.1", uiwidget: UIWidgets.Slider, desc: "Affects how much the bleeding is reduced", params: "0 1 0.001", precision: 3, category: "Game Mode")]
	private float m_fTourniquetStrengthMultiplier;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "How fast will character regenerate when situated in medical compartment", params: "0 5 0.001", precision: 3, category: "Game Mode")]
	protected float m_fCompartmentRegenerationRateMultiplier;
	
	[Attribute(defvalue: "10", uiwidget: UIWidgets.Slider, desc: "Time without receiving damage or bleeding to start regeneration [s]", params: "0 600 1", category: "Game Mode")]
	protected float m_fRegenerationDelay;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character resilience regeneration rate multiplier", params: "0 20", category: "Game Mode")]
	protected float m_fRegenerationScale;

	//------------------------------------------------------------------------------------------------
	//!
	//! \return rate (0.0-5.00) Higher numbers result in faster bleeding
	float GetBleedingScale()
	{
		return m_fDOTScale;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rate (0.0-5.00) Higher numbers result in faster bleeding
	void SetBleedingScale(float rate)
	{
		m_fDOTScale = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return rate (0.0-1000.00) Higher numbers result in slower regeneration of resilience
	float GetResilienceHzRegenScale()
	{
		return m_fRegenerationScale;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rate (0.0-1000.00) Higher numbers result in slower regeneration of resilience
	void SetResilienceHzRegenScale(float rate)
	{
		m_fRegenerationScale = rate;
	}
	//------------------------------------------------------------------------------------------------
	//!
	//! \return rate (0.0-5.00) Higher numbers result in faster regeneration
	float GetRegenScale()
	{
		return m_fRegenScale;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rate (0.0-5.00) Higher numbers result in faster regeneration
	void SetRegenScale(float rate)
	{
		m_fRegenScale = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return permitted (true/false) True means that unconsciousness is enabled.
	bool IsUnconsciousnessPermitted()
	{
		return m_bPermitUnconsciousness;
	}	
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return Return if unconscious persons with no guaranteed wake up time will die upon going unconscious.
	bool GetIfKillIndefiniteUnconscious()
	{
		return m_bKillIndefinitelyUnconsciousPlayers;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] permitted (true/false) True means that unconsciousness is enabled.
	void SetUnconsciousnessPermitted(bool permitted)
	{
		m_bPermitUnconsciousness = permitted;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return permitted (true/false) True means that using VON while unconsciousness is enabled.
	bool IsUnconsciousVONPermitted()
	{
		return m_bPermitUnconsciousVON;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] permitted (true/false) True means that VON is enabled while unconscious.
	void SetUnconsciousVONPermitted(bool permitted)
	{
		m_bPermitUnconsciousVON = permitted;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return rate (0.0-1.00) Higher numbers result in higher slowdown
	float GetTourniquettedLegMovementSlowdown()
	{
		return m_fTourniquettedLegMovementSlowdown;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rate (0.0-1.00) Higher numbers result in higher slowdown
	void SetTourniquettedLegMovementSlowdown(float rate)
	{
		m_fTourniquettedLegMovementSlowdown = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return rate (0.0-1.00) Lower numbers result in slower bleeding when tourniquet is on
	float GetTourniquetStrengthMultiplier()
	{
		return m_fTourniquetStrengthMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rate (0.0-1.00) Lower numbers result in slower bleeding when tourniquet is on
	void SetTourniquetStrengthMultiplier(float rate)
	{
		m_fTourniquetStrengthMultiplier = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return rate (0.0-5.00) Higher numbers result in faster regeneration when in certain medical compartment
	float GetCompartmentRegenRateMultiplier()
	{
		return m_fCompartmentRegenerationRateMultiplier;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rate (0.0-5.00) Higher numbers result in faster regeneration when in certain medical compartment
	void SetCompartmentRegenRateMultiplier(float rate)
	{
		m_fCompartmentRegenerationRateMultiplier = rate;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return rate (0.0-60.00) Higher numbers result in higher delay the regeneration cooldown
	float GetRegenerationDelay()
	{
		return m_fRegenerationDelay;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rate (0.0-60.00) Higher numbers result in higher delay the regeneration cooldown
	void SetRegenerationDelay(float rate)
	{
		m_fRegenerationDelay = rate;
	}
}
