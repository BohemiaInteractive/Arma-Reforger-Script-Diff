class SCR_QRFSpawnSafeZone : ScriptAndConfig
{
	[Attribute(SCR_EQRFGroupType.INFANTRY.ToString(), UIWidgets.ComboBox, desc: "To which group type this distance is meant to be applied to", enums: ParamEnumArray.FromEnum(SCR_EQRFGroupType))]
	protected SCR_EQRFGroupType m_eGroupType;

	[Attribute("500", UIWidgets.Auto, "How close players can to this spawn point for it to be used for spawning QRF", "1 inf 1")]
	protected int m_iMinDistanceToClosestObserver;

	//------------------------------------------------------------------------------------------------
	SCR_EQRFGroupType GetGroupType()
	{
		return m_eGroupType;
	}

	//------------------------------------------------------------------------------------------------
	int GetMinDistanceToClosestObserver()
	{
		return m_iMinDistanceToClosestObserver;
	}
}
