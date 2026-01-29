[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EXPRewards, "m_eRewardID")]
class SCR_CommanderAdditionalXPRewardInfo : SCR_XPRewardInfo
{
	[Attribute("0.1", UIWidgets.EditBox, params: "0 1 0.01", desc: "What fraction of gained player XP should the Commander be awarded.")]
	protected float m_fRewardXPAmountFraction;

	[Attribute(SCR_EXPRewards.UNDEFINED.ToString(), uiwidget: UIWidgets.ComboBox, desc: "List of XP awards that should award a fraction of gained XP to Commander.", enumType: SCR_EXPRewards)]
	protected ref array<SCR_EXPRewards> m_aRewardList;

	//------------------------------------------------------------------------------------------------
	float GetRewardXPAmountFraction()
	{
		return m_fRewardXPAmountFraction;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] rewardId
	//! \return whether the reward list contains the reward or not
	bool IsXPRewardListed(SCR_EXPRewards rewardId)
	{
		return m_aRewardList.Contains(rewardId);
	}
}