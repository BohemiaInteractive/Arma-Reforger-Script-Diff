[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_TaskCompleteDescriptionTooltipDetail : SCR_EntityTooltipDetail
{
	[Attribute()]
	protected LocalizedString m_sManualCompleteOnlyText;
	
	[Attribute(defvalue: "1 1 1 1", desc: "Color of images within the notification message")]
	protected ref Color m_cManualCompleteColor;
	
	protected SCR_Task m_Task;
	protected RichTextWidget m_Text;
	
	protected ref Color m_cDefaultColor;

	//------------------------------------------------------------------------------------------------
	override void UpdateDetail(SCR_EditableEntityComponent entity)
	{
		m_Text.SetText(m_Task.GetTaskDescription());
		m_Text.SetColor(m_cDefaultColor);
	}

	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{
		m_Text = RichTextWidget.Cast(widget);
		if (!m_Text)
			return false;
		
		m_Task = SCR_Task.Cast(entity.GetOwner());
		if (!m_Task)
			return false;

		m_cDefaultColor = m_Text.GetColor();
		
		return true;
	}
}
