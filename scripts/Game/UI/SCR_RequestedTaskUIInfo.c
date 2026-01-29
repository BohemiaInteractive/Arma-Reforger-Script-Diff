[BaseContainerProps()]
class SCR_RequestedTaskUIInfo : SCR_TaskUIInfo
{
	protected int m_iRequesterGroupId;

	//------------------------------------------------------------------------------------------------
	void SetRequesterGroupId(int groupId)
	{
		m_iRequesterGroupId = groupId;
	}

	//------------------------------------------------------------------------------------------------
	override bool SetDescriptionTo(TextWidget textWidget)
	{
		if (!textWidget)
			return false;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;

		SCR_AIGroup requesterGroup = groupsManager.FindGroup(m_iRequesterGroupId);
		if (!requesterGroup)
			return false;

		Faction faction = requesterGroup.GetFaction();
		if (!faction)
			return false;

		string company, platoon, squad, character, format;
		requesterGroup.GetCallsigns(company, platoon, squad, character, format);

		// This translate is used in UI menu SCR_TaskListEntryDescriptionUIComponent, where to change the language you need to close and open the shown menu, so it will be renewed.
		string translatedCallsign = WidgetManager.Translate(format, company, platoon, squad);
		textWidget.SetTextFormat(Description, translatedCallsign);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Create SCR_RequestedTaskUIInfo from basic params.
	//! \param[in] name
	//! \param[in] nameParams
	//! \param[in] description
	//! \param[in] descParams
	//! \param[in] icon
	//! \param[in] iconSetName
	//! \param[in] requesterGroupId
	//! \return SCR_RequestedTaskUIInfo class
	static SCR_RequestedTaskUIInfo CreateRequestedTaskUIInfo(
		LocalizedString name,
		array<LocalizedString> nameParams,
		LocalizedString description,
		array<LocalizedString> descParams,
		ResourceName icon = ResourceName.Empty,
		string iconSetName = string.Empty,
		int requesterGroupId = 0)
	{
		SCR_RequestedTaskUIInfo info = new SCR_RequestedTaskUIInfo();
		info.Name = name;
		info.Description = description;
		info.Icon = icon;
		info.IconSetName = iconSetName;
		info.m_iRequesterGroupId = requesterGroupId;

		if (nameParams && !nameParams.IsEmpty())
		{
			info.m_aNameParameters = {};
			info.m_aNameParameters.Copy(nameParams);
		}

		if (descParams && !descParams.IsEmpty())
		{
			info.m_aDescriptionParameters = {};
			info.m_aDescriptionParameters.Copy(descParams);
		}

		return info;
	}
}
