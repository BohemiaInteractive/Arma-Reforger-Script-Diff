class SCR_RegenerationScreenEffect : SCR_BaseScreenEffect
{
	// PP constants
	// Variables connected to a material, need to be static
	static const int CHROM_ABER_PRIORITY								= 9;

	[Attribute(defvalue: "6", uiwidget: UIWidgets.EditBox, desc: "Duration of the regeneration effect in seconds")]
	protected float m_fRegenEffectDuration;

	[Attribute("0 0 1 1", UIWidgets.GraphDialog, desc: "Trajectory of the intensity of the regenerationEffect")]
	protected ref Curve m_Curve;

	//Character
	protected ChimeraCharacter m_pCharacterEntity;
	protected SCR_CharacterDamageManagerComponent m_pDamageManager;
	protected const string CHROMATIC_ABERIATION_EMAT								= "{A78A424C3179C706}UI/Materials/ScreenEffects_ChromAberrPP.emat";
	protected bool m_bRegenerationEffect;
	protected float m_fRegenEffectTimeRemaining;

	// Variables connected to a material, need to be static
	private static bool s_bEnabled;
	private static float s_fChromAberPower;

	//------------------------------------------------------------------------------------------------
	override protected void DisplayStartDraw(IEntity owner)
	{
		m_pCharacterEntity = ChimeraCharacter.Cast(owner);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		ClearEffects();

		m_pCharacterEntity = ChimeraCharacter.Cast(to);
		if (!m_pCharacterEntity)
			return;

		m_pDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_pCharacterEntity.GetDamageManager());
		if (!m_pDamageManager)
			return;

		m_pDamageManager.GetOnDamageEffectAdded().Insert(OnDamageEffectAdded);

		m_pCharacterEntity.GetWorld().SetCameraPostProcessEffect(m_pCharacterEntity.GetWorld().GetCurrentCameraId(), CHROM_ABER_PRIORITY, PostProcessEffectType.ChromAber, CHROMATIC_ABERIATION_EMAT);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayOnSuspended()
	{
		s_bEnabled = false;
	}

	//------------------------------------------------------------------------------------------------
	override protected void DisplayOnResumed()
	{
		s_bEnabled = true;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateEffect(float timeSlice)
	{
		if (m_bRegenerationEffect)
			RegenerationEffect(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	protected void RegenerationEffect(float timeSlice)
	{
		m_fRegenEffectTimeRemaining -= timeSlice;

		float chromAberProgress = Math.InverseLerp(m_fRegenEffectDuration, 0, m_fRegenEffectTimeRemaining);

		vector chromAberPowerScaled = LegacyCurve.Curve(ECurveType.CatmullRom, chromAberProgress, m_Curve);

		// The maximum value of the EMAT is 0.05. The chromatic aberiation cannot go over this value
		s_fChromAberPower = Math.Lerp(0, 0.05, chromAberPowerScaled[1]);

		if (m_fRegenEffectTimeRemaining <= 0)
		{
			m_bRegenerationEffect = false;
			m_fRegenEffectTimeRemaining = 0;
			s_fChromAberPower = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDamageEffectAdded(notnull SCR_DamageEffect dmgEffect)
	{
		if (dmgEffect.GetDamageType() != EDamageType.HEALING)
			return;

		m_fRegenEffectTimeRemaining = m_fRegenEffectDuration;
		m_bRegenerationEffect = true;
	}

	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		m_fRegenEffectTimeRemaining = 0;
		s_fChromAberPower = 0;

		if (m_pDamageManager)
			m_pDamageManager.GetOnDamageEffectAdded().Remove(OnDamageEffectAdded);
	}
}
