[BaseContainerProps(), SCR_DamagePassRuleContainerTitle()]
class SCR_DamagePassRule : SCR_BaseDamagePassRule
{
	[Attribute(uiwidget: UIWidgets.CheckBox, desc: "Pass to default hit zone of root damage manager")]
	protected bool m_bPassToRoot;

	[Attribute(uiwidget: UIWidgets.CheckBox, desc: "Pass to default hit zone of parent damage manager")]
	protected bool m_bPassToParent;	
	
	[Attribute(uiwidget: UIWidgets.CheckBox, desc: "Pass to default hit zone of same damage manager")]
	protected bool m_bPassToDefaultHitZone;

	//------------------------------------------------------------------------------------------------
	//! Handle damage for this hit zone. Clones the damage context so that it does not override passed one
	//! \param[in,out] damageContext to be received by this hit zone
	//! \param[in] srcHitZone which initialized this process
	//! \param[in] dmgManager to which this hit zone belongs
	override void HandlePassedDamage(inout notnull BaseDamageContext damageContext, notnull SCR_HitZone srcHitZone, notnull SCR_DamageManagerComponent dmgManager)
	{
		// If damage types are defined, only allow passing specified damage types
		if (!m_aSourceDamageTypes.IsEmpty() && !m_aSourceDamageTypes.Contains(damageContext.damageType))
			return;

		// If damage states are defined, only allow passing while damage state is allowed
		if (!m_aDamageStates.IsEmpty() && !m_aDamageStates.Contains(srcHitZone.GetDamageState()))
			return;

		const BaseDamageContext contextCopy = BaseDamageContext.Cast(damageContext.Clone());
		contextCopy.damageValue *= m_fMultiplier;

		// Change passed damage type unless it is set to true
		if (m_eOutputDamageType != EDamageType.TRUE)
			contextCopy.damageType = m_eOutputDamageType;

		const IEntity owner = srcHitZone.GetOwner();
		if (m_bPassToDefaultHitZone)
		{
			const SCR_HitZone defaultHitZone = SCR_HitZone.Cast(dmgManager.GetDefaultHitZone());
			if (defaultHitZone == srcHitZone)
			{
#ifdef WORKBENCH
				Debug.Error("SCR_DamagePassRule.HandlePassedDamage: Entity " + FilePath.StripPath(owner.GetPrefabData().GetPrefabName().GetPath()) + " has " + Type().ToString() + " on a default hit zone (" + srcHitZone.GetName() + "), and it tries to pass the damage to itself, because m_bPassToDefaultHitZone is selected!");
#endif
				m_bPassToDefaultHitZone = false; // disable so until someone fixes this in data, we will not waste time on this infinite loop in the future
			}
			else
			{
				contextCopy.struckHitZone = defaultHitZone;
				dmgManager.HandleDamage(contextCopy);
			}
		}

		if (m_bPassToParent)
		{
			const IEntity parentEnt = owner.GetParent();
			if (parentEnt)
			{
				const SCR_DamageManagerComponent parentDmgManager = SCR_DamageManagerComponent.GetDamageManager(parentEnt);
				if (parentDmgManager)
				{
					contextCopy.struckHitZone = SCR_HitZone.Cast(parentDmgManager.GetDefaultHitZone());
					if (contextCopy.struckHitZone && contextCopy.struckHitZone != srcHitZone)
						parentDmgManager.HandleDamage(contextCopy);
				}
			}
		}

		if (m_bPassToRoot)
		{
			const IEntity rootEnt = owner.GetRootParent();
			SCR_DamageManagerComponent rootDmgManager;
			if (owner == rootEnt)
			{
				if (m_bPassToDefaultHitZone)
				{
#ifdef WORKBENCH
					Debug.Error("SCR_DamagePassRule.HandlePassedDamage: Entity " + FilePath.StripPath(owner.GetPrefabData().GetPrefabName().GetPath()) + " has " + Type().ToString() + " which tries to damage itself twice, as it is the root entity, while both m_bPassToDefaultHitZone and m_bPassToRoot are selected for " + srcHitZone.GetName() + "!");
#endif
					return; // dont do that twice
				}

				rootDmgManager = dmgManager;
			}
			else
			{
				rootDmgManager = SCR_DamageManagerComponent.GetDamageManager(rootEnt);
			}

			if (rootDmgManager)
			{
				contextCopy.struckHitZone = SCR_HitZone.Cast(rootDmgManager.GetDefaultHitZone());
				if (contextCopy.struckHitZone && contextCopy.struckHitZone != srcHitZone)
					rootDmgManager.HandleDamage(contextCopy);
			}
		}
	}
}

//------------------------------------------------------------------------------------------------
class SCR_DamagePassRuleContainerTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = string.Empty;

		int type;
		if (source.Get("m_eOutputDamageType", type))
		{
			if (type == EDamageType.TRUE)
				title = "Pass as same type";
			else
				title = "Pass as " + SCR_Enum.GetEnumName(EDamageType, type);
		}

		float multiplier;
		if (source.Get("m_fMultiplier", multiplier))
			title += " x " + multiplier.ToString(-1, 3);

		bool root;
		if (source.Get("m_bPassToRoot", root) && root)
			title += ", root";

		bool parent;
		if (source.Get("m_bPassToParent", parent) && parent)
			title += ", parent";		
		
		bool defaultHitZone;
		if (source.Get("m_bPassToDefaultHitZone", defaultHitZone) && defaultHitZone)
			title += ", defaultHitZone";

		return true;
	}
}

