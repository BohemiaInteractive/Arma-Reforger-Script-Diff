[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_EntityTooltipDetail
{
	[Attribute()]
	protected LocalizedString m_sDisplayName;
	
	[Attribute(params: "layout")]
	protected ResourceName m_Layout;
	
	protected Widget m_Widget;
	
	[Attribute("1", "If false hides label")]
	protected bool m_bShowLabel;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool NeedUpdate()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \param[in] parent
	//! \param[in] label
	//! \param[in] setFrameslot
	//! \return
	bool CreateDetail(SCR_EditableEntityComponent entity, Widget parent, TextWidget label, bool setFrameslot = true)
	{
		if (label)
			label.SetText(m_sDisplayName);
		
		WorkspaceWidget workspace = parent.GetWorkspace();
		m_Widget = workspace.CreateWidgets(m_Layout, parent);
		
		if (InitDetail(entity, m_Widget))
		{
			return true;
		}
		else
		{
			m_Widget.RemoveFromHierarchy();
			m_Widget = null;
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	void UpdateDetail(SCR_EditableEntityComponent entity);

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \param[in] widget
	//! \return true to show the detail
	bool InitDetail(SCR_EditableEntityComponent entity, Widget widget);
	
	//------------------------------------------------------------------------------------------------
	//! \return if label should be shown
	bool GetShowLabel()
	{
		return m_bShowLabel;
	}
}
