[BaseContainerProps(), SCR_DamagePassRuleContainerTitle()]
class SCR_BaseDamagePassRule
{
	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EDamageState), desc: "Allow this rule if current damage state is one of defined.\nLeave empty to allow all damage states")]
	protected ref array<EDamageState> m_aDamageStates;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EDamageType), desc: "Allow this rule if source damage type is one of defined.\nLeave empty to allow all damage types")]
	protected ref array<EDamageType> m_aSourceDamageTypes;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EDamageType), desc: "Force damage type to be passed.\nSet to TRUE to pass unmodified")]
	protected EDamageType m_eOutputDamageType;

	[Attribute(desc: "Scaling of damage\n[x]")]
	protected float m_fMultiplier;

	//------------------------------------------------------------------------------------------------
	//! Handle damage for this hit zone.
	//! \param[in,out] damageContext to be received by this hit zone
	//! \param[in] srcHitZone which initialized this process
	//! \param[in] dmgManager to which this hit zone belongs
	void HandlePassedDamage(inout notnull BaseDamageContext damageContext, notnull SCR_HitZone srcHitZone, notnull SCR_DamageManagerComponent dmgManager);
}