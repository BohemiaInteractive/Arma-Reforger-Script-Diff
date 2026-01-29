[BaseContainerProps(configRoot: true)]
class SCR_GroupTaskRelevanceConfig
{
	[Attribute(desc: "Group task relevance list")]
	protected ref array<ref SCR_GroupTaskRelevance> m_aGroupTaskRelevanceList;

	//------------------------------------------------------------------------------------------------
	//! \param[in] taskTypename
	//! \return group task relevance
	SCR_GroupTaskRelevance GetGroupTaskRelevance(typename taskTypename)
	{
		if (!taskTypename || taskTypename == typename.Empty)
			return null;

		foreach (SCR_GroupTaskRelevance groupTaskRelevance : m_aGroupTaskRelevanceList)
		{
			if (taskTypename == groupTaskRelevance.GetTaskTypename())
				return groupTaskRelevance;
		}

		return null;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sTaskTypename")]
class SCR_GroupTaskRelevance
{
	[Attribute(desc: "Typename of SCR_Task class")]
	protected string m_sTaskTypename;

	[Attribute(desc: "List with relevant group roles for the task defined by typename")]
	protected ref array<ref SCR_GroupRoleRelevance> m_aOrderedGroupRoleRelevanceList;

	//------------------------------------------------------------------------------------------------
	//! \return task typename
	typename GetTaskTypename()
	{
		return m_sTaskTypename.ToType();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[out] relevanceList
	//! \return
	int GetOrderedGroupRoleRelevanceList(out array<ref SCR_GroupRoleRelevance> relevanceList)
	{
		relevanceList = m_aOrderedGroupRoleRelevanceList;

		return m_aOrderedGroupRoleRelevanceList.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	//! \return true if the task is relevant and visible for the group
	bool IsTaskVisibleForGroup(notnull SCR_AIGroup group)
	{
		SCR_EGroupRole groupRole = group.GetGroupRole();
		foreach (SCR_GroupRoleRelevance groupRoleRelevance : m_aOrderedGroupRoleRelevanceList)
		{
			if (groupRoleRelevance.GetGroupRole() == groupRole)
				return groupRoleRelevance.IsTaskVisible();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	//! \return true if the task is relevant and executable by the group
	bool IsTaskExecutableByGroup(notnull SCR_AIGroup group)
	{
		SCR_EGroupRole groupRole = group.GetGroupRole();
		foreach (SCR_GroupRoleRelevance groupRoleRelevance : m_aOrderedGroupRoleRelevanceList)
		{
			if (groupRoleRelevance.GetGroupRole() == groupRole)
				return groupRoleRelevance.IsGroupExecutor();
		}

		return false;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EGroupRole, "m_eGroupRole")]
class SCR_GroupRoleRelevance
{
	[Attribute(uiwidget: UIWidgets.ComboBox, enumType: SCR_EGroupRole)]
	protected SCR_EGroupRole m_eGroupRole;

	[Attribute("1", desc: "Task visibility for this group role")]
	protected bool m_bIsTaskVisible;

	[Attribute("1", desc: "Task executability for this group role")]
	protected bool m_bIsGroupExecutor;

	//------------------------------------------------------------------------------------------------
	//! \return group role
	SCR_EGroupRole GetGroupRole()
	{
		return m_eGroupRole;
	}

	//------------------------------------------------------------------------------------------------
	//! \return task visibility for this group role
	bool IsTaskVisible()
	{
		return m_bIsTaskVisible;
	}

	//------------------------------------------------------------------------------------------------
	//! \return task executability for this group role
	bool IsGroupExecutor()
	{
		return m_bIsGroupExecutor;
	}
}
