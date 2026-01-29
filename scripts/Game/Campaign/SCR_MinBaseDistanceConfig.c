[BaseContainerProps(configRoot: true)]
class SCR_MinBaseDistanceConfig
{
	[Attribute()]
	protected ref array<ref SCR_MinBaseDistanceInfo> m_aMinBaseDistanceInfoList;

	//------------------------------------------------------------------------------------------------
	//! \param[in] baseType
	//! \return minimal distance to a base based on base type to establish a base
	int GetMinDistanceFromBase(SCR_ECampaignBaseType baseType)
	{
		foreach (SCR_MinBaseDistanceInfo baseDistanceInfo : m_aMinBaseDistanceInfoList)
		{
			if (baseDistanceInfo.GetBaseType() == baseType)
				return baseDistanceInfo.GetDistance();
		}

		return 0;
	}
}

[BaseContainerProps()]
class SCR_MinBaseDistanceInfo
{
	[Attribute(defvalue: SCR_ECampaignBaseType.BASE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Base type", enumType: SCR_ECampaignBaseType)]
	protected SCR_ECampaignBaseType m_eBaseType;

	[Attribute("0", params: "0 inf 1", desc: "Minimum required distance from the base to establish a new base.")]
	protected int m_iMinimumRequiredDistanceFromBase;

	//------------------------------------------------------------------------------------------------
	SCR_ECampaignBaseType GetBaseType()
	{
		return m_eBaseType;
	}

	//------------------------------------------------------------------------------------------------
	int GetDistance()
	{
		return m_iMinimumRequiredDistanceFromBase;
	}
}
