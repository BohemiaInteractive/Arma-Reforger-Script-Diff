class SCR_QRFTypeMaxDistance : ScriptAndConfig
{
	[Attribute(SCR_EQRFGroupType.INFANTRY.ToString(), UIWidgets.ComboBox, desc: "Group type to which this distance will apply", enums: ParamEnumArray.FromEnum(SCR_EQRFGroupType))]
	protected SCR_EQRFGroupType m_eGroupType;// (INFANTRY/VEHICLE/AIR),

	[Attribute("-1", UIWidgets.Auto, "How far this unit type can be spawned from QRF target location - where -1 == unlimited", "-1 inf 1")]
	protected int m_iMaxSpawnDistance;// (0 unlimited, 1/2/3...)

	//------------------------------------------------------------------------------------------------
	SCR_EQRFGroupType GetGroupType()
	{
		return m_eGroupType;
	}

	//------------------------------------------------------------------------------------------------
	int GetMaxSpawnDistance()
	{
		return m_iMaxSpawnDistance;
	}
}
