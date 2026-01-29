class SCR_MorphineDamageEffect: SCR_DotDamageEffect
{
	float m_fDurationPerHitZone; 
	float m_fAccurateTimeSlice;
	protected ref array<HitZone> m_aPhysicalHitZones = {};
	
	//------------------------------------------------------------------------------------------------
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);
		
		evaluator.HandleEffectConsequences(this, dmgManager);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HijackDamageEffect(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		SetAffectedHitZone(dmgManager.GetDefaultHitZone());
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnEffectAdded(SCR_ExtendedDamageManagerComponent dmgManager)			
	{
		super.OnEffectAdded(dmgManager);
		
		dmgManager.GetPhysicalHitZones(m_aPhysicalHitZones);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(float timeSlice, SCR_ExtendedDamageManagerComponent dmgManager)
	{
		m_fAccurateTimeSlice = GetAccurateTimeSlice(timeSlice);

		float damageAmount = GetDPS() * m_fAccurateTimeSlice;
		
		DotDamageEffectTimerToken token = UpdateTimer(m_fAccurateTimeSlice, dmgManager);
		
		foreach(HitZone hz : m_aPhysicalHitZones)
		{
			DealCustomDot(hz, damageAmount, token, dmgManager);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override EDamageType GetDefaultDamageType()
	{
		return EDamageType.HEALING;
	}
}