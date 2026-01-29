/*!
\addtogroup DamageEffects
\{
*/

class SCR_InstantDamageEffect : InstantDamageEffect
{
	//------------------------------------------------------------------------------------------------
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);

		evaluator.HandleEffectConsequences(this, dmgManager);
	}
}

/*!
\}
*/
