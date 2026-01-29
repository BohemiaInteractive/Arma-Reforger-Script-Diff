[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_DescriptionTooltipDetail : SCR_EntityTooltipDetail
{
	protected RichTextWidget m_Text;
	
	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		entity.GetInfo().SetDescriptionTo(m_Text);
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_Text = RichTextWidget.Cast(widget);
		
		return m_Text && entity.GetInfo() && entity.GetInfo().HasDescription();
	}
}
