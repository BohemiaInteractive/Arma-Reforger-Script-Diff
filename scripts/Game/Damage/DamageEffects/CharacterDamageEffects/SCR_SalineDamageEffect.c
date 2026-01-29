class SCR_SalineDamageEffect: SCR_DotDamageEffect
{
	//------------------------------------------------------------------------------------------------
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);
		
		evaluator.HandleEffectConsequences(this, dmgManager);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HijackDamageEffect(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		SCR_CharacterDamageManagerComponent characterDmgMan = SCR_CharacterDamageManagerComponent.Cast(dmgManager);
		if (characterDmgMan)
			SetAffectedHitZone(characterDmgMan.GetBloodHitZone());
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(float timeSlice, SCR_ExtendedDamageManagerComponent dmgManager)
	{
		DealDot(timeSlice, dmgManager);
	}
	
	//------------------------------------------------------------------------------------------------
	override EDamageType GetDefaultDamageType()
	{
		return EDamageType.HEALING;
	}
}