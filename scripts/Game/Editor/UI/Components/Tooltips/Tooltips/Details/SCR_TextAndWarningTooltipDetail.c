[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_TextAndWarningTooltipDetail : SCR_EntityTooltipDetail
{
	[Attribute()]
	protected LocalizedString m_sWarningText;

	[Attribute(desc: "Optional")]
	protected ResourceName m_WarningIcon;

	[Attribute("Text")]
	protected string m_sTextWidgetName;

	[Attribute("WarningText")]
	protected string m_sWarningTextName;

	[Attribute("WarningIcon")]
	protected string m_sWarningImageName;
	
	protected TextWidget m_Text;
	protected TextWidget m_WarningText;
	protected ImageWidget m_WarningImage;
	
	//------------------------------------------------------------------------------------------------
	override bool InitDetail(SCR_EditableEntityComponent entity, Widget widget)
	{		
		m_Text = TextWidget.Cast(widget.FindAnyWidget(m_sTextWidgetName));
		if (!m_Text)
			return false;
		
		m_WarningText = TextWidget.Cast(widget.FindAnyWidget(m_sWarningTextName));
		if (!m_WarningText)
			return false;
		else 
			m_WarningText.SetText(m_sWarningText);
		
		m_WarningImage = ImageWidget.Cast(widget.FindAnyWidget(m_sWarningImageName));
		if (m_WarningImage)
		{
			if (m_WarningIcon != string.Empty)
			{
				m_WarningImage.LoadImageTexture(0, m_WarningIcon);
				m_WarningImage.SetImage(0);
			}
			else 
			{
				m_WarningImage.SetVisible(false);
			}
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] showWarning
	void ToggleWarning(bool showWarning)
	{
		if (m_WarningImage && m_WarningIcon != string.Empty)
			m_WarningImage.SetVisible(showWarning);

		if (m_WarningText)
			m_WarningText.SetVisible(showWarning);
		
		m_Text.SetVisible(!showWarning);
	}
}
