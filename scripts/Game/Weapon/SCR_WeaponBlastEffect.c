class SCR_WeaponBlastEffect : ScriptAndConfig
{
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EDamageType))]
	protected EDamageType m_eDamageType;

	[Attribute()]
	protected ref BaseDamageEffect m_DamageEffect;

	[Attribute(desc: "How much damage should be dealt at point blank")]
	protected float m_fDamageValue;

	[Attribute("0 1 1 0", uiwidget: UIWidgets.GraphDialog, desc: "How damage will scale with distance", params: "1 1 0 0")]
	protected ref Curve m_aDistanceDamageFalloff;

	[Attribute("0 1 1 0", uiwidget: UIWidgets.GraphDialog, desc: "How damage will scale with angular distance from the center stream of the blast.\nThis can be used to form the shape in which damage is applied.\nHorizontal axis is responsible for how close to the zero deviation we are 0 == as far as possible, 1 as close as possible", params: "1 1 0 0")]
	protected ref Curve m_aAngleDamageFalloff;

	//------------------------------------------------------------------------------------------------
	EDamageType GetDamageType()
	{
		return m_eDamageType;
	}

	//------------------------------------------------------------------------------------------------
	BaseDamageEffect GetDamageEffect()
	{
		return m_DamageEffect;
	}

	//------------------------------------------------------------------------------------------------
	float GetDamageValueRaw()
	{
		return m_fDamageValue;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] normDistance normalized distance value (0-1)
	//! \param[in] dotAngle dot product from the direction of the blast to the hit direction (0-1)
	float GetComputedDamage(float normDistance, float dotAngle)
	{
		return m_fDamageValue * LegacyCurve.Curve(ECurveType.CurveProperty2D, normDistance, m_aDistanceDamageFalloff)[1] * LegacyCurve.Curve(ECurveType.CurveProperty2D, dotAngle, m_aAngleDamageFalloff)[1];
	}
}
