/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class ExtendedDamageManagerComponent: SCR_DamageManagerComponent
{
	/*!
	Adds A CLONE of a DamageEffect to this DamageManager
	\param BaseDamageEffect damageEffect: A CLONE of this damage effect will be added.
	*/
	proto external void AddDamageEffect(notnull BaseDamageEffect damageEffect);
	/*!
	If this persistent effect is contained on the damage manger, it will be removed.
	Note: Function checks PersistentDamageEffects.
	\param PersistentDamageEffect dmgEffect: The damage effect to terminate.
	\return returns true if the effect was contained and removed, false otherwise or if you are a proxy.
	*/
	proto external bool TerminateDamageEffect(notnull PersistentDamageEffect dmgEffect);
	/*!
	Terminates all damage effects of the given type
	Note: Function checks PersistentDamageEffects.
	\param typename typeName: the type of the persistent effects to terminate
	*/
	proto external void TerminateDamageEffectsOfType(typename typeName);
	/*!
	Returns the DamageEffectEvaluator this ExtendedDamageManager is using
	*/
	proto external DamageEffectEvaluator GetEvaluator();
	/*!
	Switches the DamageEffectEvaluator this ExtendedDamageManager is using
	\param DamageEffectEvaluator evaluator: An instance of the evaluator to use
	*/
	proto external void SetEvaluator(notnull DamageEffectEvaluator evaluator);
	/*!
	Clears passed array (persistentEffects) and fills it with all current PersistentEffects contained in the damage manager.
	\param array<ref PersistentDamageEffect> persistentEffects: An array that will be filled with all the persistent effects on the damage manager.
	*/
	proto external void GetPersistentEffects(out notnull array<ref SCR_PersistentDamageEffect> persistentEffects);
	/*!
	Returns true if a persistent effect of the type is currently present.
	Note: Function checks PersistentDamageEffects.
	\param typename typeName: The type of DamageEffect we want to check for.
	*/
	proto external bool IsDamageEffectPresent(typename typeName);
	/*!
	Finds a DamageEffect of the requested type contained on this ExtendedDamageManager
	Note: Function checks PersistentDamageEffects.
	\param typename typeName: The type of DamageEffect we want to check for.
	\return The first DamageEffect of the requested type. Otherwise it returns null.
	*/
	proto external SCR_PersistentDamageEffect FindDamageEffectOfType(typename typeName);
	/*!
	Fills damageEffects array with all the DamageEffects of the given type present on this manager. damageEffects array is NOT cleared when calling this method.
	Note: Function checks PersistentDamageEffects.
	\param typename typeName: The type of DamageEffect we want to check for.
	\param array<ref SCR_PersistentDamageEffect> damageEffects: The found effects will be stored here
	\return the amount of matches for the effect type.
	*/
	proto external int FindAllDamageEffectsOfType(typename typeName, out notnull array<ref SCR_PersistentDamageEffect> damageEffects);
	/*!
	Fills passed array with all current persistent DamageEffects affecting the chosen hitzone.
	\param array<ref BaseDamageEffect> damageEffects: DamageEffects that affect the hitzone passed.
	\param HitZone hitzone: The hitzone where the DamageEffects are applied
	*/
	proto external void FindDamageEffectsOnHitZone(HitZone hitzone, out notnull array<ref SCR_PersistentDamageEffect> damageEffects);
	/*!
	Finds a DamageEffect of the requested type contained on this ExtendedDamageManager, that is being applied to the HitZone
	\param typename typeName: The type of DamageEffect we want to check for.
	\return The first DamageEffect of the requested type. Otherwise it returns null.
	*/
	proto external SCR_PersistentDamageEffect FindDamageEffectOnHitZone(typename typeName, HitZone hitzone);
	/*!
	Finds all the DamageEffects of the requested type contained on this ExtendedDamageManager, that is being applied to the requested HitZone
	Note: Function checks PersistentDamageEffects.
	\param typename typeName: The type of DamageEffect we want to check for.
	\param HitZone hitzone: hitzone the effects have to be applied to
	\param array<ref SCR_PersistentDamageEffect> damageEffects: Matching effects will get stored here. This array is NOT cleared when calling this method.
	\return Number of hits for the search.
	*/
	proto external int FindAllDamageEffectsOfTypeOnHitZone(typename typeName, HitZone hitzone, out notnull array<ref SCR_PersistentDamageEffect> damageEffects);
	/*!
	Returns true if a persistent effect of the type is currently present in any of the hitzones passed
	Note: Function checks PersistentDamageEffects.
	\param typename typeName: The type of DamageEffect we want to check for.
	\param array<HitZone> hitZones: List of hitzones to check against
	*/
	proto external bool IsDamageEffectPresentOnHitZones(typename typeName, notnull array<HitZone> hitZones);
	/*!
	Clears passed array (damageHistory) and fills it with all DamageEffects stored in this damage managers damage history.
	\param array<ref BaseDamageEffect> damageHistory: An array that will be filled with all the DamageEffects that affected this entity since the last time it was cleared.
	*/
	proto external void GetDamageHistory(out notnull array<ref BaseDamageEffect> damageHistory);
	/*!
	Clears the DamageHistory, containing all the DamageEffects applied to this DamageManager.
	*/
	proto external void ClearDamageHistory();

	// callbacks

	/*!
	Called when a damage effect has been added
	\param BaseDamageEffect SCR_DamageEffect: The DamageEffect that got added
	*/
	event void OnDamageEffectAdded(notnull SCR_DamageEffect dmgEffect);
	/*!
	Called when a damage effect has been removed
	\param BaseDamageEffect SCR_DamageEffect: The DamageEffect that got removed
	*/
	event void OnDamageEffectRemoved(notnull SCR_DamageEffect dmgEffect);
}

/*!
\}
*/
