class SCR_TourniquetDamageEffect: SCR_PersistentDamageEffect
{	
	//------------------------------------------------------------------------------------------------
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);
		
		evaluator.HandleEffectConsequences(this, dmgManager);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnEffectAdded(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		super.OnEffectAdded(dmgManager);

		//tourniquets don't have EOnFrame logic, so they don't need to be active		
		SetActive(false);
	}
}