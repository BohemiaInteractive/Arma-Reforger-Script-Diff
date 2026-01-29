class SCR_DesaturationEffect : SCR_BaseScreenEffect
{
	// PP constants
	// Variables connected to a material, need to be static
	static const int COLORS_PP_PRIORITY									= 5;
	
	//Saturation
	private static float s_fSaturation 									= 1;
	protected const string DESATURATION_EMAT							= "{7C202A913EB8B1A9}UI/Materials/ScreenEffects_ColorPP.emat";
	
	[Attribute( defvalue: "0.5", uiwidget: UIWidgets.EditBox)]
	protected float m_fDesaturationStart;
	
	[Attribute( defvalue: "0.333", uiwidget: UIWidgets.EditBox)]
	protected float m_fDesaturationEnd;

	// Enabling/Disabling of PP fx
	private static bool s_bEnableSaturation;

	// Character
	protected ChimeraCharacter m_pCharacterEntity;
	protected SCR_CharacterBloodHitZone									m_BloodHZ;
	protected SCR_CharacterDamageManagerComponent						m_DamageManager;
	protected bool m_bLocalPlayerOutsideCharacter;
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_pCharacterEntity = ChimeraCharacter.Cast(owner);
	}

	//------------------------------------------------------------------------------------------------
	override void SettingsChanged()
	{
		if (!m_pCharacterEntity)
			return;
		
		m_pCharacterEntity.GetWorld().SetCameraPostProcessEffect(m_pCharacterEntity.GetWorld().GetCurrentCameraId(), COLORS_PP_PRIORITY, PostProcessEffectType.Colors, DESATURATION_EMAT);
	}

	//------------------------------------------------------------------------------------------------
 	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		ClearEffects();
		
		m_pCharacterEntity = ChimeraCharacter.Cast(to);
		if (!m_pCharacterEntity)
			return;
		
		m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(m_pCharacterEntity.GetDamageManager());
		if (!m_DamageManager)
			return;
		
		// define hitzones for later getting
		m_BloodHZ = m_DamageManager.GetBloodHitZone();
		
		m_pCharacterEntity.GetWorld().SetCameraPostProcessEffect(m_pCharacterEntity.GetWorld().GetCurrentCameraId(), COLORS_PP_PRIORITY, PostProcessEffectType.Colors, DESATURATION_EMAT);
	}
	
	//------------------------------------------------------------------------------------------------	
	protected override void DisplayOnSuspended()
	{
		s_bEnableSaturation = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void DisplayOnResumed()
	{
		s_bEnableSaturation = true;
	}

	//------------------------------------------------------------------------------------------------
	protected override void UpdateEffect(float timeSlice)
	{
		AddDesaturationEffect();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddDesaturationEffect()
	{
		if (!m_BloodHZ)
			return;

		float bloodLevel = Math.InverseLerp(m_fDesaturationEnd, m_fDesaturationStart, m_BloodHZ.GetHealthScaled());

		s_fSaturation = Math.Clamp(bloodLevel, 0, 1);
		s_bEnableSaturation = s_fSaturation < 1;
	}

	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		s_fSaturation						= 1;
	}
};