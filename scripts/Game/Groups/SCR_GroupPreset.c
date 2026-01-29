[BaseContainerProps()]
class SCR_GroupPreset
{	
	[Attribute(desc: "Name of the Group")]
	protected string m_sGroupName;
	
	[Attribute(defvalue:"0", uiwidget: UIWidgets.ComboBox, desc: "Group role", params: "", enumType: SCR_EGroupRole)]
	protected SCR_EGroupRole m_eGroupRole;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Flag icon of this particular group.", params: "edds")]
	private ResourceName m_sGroupFlag;
	
	[Attribute(desc: "Name of item in imageset")]
	protected string m_sGroupFlagName;

	[Attribute(desc: "Description of the Group")]
	protected string m_sGroupDescription;
	
	[Attribute(desc: "Count of group members.")]
	protected int m_iGroupSize;
	
	[Attribute(desc: "Radio frequency for communication in kHz.")]
	protected int m_iRadioFrequency;
	
	[Attribute("0", params: "0 inf", desc: "Index of default active radio channel of group")]
	protected int m_iDefaultActiveRadioChannel;
	
	[Attribute(defvalue: SCR_ECharacterRank.PRIVATE.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Rank required to create and join the group", params: "", enumType: SCR_ECharacterRank)]
	protected SCR_ECharacterRank m_eRequiredRank;

	[Attribute(desc: "Group is private.")]
	protected bool m_bIsPrivate;

	[Attribute("1", desc: "Is privacy changeable")]
	protected bool m_bIsPrivacyChangeable;
		
	//------------------------------------------------------------------------------------------------	
	void SetupGroup(SCR_AIGroup group)
	{
		group.SetCustomName(m_sGroupName, 0);
		group.SetRadioFrequency(m_iRadioFrequency);
		group.SetMaxGroupMembers(m_iGroupSize);
		group.SetPrivate(m_bIsPrivate);
		group.SetCustomDescription(m_sGroupDescription, 0);
		group.SetPrivacyChangeable(m_bIsPrivacyChangeable);
		group.SetRequiredRank(m_eRequiredRank);
		
		if (!m_sGroupFlag.IsEmpty())
			group.SetCustomGroupFlag(m_sGroupFlag);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	//! \param[in] overridingGroupPreset
	void SetupGroupWithOverride(SCR_AIGroup group, SCR_GroupPreset overridingGroupPreset)
	{
		if (!overridingGroupPreset.m_sGroupName.IsEmpty())
			group.SetCustomName(overridingGroupPreset.m_sGroupName, 0);
		else
			group.SetCustomName(m_sGroupName, 0);

		if (overridingGroupPreset.m_iRadioFrequency > 0)
			group.SetRadioFrequency(overridingGroupPreset.m_iRadioFrequency);
		else
			group.SetRadioFrequency(m_iRadioFrequency);

		if (overridingGroupPreset.m_iGroupSize > 0)
			group.SetMaxGroupMembers(overridingGroupPreset.m_iGroupSize);
		else
			group.SetMaxGroupMembers(m_iGroupSize);

		if (overridingGroupPreset.m_bIsPrivate)
			group.SetPrivate(overridingGroupPreset.m_bIsPrivate);
		else
			group.SetPrivate(m_bIsPrivate);

		if (!overridingGroupPreset.m_sGroupDescription.IsEmpty())
			group.SetCustomDescription(overridingGroupPreset.m_sGroupDescription, 0);
		else
			group.SetCustomDescription(m_sGroupDescription, 0);

		if (!overridingGroupPreset.GetGroupFlag().IsEmpty())
			group.SetCustomGroupFlag(overridingGroupPreset.GetGroupFlag());
		else if (!GetGroupFlag().IsEmpty())
			group.SetCustomGroupFlag(GetGroupFlag());

		if (!overridingGroupPreset.m_bIsPrivacyChangeable)
			group.SetPrivacyChangeable(overridingGroupPreset.m_bIsPrivacyChangeable);
		else
			group.SetPrivacyChangeable(m_bIsPrivacyChangeable);

		if (overridingGroupPreset.GetRequiredRank() != SCR_ECharacterRank.INVALID)
			group.SetRequiredRank(overridingGroupPreset.m_eRequiredRank);
		else
			group.SetRequiredRank(m_eRequiredRank);

		if (overridingGroupPreset.GetDefaultActiveRadioChannel() > 0)
			group.SetDefaultActiveRadioChannel(overridingGroupPreset.GetDefaultActiveRadioChannel());
		else
			group.SetDefaultActiveRadioChannel(m_iDefaultActiveRadioChannel);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	//! \param[in] faction
	void SetupGroupFlag(SCR_AIGroup group, SCR_Faction faction)
	{
		if (!GetDefaultGroupFlagName().IsEmpty() && !faction.GetGroupFlagImageSet().IsEmpty())
		{
			array<string> flagNames = {};
			faction.GetFlagNames(flagNames);
			group.SetGroupFlag(flagNames.Find(GetDefaultGroupFlagName()), !faction.GetGroupFlagImageSet().IsEmpty());
		}
		else
		{
			if (group.GetGroupFlag().IsEmpty())
				group.SetGroupFlag(0, !faction.GetGroupFlagImageSet().IsEmpty());
		}
	}

	//Getters
	//------------------------------------------------------------------------------------------------	
	string GetGroupName()
	{
		return m_sGroupName;
	}
	
	//------------------------------------------------------------------------------------------------	
	ResourceName GetGroupFlag()
	{
		return m_sGroupFlag;
	}
	
	//------------------------------------------------------------------------------------------------	
	int GetGroupSize()
	{
		return m_iGroupSize;
	}
	
	//------------------------------------------------------------------------------------------------	
	int GetRadioFrequency()
	{
		return m_iRadioFrequency;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetDefaultActiveRadioChannel()
	{
		return m_iDefaultActiveRadioChannel;
	}
	
	//------------------------------------------------------------------------------------------------	
	string GetGroupDescription()
	{
		return m_sGroupDescription;
	}
	
	//------------------------------------------------------------------------------------------------	
	bool IsPrivate()
	{
		return m_bIsPrivate;
	}	
	
	//------------------------------------------------------------------------------------------------
	bool IsPrivacyChangeable()
	{
		return m_bIsPrivacyChangeable;
	}

	//------------------------------------------------------------------------------------------------
	//! \return group role
	SCR_EGroupRole GetGroupRole()
	{
		return m_eGroupRole;
	}

	//------------------------------------------------------------------------------------------------
	//! \return default group flag name
	string GetDefaultGroupFlagName()
	{
		return m_sGroupFlagName;
	}

	//------------------------------------------------------------------------------------------------
	//! \return required character rank
	SCR_ECharacterRank GetRequiredRank()
	{
		return m_eRequiredRank;
	}

	//Setters
	//------------------------------------------------------------------------------------------------
	
	void SetGroupName(string name)
	{
		m_sGroupName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupSize(int size)
	{
		m_iGroupSize = size;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRadioFrequency(int freq)
	{
		m_iRadioFrequency = freq;	
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsPrivate(bool privacy)
	{
		m_bIsPrivate = privacy;
	}
}
