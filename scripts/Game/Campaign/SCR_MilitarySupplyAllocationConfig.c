[BaseContainerProps(configRoot: true)]
class SCR_MilitarySupplyAllocationConfig
{
	[Attribute("600", desc: "Duration of Available Allocated Supplies replenishment cooldown in seconds.", params: "0 inf 1")]
	protected int m_iAvailableAllocatedSuppliesReplenishmentTimer;

	[Attribute("1", desc: "Multiplier of amount of Available Allocated Supplies (AAS) replenished when player receives Supplies Delivery XP reward. By Default 1 XP awarded = 1 AAS replenished.", params: "0 inf 0.01")]
	protected float m_fAvailableAllocatedSuppliesReplenishmentOnSupplyDeliveryMultiplier;

	[Attribute()]
	protected ref array<ref SCR_MilitarySupplyAllocationRankInfo> m_aMilitarySupplyAllocationRankList;

	//------------------------------------------------------------------------------------------------
	void GetRankList(out notnull array<ref SCR_MilitarySupplyAllocationRankInfo> militarySupplyAllocationRankList)
	{
		foreach (SCR_MilitarySupplyAllocationRankInfo info : m_aMilitarySupplyAllocationRankList)
		{
			militarySupplyAllocationRankList.Insert(info);
		}
	}

	//------------------------------------------------------------------------------------------------
	int GetMilitarySupplyAllocationValueAtRank(SCR_ECharacterRank rank)
	{
		foreach (SCR_MilitarySupplyAllocationRankInfo rankInfo : m_aMilitarySupplyAllocationRankList)
		{
			if (rankInfo.GetRankID() == rank)
				return rankInfo.GetMilitarySupplyAllocationValue();
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	int GetAvailableAllocatedSuppliesReplenishmentTimer()
	{
		return m_iAvailableAllocatedSuppliesReplenishmentTimer;
	}

	//------------------------------------------------------------------------------------------------
	int GetAvailableAllocatedSuppliesReplenishmentThresholdValueAtRank(SCR_ECharacterRank rank)
	{
		foreach (SCR_MilitarySupplyAllocationRankInfo rankInfo : m_aMilitarySupplyAllocationRankList)
		{
			if (rankInfo.GetRankID() == rank)
				return rankInfo.GetThresholdValue();
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	float GetAvailableAllocatedSuppliesReplenishmentOnSupplyDeliveryMultiplier()
	{
		return m_fAvailableAllocatedSuppliesReplenishmentOnSupplyDeliveryMultiplier;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_MilitarySupplyAllocationRankInfo
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Player rank.", enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_eRank;

	[Attribute("0", desc: "Military supply allocation for player rank.")]
	protected int m_iMilitarySupplyAllocationValue;

	[Attribute("10", desc: "Value that Available Allocated Supplies gets replenished to.", params: "0 inf 1")]
	protected int m_iAvailableAllocatedSuppliesReplenishmentThresholdValue;

	//------------------------------------------------------------------------------------------------
	SCR_ECharacterRank GetRankID()
	{
		return m_eRank;
	}

	//------------------------------------------------------------------------------------------------
	int GetMilitarySupplyAllocationValue()
	{
		return m_iMilitarySupplyAllocationValue;
	}

	//------------------------------------------------------------------------------------------------
	int GetThresholdValue()
	{
		return m_iAvailableAllocatedSuppliesReplenishmentThresholdValue;
	}
}
