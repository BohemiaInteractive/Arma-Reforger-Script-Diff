class SCR_TaskAssignGroupListEntryUIComponent : SCR_ScriptedWidgetComponent
{
	protected ref SCR_TaskAssignGroupListEntryWidgets m_Widgets = new SCR_TaskAssignGroupListEntryWidgets();

	protected int m_iGroupID;
	protected SCR_AIGroup m_Group;
	protected bool m_bIsAssignmentChangedByPlayer;
	protected SCR_ModularButtonComponent m_ButtonComponent;

	protected const string GROUP_ROLE_FORMAT = "(%1)";

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_Widgets.Init(m_wRoot);
		m_ButtonComponent = SCR_ModularButtonComponent.Cast(m_wRoot.FindHandler(SCR_ModularButtonComponent));
		if (m_ButtonComponent)
			m_ButtonComponent.m_OnToggled.Insert(OnAssignCheckerToggled);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		if (m_ButtonComponent)
			m_ButtonComponent.m_OnToggled.Remove(OnAssignCheckerToggled);
	}

	//------------------------------------------------------------------------------------------------
	//!	Init entry
	//! \param[in] group
	void InitEntry(notnull SCR_AIGroup group)
	{
		m_Group = group;
		m_iGroupID = group.GetGroupID();
	}

	//------------------------------------------------------------------------------------------------
	//! Update entry
	void UpdateEntry()
	{
		if (!m_Group)
			return;

		string company, platoon, squad, character, format;
		m_Group.GetCallsigns(company, platoon, squad, character, format);
		m_Widgets.m_wGroupName.SetTextFormat(format, company, platoon, squad, character);

		m_Widgets.m_wGroupRoleName.SetTextFormat(GROUP_ROLE_FORMAT, m_Group.GetGroupRoleName());
		LoadGroupFlag(m_Group);
	}

	//------------------------------------------------------------------------------------------------
	void SetFocus()
	{
		if (m_ButtonComponent)
			GetGame().GetWorkspace().SetFocusedWidget(m_wRoot);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAssignCheckerToggled(SCR_ModularButtonComponent modularButton, bool isOn)
	{
		SetAssignmentChangedByPlayer(true);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] isChanged
	void SetAssignmentChangedByPlayer(bool isChanged)
	{
		m_bIsAssignmentChangedByPlayer = isChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if player changed assignment settings
	bool IsAssignmentChangedByPlayer()
	{
		return m_bIsAssignmentChangedByPlayer;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] assigned
	void SetAssignmentToggle(bool assigned)
	{
		m_ButtonComponent.SetToggled(assigned, invokeOnToggled: false);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetAssignmentToggle()
	{
		return m_ButtonComponent.GetToggled();
	}

	//------------------------------------------------------------------------------------------------
	//! Update task icon
	//! \param[in] task
	void UpdateTaskIcon(SCR_Task task)
	{
		if (task)
		{
			if (!task.GetTaskIconPath() || !task.GetTaskIconSetName())
				return;

			m_Widgets.m_wAssignedTaskIcon.LoadImageFromSet(0, task.GetTaskIconPath(), task.GetTaskIconSetName());
			m_Widgets.m_wAssignedTaskOverlay.SetVisible(true);
		}
		else
		{
			m_Widgets.m_wAssignedTaskOverlay.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadGroupFlag(notnull SCR_AIGroup aiGroup)
	{
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
	//! \param[in] name of flag in imageset
	protected void SetFlagButtonFromImageSet(string name)
	{
		if (name.IsEmpty())
			return;

		SCR_Faction playerFaction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		if (!playerFaction)
			return;

		ResourceName res = playerFaction.GetGroupFlagImageSet();
		if (!res)
			return;

		m_Widgets.m_wGroupIcon.LoadImageFromSet(0, res, name);
	}
}
