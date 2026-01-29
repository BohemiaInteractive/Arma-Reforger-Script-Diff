[BaseContainerProps()]
class SCR_BaseNetworkedStat
{
	[Attribute(uiwidget : UIWidgets.ComboBox, enums : ParamEnumArray.FromEnum(SCR_ENetworkedStatType))]
	protected SCR_ENetworkedStatType m_eStatType;

	protected SCR_NetworkedStatsComponent m_Owner;
	protected SCR_NetworkedStatsComponentClass m_OwnerData;

	//------------------------------------------------------------------------------------------------
	SCR_ENetworkedStatType GetStatType()
	{
		return m_eStatType;
	}

	//------------------------------------------------------------------------------------------------
	// Override this method in child classes.
	void UpdateStat();

	//------------------------------------------------------------------------------------------------
	// Override this method in child classes.
	void OnAuthorityStatValueSet(int value);

	//------------------------------------------------------------------------------------------------
	void SetOwner(notnull SCR_NetworkedStatsComponent owner)
	{
		m_Owner = owner;
		m_OwnerData = m_Owner.GetNetworkedStatPrefabData();
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeStatValueChange()
	{
		if (!m_OwnerData)
		{
			Print("[SCR_FPSNetworkedStat] Failed to obtain Prefab Data for SCR_NetworkedStatsComponent!", LogLevel.ERROR);
			return;
		}

		SCR_OnStatChangedInvoker onStatChanged = m_OwnerData.GetOnStatsChanged(false);
		if (onStatChanged)
			onStatChanged.Invoke(this);
	}
}
