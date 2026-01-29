class SCR_NoiseFilterEffect : SCR_BaseScreenEffect
{
	//Character
	protected ChimeraCharacter 											m_pCharacterEntity;
	protected SCR_CharacterDamageManagerComponent 						m_pDamageManager;
	private bool m_bLocalPlayerOutsideCharacter							= true;
	bool m_bDisplaySuspended											= false;

	[Attribute(defvalue: "0.01", desc: "Max intensity of the grain effect which will be visible in dark areas", params: "0 inf 0.01")]
	protected float m_fMaxGrainIntensity;

	[Attribute(uiwidget: UIWidgets.CurveDialog, desc: "Curve defining grain effect intensity for perceived scene brightness", params:"type=Custom, ends=OpenFlat, paramRange=0 1 fixed, valueRange=0 1 fixed")]
	protected ref CurveFloat m_BrightnessIntensityMap;

	protected static float s_fGrainIntensity;
	protected static bool s_bDisableNightNoiseEffect = true;

	//------------------------------------------------------------------------------------------------
	//! \param[in] newState
	static void SetNightNoiseEffectState(bool newState)
	{
		s_bDisableNightNoiseEffect = newState;
		if (s_bDisableNightNoiseEffect)
			s_fGrainIntensity = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		ClearEffects();

		m_pCharacterEntity = ChimeraCharacter.Cast(to);

		if (!m_pCharacterEntity)
			return;

		m_pDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_pCharacterEntity.GetDamageManager());

		if (m_pDamageManager)
			m_pDamageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);

		// In case of unconsciousness starting outside of character, apply effect now
		if (m_pCharacterEntity.GetCharacterController() && m_pCharacterEntity.GetCharacterController().IsUnconscious())
			LowPassFilterEffect();
	}

	//------------------------------------------------------------------------------------------------
	// Check whether to apply or unapply the filter effect in case of unconsciousness
	override protected void DisplayConsciousnessChanged(bool conscious, bool init = false)
	{
		if (m_bDisplaySuspended)
			return;

		LowPassFilterEffect();
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayOnSuspended()
	{
		s_fGrainIntensity = 0;
		m_bDisplaySuspended = true;
		LowPassFilterEffect(true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayOnResumed()
	{
		m_bDisplaySuspended = false;
		LowPassFilterEffect(false);
	}

	//------------------------------------------------------------------------------------------------
	// Check whether to apply or unapply the filter effect in case of death
	protected void OnDamageStateChanged()
	{
		if (m_bDisplaySuspended)
			return;

		LowPassFilterEffect();
	}

	//------------------------------------------------------------------------------------------------
	private void LowPassFilterEffect(bool outsideCharacter = false)
	{
		ECharacterLifeState state = ECharacterLifeState.ALIVE;

		if (!outsideCharacter && m_pCharacterEntity)
		{
			CharacterControllerComponent charController = m_pCharacterEntity.GetCharacterController();
			state = charController.GetLifeState();
		}

		AudioSystem.SetVariableByName("CharacterLifeState", state, "{A60F08955792B575}Sounds/_SharedData/Variables/GlobalVariables.conf");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRespawnMenuOpen()
	{
		LowPassFilterEffect(true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayInit(IEntity owner)
	{
		SCR_DeployMenuBase.SGetOnMenuOpen().Insert(OnRespawnMenuOpen);
	}

	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		if (m_pDamageManager)
			m_pDamageManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);

		s_fGrainIntensity = 0;
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayUpdate(IEntity owner, float timeSlice)
	{
		super.DisplayUpdate(owner, timeSlice);
		if (s_bDisableNightNoiseEffect)
			return;

		BaseWorld world = GetGame().GetWorld();
		float hdrBrightnessFactor = world.GetCameraHDRBrightness(0);//how much is HDR boosting the image brightness - the darker it is the higher this value is
		if (hdrBrightnessFactor != 0)
			hdrBrightnessFactor = 1 / hdrBrightnessFactor;

		s_fGrainIntensity = m_fMaxGrainIntensity * m_BrightnessIntensityMap.Compute(world.GetCameraSceneMiddleBrightness(0) * hdrBrightnessFactor);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_NoiseFilterEffect()
	{
		SCR_DeployMenuBase.SGetOnMenuOpen().Remove(OnRespawnMenuOpen);
		s_fGrainIntensity = 0;
		s_bDisableNightNoiseEffect = true;
	}
}
