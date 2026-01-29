[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_GroupSizeTooltipDetail : SCR_EntityTooltipDetail
{
	protected TextWidget m_Text;
	protected ProgressBarWidget m_Bar;
	
	//------------------------------------------------------------------------------------------------
	override bool NeedUpdate()
	{
		return m_Text != null;
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		SCR_EditableGroupComponent group = SCR_EditableGroupComponent.Cast(entity);
		if (!group)
			return;
		
		if (m_Text)
			m_Text.SetText(group.GetSize().ToString());
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_Text = TextWidget.Cast(widget);
		return m_Text != null;
		
		//--- ToDo: Progress bar to show status compared to initial group size
		//m_Value = TextWidget.Cast(m_Widget.FindAnyWidget("Value"));
		//m_Bar = ProgressBarWidget.Cast(m_Widget.FindAnyWidget("Bar"));
		//return m_Value || m_Bar;
	}
}
