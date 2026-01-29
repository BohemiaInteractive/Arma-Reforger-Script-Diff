[ComponentEditorProps(category: "GameScripted/Weapon", description:"Keeps settings for melee weapon")]
class SCR_MeleeWeaponPropertiesClass: ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_MeleeWeaponProperties : ScriptComponent
{
	[Attribute("10", "Size of damage dealt by the weapon", category: "Global")]
	private float m_fDamage;	
	
	[Attribute("1", "Range of the weapon [m]", category: "Global")]
	private float m_fRange;
	
	[Attribute("0.3", "Accuracy of melee attacks, smaller values are more accurate", category: "Global")]
	protected float m_fAccuracy;
	
	protected BaseWeaponStatsManagerComponent m_statsManager;

	//------------------------------------------------------------------------------------------------	
	//! Value of damage dealt to the target
	float GetWeaponDamage()
	{
		if (!m_statsManager)
			return m_fDamage;
		
		float fDamageFactor;
		if (!m_statsManager.GetMeleeDamageFactor(fDamageFactor))
			return m_fDamage;
		
		return fDamageFactor * m_fDamage;
	}

	//------------------------------------------------------------------------------------------------	
	//! Range in meters that is used as max raycast length
	float GetWeaponRange()
	{
		if (!m_statsManager)
			return m_fRange;
		
		float fRangeFactor;
		if (!m_statsManager.GetMeleeRangeFactor(fRangeFactor))
			return m_fRange;
		
		return fRangeFactor * m_fRange;
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Size of the raysphere used to trace the target
	float GetWeaponMeleeAccuracy()
	{
		if (!m_statsManager)
			return m_fAccuracy;
		
		float fAccuracyFactor;
		if (!m_statsManager.GetMeleeAccuracyFactor(fAccuracyFactor))
			return m_fAccuracy;
		
		return fAccuracyFactor * m_fAccuracy;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_statsManager = BaseWeaponStatsManagerComponent.Cast(owner.FindComponent(BaseWeaponStatsManagerComponent));
	}
};