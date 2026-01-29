class SCR_ContextMenuButtonEditorUIComponent : ScriptedWidgetComponent
{
	[Attribute()]
	string m_sButtonName;

	[Attribute()]
	string m_sButtonIconName;

	[Attribute()]
	string m_sShortcutTextName;
	
	private Widget m_RootWidget;
	private ImageWidget m_ButtonIconWidget;
	private TextWidget m_ShortcutTextWidget;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] info
	//! \param[in] shortcutAction
	//! \param[in] canBePerformed
	void SetData(SCR_UIInfo info, string shortcutAction, bool canBePerformed)
	{
		SCR_ButtonTextComponent buttonTextComponent = SCR_ButtonTextComponent.Cast(m_RootWidget.FindHandler(SCR_ButtonTextComponent));
		buttonTextComponent.SetText(info.GetName());
		buttonTextComponent.SetEnabled(canBePerformed);
		info.SetIconTo(m_ButtonIconWidget);
		SetShortcutText(shortcutAction);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetShortcutText(string shortcutAction)
	{		
		string shortcutText = string.Empty;
		if (shortcutAction != string.Empty)
		{
			shortcutText = string.Format("<action name='%1'/>", shortcutAction);
		}
		
		m_ShortcutTextWidget.SetText(shortcutText);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_RootWidget = w;
		m_ButtonIconWidget = ImageWidget.Cast(w.FindAnyWidget(m_sButtonIconName));
		m_ShortcutTextWidget = TextWidget.Cast(w.FindAnyWidget(m_sShortcutTextName));
	}
}
