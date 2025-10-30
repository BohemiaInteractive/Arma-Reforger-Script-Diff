//------------------------------------------------------------------------------------------------
//! Config template for XP rewards
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EXPRewards, "m_eRewardID")]
class SCR_XPRewardInfo
{	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "ID of this reward.", enums: ParamEnumArray.FromEnum(SCR_EXPRewards))]
	protected SCR_EXPRewards m_eRewardID;
	
	//[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, desc: "Corresponding skill.", enums: ParamEnumArray.FromEnum(EProfileSkillID))]
	//protected EProfileSkillID m_eSkillID;
	
	[Attribute("Reward name", desc: "Name of this reward.")]
	protected string m_sRewardName;

	[Attribute("10", desc: "Amount of XP awarded.")]
	protected int m_iRewardXP;
	
	[Attribute("1", desc: "Toggles UI feedback for XP change.")]
	protected bool m_bAllowNotification;
	
	//------------------------------------------------------------------------------------------------
	SCR_EXPRewards GetRewardID()
	{
		return m_eRewardID;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetRewardName()
	{
		return m_sRewardName;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRewardXP()
	{
		return m_iRewardXP;
	}
	
	//------------------------------------------------------------------------------------------------
	/*EProfileSkillID GetRewardSkill()
	{
		return m_eSkillID;
	}*/
	
	//------------------------------------------------------------------------------------------------
	bool AllowNotification()
	{
		return m_bAllowNotification;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_XPRewardList
{
	[Attribute()]
	protected ref array<ref SCR_XPRewardInfo> m_aRewardList;
	
	//------------------------------------------------------------------------------------------------
	void GetRewardList(out notnull array<ref SCR_XPRewardInfo> rewardList)
	{
		foreach (SCR_XPRewardInfo info : m_aRewardList)
		{
			rewardList.Insert(info);
		}
	}
};