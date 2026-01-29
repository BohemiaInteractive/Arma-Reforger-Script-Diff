class SCR_DamageBlurEffect : SCR_BaseScreenEffect
{
	// PP constants
	// Variables connected to a material, need to be static
	static const int GAUSS_BLUR_PRIORITY								= 7;

	//GaussBlurriness
	[Attribute(defvalue: "0.25", uiwidget: UIWidgets.EditBox, desc: "Duration of the blurriness upon taking damage")]
	protected float m_fMomentaryDamageBlurDuration;

	[Attribute(defvalue: "5", uiwidget: UIWidgets.EditBox, desc: "Minimum amount of damage in one event for effect to appear")]
	protected float m_fGaussBlurMinDamage;

	protected const string GAUSS_BLUR_EMAT								= "{790527EE96732730}UI/Materials/ScreenEffects_GaussBlurPP.emat";
	private static float s_fGaussBlurriness;
	private static bool s_bRemoveGaussBlur 								= true;
	private float m_fGaussBlurReduction 								= 1;
	private static bool s_bEnableGaussBlur;

	// Owner data
	protected SignalsManagerComponent 									m_pSignalsManager;

	// Character
	protected ChimeraCharacter 											m_pCharacterEntity;
	protected SCR_CharacterDamageManagerComponent 						m_pDamageManager;

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		ClearEffects();

		m_pCharacterEntity = ChimeraCharacter.Cast(to);
		if (!m_pCharacterEntity)
			return;

		m_pCharacterEntity.GetWorld().SetCameraPostProcessEffect(m_pCharacterEntity.GetWorld().GetCurrentCameraId(), GAUSS_BLUR_PRIORITY, PostProcessEffectType.GaussFilter, GAUSS_BLUR_EMAT);

		m_pDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_pCharacterEntity.GetDamageManager());
		if (m_pDamageManager)
			m_pDamageManager.GetOnDamage().Insert(OnDamage);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDamage(BaseDamageContext damageContext)
	{
		if (damageContext.damageValue > m_fGaussBlurMinDamage)
			m_fGaussBlurReduction = Math.Clamp(++m_fGaussBlurReduction, 0, 2);
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayOnSuspended()
	{
		s_bEnableGaussBlur = false;
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayOnResumed()
	{
		s_bEnableGaussBlur = true;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateEffect(float timeSlice)
	{
		if (m_fGaussBlurReduction > 0)
			MomentaryDamageEffect(timeSlice);
		else
			s_fGaussBlurriness = 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void MomentaryDamageEffect(float timeslice)
	{
		// enable the gaussblur ematerial
		s_bEnableGaussBlur = true;

		// gaussblur is set to max, then drained back to 0
		if (!s_bRemoveGaussBlur)
		{
			s_fGaussBlurriness = 1;
			s_bRemoveGaussBlur = true;
			return;
		}

		m_fGaussBlurReduction -= timeslice / m_fMomentaryDamageBlurDuration;
		s_fGaussBlurriness = Math.Lerp(0, 1, m_fGaussBlurReduction);

		if (s_fGaussBlurriness <= 0)
		{
			s_bRemoveGaussBlur = false;
			s_fGaussBlurriness = 0;
			m_fGaussBlurReduction = 0;
			s_bEnableGaussBlur = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		s_fGaussBlurriness = 0;

		if (!m_pDamageManager)
			return;

		m_pDamageManager.GetOnDamage().Remove(OnDamage);
	}
}
