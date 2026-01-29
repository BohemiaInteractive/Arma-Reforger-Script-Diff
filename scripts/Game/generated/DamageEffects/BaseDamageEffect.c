/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup DamageEffects
\{
*/

class BaseDamageEffect: ScriptAndConfig
{
	/*!
	Called to apply a DamageEffect.
	InstantDamageEffects get applied whenever they are added.
	PersistentDamageEffects only get applied when ApplyEffect() gets called.
	This call gets automatically replicated to clients, which means that BaseDamageEffect.Save will be getting called.
	\param SCR_ExtendedDamageManagerComponent dmgManager: Manager containing this DamageEffect
	*/
	proto external void ApplyEffect(SCR_ExtendedDamageManagerComponent dmgManager);
	/*!
	Get total damage dealt until this point by this damage effect
	\return total damage applied by this damage effect
	*/
	proto external float GetTotalDamage();
	/*!
	Gettor for the DamageType of this DamageEffect
	\return Returns type of the damage dealt by this DamageEffect
	*/
	proto external EDamageType GetDamageType();
	/*!
	Gettor for the Instigator of this DamageEffect
	\return Instigator of this DamageEffect
	*/
	proto external notnull Instigator GetInstigator();
	/*!
	Gettor for which hitzone this DamageEffect was applied to
	\return Returns the HitZone where this DamageEffect got applied
	*/
	proto external HitZone GetAffectedHitZone();
	/*!
	Settor for the DamageType of this DamageEffect
	\param EDamageType type: new damage type
	*/
	proto external void SetDamageType(EDamageType type);
	/*!
	Settor for the instigator of the damage effect
	\param Instigator instigator: new Instigator of the effect
	*/
	proto external void SetInstigator(notnull Instigator instigator);
	/*!
	Settor for the affected HitZone of the damage effect
	\param HitZone hitZone: new HitZone for this DamageEffect
	*/
	proto external void SetAffectedHitZone(notnull HitZone hitZone);
	/*!
	Once this effect gets added to a manager, some of its values shouldn't change, as their change doesn't get replicated (instigator, damage type, affected hitzone)
	and it can lead to desync.
	\return true if the effect hasnt been added to a manager yet (and therefore its values can still change)
	*/
	proto external bool IsValueChangeAllowed();
	/*!
	Used to check if you are a proxy or not
	\return true if this DamageEffect is a proxy
	*/
	proto external bool IsProxy();

	// callbacks

	/*!
	Called before the damage effect is added to the damage manager
	If it returns true, damage effect is hijacked/intercepted and therefore not added/applied to the damage manager
	Any modifications done to the damage effect will persist.
	\param SCR_ExtendedDamageManagerComponent dmgManager: Manager containing this DamageEffect
	*/
	event bool HijackDamageEffect(SCR_ExtendedDamageManagerComponent dmgManager) {return false;};
	/*!
	Called when this DamageEffect gets added to a DamageManager
	\param SCR_ExtendedDamageManagerComponent dmgManager: Manager containing this DamageEffect
	*/
	event void OnEffectAdded(SCR_ExtendedDamageManagerComponent dmgManager);
	/*!
	Called when this DamageEffect gets applied on a DamageManager
	\param SCR_ExtendedDamageManagerComponent dmgManager: Manager containing this DamageEffect
	*/
	event void OnEffectApplied(SCR_ExtendedDamageManagerComponent dmgManager);
	/*!
	Called from ApplyEffect.
	Consequences of handling a damage effect should be implemented on the evaluator when possible, as it makes effects more flexible.
	\param SCR_ExtendedDamageManagerComponent dmgManager: Manager containing this DamageEffect
	\param DamageEffectEvaluator evaluator: Evaluator being used by the damage manager.
	*/
	event void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator);
	/*!
	Called when this DamageEffect gets removed from a DamageManager
	\param SCR_ExtendedDamageManagerComponent dmgManager: Manager containing this DamageEffect
	*/
	event void OnEffectRemoved(SCR_ExtendedDamageManagerComponent dmgManager);
	/*!
	When DamageEffect info is enabled on the diag menu, this will be called.
	Used to write debug information about this DamageEffect
	\param SCR_ExtendedDamageManagerComponent dmgManager: Manager containing this DamageEffect
	*/
	event void OnDiag(SCR_ExtendedDamageManagerComponent dmgManager);
	/*!
	Since DamageEffects can't have rpl methods, streaming has to be done through this method.
	This method writes the necessary info for the replication of this DamageEffect and sends it through the network
	\param ScriptBitWriter w: data to stream should be written here.
	\return true if there were no issues on replication
	*/
	event bool Save(ScriptBitWriter w);
	/*!
	Since DamageEffects can't have rpl methods, streaming has to be done through this method.
	This method writes the necessary info for the replication of this DamageEffect and sends it through the network
	\param ScriptBitReader r: Streamed data should be read from here.
	\return true if there were no issues on replication
	*/
	event bool Load(ScriptBitReader r);
}

/*!
\}
*/
