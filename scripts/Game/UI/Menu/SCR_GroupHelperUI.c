class SCR_GroupHelperUI
{
	protected const string CUSTOM_GROUP_NAME_FORMAT = "#AR-Player_Groups_CustomName_Format";

	//------------------------------------------------------------------------------------------------
	//! This function can only be used in UI menu, where to change the language you need to close and open the shown menu, so it will be renewed.
	//! \param[in] group
	//! \return custom name if is allowed and translated group name
	static string GetTranslatedGroupName(notnull SCR_AIGroup group)
	{
		string company, platoon, squad, character, format;
		group.GetCallsigns(company, platoon, squad, character, format);
		string originalName = WidgetManager.Translate(format, company, platoon, squad, character);

		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return originalName;

		string customName = group.GetCustomName();
		if (customName.IsEmpty())
			return originalName;

		return WidgetManager.Translate(CUSTOM_GROUP_NAME_FORMAT, customName, originalName);
	}

	//------------------------------------------------------------------------------------------------
	//! This function can only be used in UI menu, where to change the language you need to close and open the shown menu, so it will be renewed.
	//! \param[in] group
	//! \returns custom name if is allowed and translated group name and group role
	static string GetTranslatedGroupNameAndRoleName(notnull SCR_AIGroup group)
	{
		string groupRoleName = group.GetGroupRoleName();
		if (groupRoleName.IsEmpty() || group.GetGroupRole() == SCR_EGroupRole.NONE)
			return GetTranslatedGroupName(group);

		return WidgetManager.Translate(CUSTOM_GROUP_NAME_FORMAT, GetTranslatedGroupName(group), groupRoleName);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	//! \returns group role
	static string GetRoleName(notnull SCR_AIGroup group)
	{
		string groupRoleName = group.GetGroupRoleName();
		if (group.GetGroupRole() == SCR_EGroupRole.NONE)
			return string.Empty;

		return groupRoleName;
	}
}
