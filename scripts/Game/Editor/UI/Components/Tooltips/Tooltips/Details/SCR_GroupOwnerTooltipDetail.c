[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_GroupOwnerTooltipDetail : SCR_EntityTooltipDetail
{
	protected TextWidget m_Text;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		SCR_EditableEntityComponent group = entity.GetAIGroup();
		if (group)
			m_Text.SetText(group.GetDisplayName());
		else
			m_Text.SetText("");
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_Text = TextWidget.Cast(widget);
		return m_Text != null;
	}
}
