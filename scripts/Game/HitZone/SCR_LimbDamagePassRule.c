[BaseContainerProps(), SCR_DamagePassRuleContainerTitle()]
class SCR_LimbDamagePassRule : SCR_BaseDamagePassRule
{
	[Attribute(defvalue: "0", desc: "Hit zone group which is going to absorb the rest of the damage, in case that all hit zones from the group of this hit zone are already destroyed", uiwidget: UIWidgets.ComboBox, enumType: ECharacterHitZoneGroup)]
	protected ECharacterHitZoneGroup m_eSpilloverHitZoneGroup;

	//------------------------------------------------------------------------------------------------
	//! Distributes sequnetially the damage amongst other hit zone group members
	//! \param[in,out] damageContext to be received by the hit zone which called this method
	//! \param[in] srcHitZone which called this method
	//! \param[in] dmgManager to which this hit zone belongs
	override void HandlePassedDamage(inout notnull BaseDamageContext damageContext, notnull SCR_HitZone srcHitZone, notnull SCR_DamageManagerComponent dmgManager)
	{
		// If damage states are defined, only allow passing while damage state is allowed
		if (!m_aDamageStates.IsEmpty() && !m_aDamageStates.Contains(srcHitZone.GetDamageState()))
			return;

		// If damage types are defined, only allow passing specified damage types
		if (!m_aSourceDamageTypes.IsEmpty() && !m_aSourceDamageTypes.Contains(damageContext.damageType))
			return;

		//! Damage context clone that prevents this rule from influencing the result of other potential rules
		BaseDamageContext contextCopy = BaseDamageContext.Cast(damageContext.Clone());
		contextCopy.damageValue *= m_fMultiplier;

		// Change passed damage type unless it is set to true
		if (m_eOutputDamageType != EDamageType.TRUE)
			contextCopy.damageType = m_eOutputDamageType;

		array<HitZone> groupHitZones = {};
		if (dmgManager.GetHitZonesOfGroupsFromOwner({srcHitZone.GetHitZoneGroup(), m_eSpilloverHitZoneGroup}, groupHitZones) < 1)
			return; // probably this is never going to happen but if it somehow does then its going to be sus as... very sus!

		SCR_HitZone targetHitZone;
		foreach (HitZone hZ : groupHitZones)
		{
			if (contextCopy.damageValue <= 0)
				return; // we quit when we managed to distribute all of the damage, or when we run out of the hit zones

			if (hZ == srcHitZone) // no infinity loop allowed!
				continue;

			if (hZ.GetDamageState() == EDamageState.DESTROYED) // same goes for this, as otherwise other parts would try to pass this damage to other parts... you get the point
				continue;

			targetHitZone = SCR_HitZone.Cast(hZ); // there should be no need to null check this, as GetHitZonesOfGroups will only return hit zones of SCR_HitZone type
			TryDealDamage(contextCopy, targetHitZone, dmgManager);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Distributes sequnetially the damage amongst other hit zone group members
	//! \param[in,out] damageContext reusable context which is going to be used to deal damage, and store information of how much damage was still left for distribution
	//! \param[in] targetHitZone which should be damaged
	//! \param[in] dmgManager which is going to deal the damage
	protected void TryDealDamage(inout notnull BaseDamageContext damageContext, notnull SCR_HitZone targetHitZone, notnull SCR_DamageManagerComponent dmgManager)
	{
		//! actual amount of damage that was absorbed by target hit zone
		float absorbedDamage;
		damageContext.struckHitZone = targetHitZone;
		if (damageContext.damageType == EDamageType.TRUE)
		{
			absorbedDamage = targetHitZone.GetHealth();
			dmgManager.HandleDamage(damageContext);
			damageContext.damageValue -= absorbedDamage;
			return;
		}

		const float dmgBaseMultiplier = targetHitZone.GetBaseDamageMultiplier();
		if (dmgBaseMultiplier == 0)
			return;

		const float dmgTypeMultiplier = targetHitZone.GetDamageMultiplier(damageContext.damageType);
		if (dmgTypeMultiplier == 0)
			return;

		const float dmgReduction = targetHitZone.GetDamageReduction();
		const float dmgThreshold = targetHitZone.GetDamageThreshold();

		//! The actual amount of damage that will be done to the target hit zone
		const float effectiveDamageValue = damageContext.damageValue * dmgBaseMultiplier * dmgTypeMultiplier - dmgReduction;
		if (effectiveDamageValue < dmgThreshold)
			return;

		absorbedDamage = targetHitZone.GetHealth() + dmgReduction;
		if (absorbedDamage < dmgThreshold)
			absorbedDamage = dmgThreshold;

		absorbedDamage = absorbedDamage / dmgBaseMultiplier / dmgTypeMultiplier;

		dmgManager.HandleDamage(damageContext);
		damageContext.damageValue -= absorbedDamage;
	}
}