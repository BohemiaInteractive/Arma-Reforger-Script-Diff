[BaseContainerProps(configRoot: true)]
class SCR_SeizeXpRewardConfig
{
	[Attribute("60", params: "0 inf", desc: "Time in seconds between seizing XP rewards during the seizing.")]
	protected int m_iXPRewardTimer;

	[Attribute()]
	protected ref array<ref SCR_SeizeCompletionXpRewardInfo> m_aSeizeCompletionXpRewardList;

	[Attribute()]
	protected ref array<ref SCR_SeizeProgressionXpRewardInfo> m_aSeizeProgressionXpRewardList;

	//------------------------------------------------------------------------------------------------
	int GetSeizeProgressionTimer()
	{
		return m_iXPRewardTimer;
	}

	//------------------------------------------------------------------------------------------------
	int GetSeizeProgressionXpReward(string factionKey)
	{
		foreach (SCR_SeizeProgressionXpRewardInfo info : m_aSeizeProgressionXpRewardList)
		{
			if (factionKey != info.GetFactionKey())
				continue;

			return info.GetXpReward();
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	int GetSeizeCompletionXpReward(SCR_ECampaignSeizingBaseType baseType)
	{
		foreach (SCR_SeizeCompletionXpRewardInfo info : m_aSeizeCompletionXpRewardList)
		{
			if (baseType != info.GetBaseType())
				continue;

			return info.GetXpReward();
		}

		return 0;
	}
}

[BaseContainerProps()]
class SCR_SeizeCompletionXpRewardInfo
{
	[Attribute(defvalue: SCR_ECampaignSeizingBaseType.FOB.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Seized base type.", enumType: SCR_ECampaignSeizingBaseType)]
	protected SCR_ECampaignSeizingBaseType m_eBaseType;

	[Attribute("0", params: "0 inf", desc: "XP reward.")]
	protected int m_iXPReward;

	//------------------------------------------------------------------------------------------------
	SCR_ECampaignSeizingBaseType GetBaseType()
	{
		return m_eBaseType;
	}

	//------------------------------------------------------------------------------------------------
	int GetXpReward()
	{
		return m_iXPReward;
	}
}

[BaseContainerProps()]
class SCR_SeizeProgressionXpRewardInfo
{
	[Attribute(desc: "Faction key of the seized base.")]
	protected string m_sFactionKey;

	[Attribute("0", params: "0 inf", desc: "XP reward per seizing period")]
	protected int m_iXPReward;

	//------------------------------------------------------------------------------------------------
	string GetFactionKey()
	{
		return m_sFactionKey;
	}

	//------------------------------------------------------------------------------------------------
	int GetXpReward()
	{
		return m_iXPReward;
	}
}

enum SCR_ECampaignSeizingBaseType
{
	FOB,
	SOURCE_BASE,
	CONTROL_POINT
}
