class SCR_ToolbarItemNotificationEditorUIComponent : ScriptedWidgetComponent
{
	[Attribute("Icon")]
	protected string m_sIconWidgetName;

	[Attribute("Number")]
	protected string m_sNumberWidgetName;

	[Attribute()]
	protected ResourceName m_sDefaultIcon;
	
	protected Widget m_root;
	protected ImageWidget m_IconWidget;
	protected TextWidget m_NumberWidget;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] number
	//! \param[in] info
	void SetNotification(int number, SCR_UIInfo info = null)
	{
		//Hide
		if (number <= 0)
		{
			m_root.SetVisible(false);
		}
		//Custom image if one notification
		else if (number == 1 && info && info.SetIconTo(m_IconWidget))
		{
			m_NumberWidget.SetVisible(false);
			m_root.SetVisible(true);
		}
		//Default icon
		else 
		{
			m_IconWidget.LoadImageTexture(0, m_sDefaultIcon);
			m_IconWidget.SetImage(0);
			m_NumberWidget.SetText(number.ToString());
			m_NumberWidget.SetVisible(true);
			m_root.SetVisible(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_root = w;
		m_IconWidget = ImageWidget.Cast(w.FindAnyWidget(m_sIconWidgetName));
		m_NumberWidget = TextWidget.Cast(w.FindAnyWidget(m_sNumberWidgetName));
		m_root.SetVisible(false);
	}
}
