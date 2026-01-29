class SCR_AssignedGroupLabelUIComponent : SCR_ScriptedWidgetComponent
{	
	[Attribute("{1228936E5DB30403}UI/Textures/GroupManagement/FlagIcons/GroupFlagsBlufor.imageset", params: "layout")]
	protected ResourceName m_sGroupsBLUFOR;
	
	[Attribute("{7CD99D22C7AE8195}UI/Textures/GroupManagement/FlagIcons/GroupFlagsOpfor.imageset", params: "layout")]
	protected ResourceName m_sGroupsOPFOR;
	
	[Attribute("{301FC1A2A46D3E0D}UI/Textures/GroupManagement/FlagIcons/GroupFlagsIndfor.imageset", params: "layout")]
	protected ResourceName m_sGroupsINDFOR;
	
	[Attribute("{D7E06968B5F9FC92}UI/Textures/GroupManagement/FlagIcons/GroupFlagsCivilian.imageset", params: "layout")]
	protected ResourceName m_sGroupsCIVILIAN;
	 
	protected ref SCR_AssignedGroupLabelWidgets m_Widgets = new SCR_AssignedGroupLabelWidgets();
	
	protected const string DEFAULT_ICON_NAME = "undefined";
	protected const string CUSTOM_GROUP_NAME_FORMAT = "#AR-Player_Groups_CustomName_Format";
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_Widgets.Init(w);
	}
	
	//------------------------------------------------------------------------------------------------
	void LoadGroupFlag(notnull SCR_AIGroup aiGroup)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;
		
		ResourceName flag = aiGroup.GetGroupFlag();
		if (flag.IsEmpty())
			return;
		
		if (aiGroup.GetFlagIsFromImageSet())
			SetFlagButtonFromImageSet(flag);
		else
			m_Widgets.m_wGroupIcon.LoadImageTexture(0, flag, false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets flag visuals
	//! \param[in] name of flag in imagest
	protected void SetFlagButtonFromImageSet(string name)
	{	
		if (name.IsEmpty())
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;
		
		SCR_Faction playerFaction = SCR_Faction.Cast(factionManager.GetLocalPlayerFaction());
		if (!playerFaction)
			return;		
		
		ResourceName res = playerFaction.GetGroupFlagImageSet();
		if (!res)
			return;
		
		m_Widgets.m_wGroupIcon.LoadImageFromSet(0, res, name);
	}
	
	//------------------------------------------------------------------------------------------------
	//!	Set formated group name based on group callsign data
	//! \param[in] aiGroup to have name loaded
	void LoadGroupName(notnull SCR_AIGroup aiGroup)
	{
		string groupName = aiGroup.GetCustomNameWithOriginal();
		string company, platoon, squad, character, format;
		aiGroup.GetCallsigns(company, platoon, squad, character, format);
		
		if (groupName.IsEmpty())
		{
			m_Widgets.m_wGroupName.SetTextFormat(format, company, platoon, squad, character);	
		}
		else
		{
			string customName = aiGroup.GetCustomName().Trim();
			
			if (customName.IsEmpty())
				m_Widgets.m_wGroupName.SetTextFormat(format, company, platoon, squad, character);
			else
				m_Widgets.m_wGroupName.SetText(SCR_GroupHelperUI.GetTranslatedGroupName(aiGroup));
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] name of group to be set
	void SetGroupName(string name)
	{	
		if (name.IsEmpty())
			return;
		
		m_Widgets.m_wGroupName.SetText(name);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupIcon(ResourceName imageset, string iconName)
	{
		m_Widgets.m_wGroupIcon.LoadImageFromSet(0, imageset, iconName);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets group icon based on its faction
	//! \param[in] faction of group
	//! \param[in] iconName of icon in imageset
	void SetGroupIconFromFaction(Faction faction, string iconName = string.Empty)
	{	
		if (!iconName)
			iconName = DEFAULT_ICON_NAME;
		
		ResourceName imageset;
		switch (faction.GetFactionKey())
		{
			case "US":
			{	
				imageset = m_sGroupsBLUFOR;
				break;
			}
			
			case "USSR":
			{	
				imageset = m_sGroupsOPFOR;
				break;
			}
			
			case "FIA":
			{	
				imageset = m_sGroupsINDFOR;
				break;
			}
			
			case "CIV":
			{	
				imageset = m_sGroupsCIVILIAN;
				break;
			}
			
			default:
			{
				imageset = m_sGroupsBLUFOR;
			}
		}
		
		m_Widgets.m_wGroupIcon.LoadImageFromSet(0, imageset, iconName);
	}
}
