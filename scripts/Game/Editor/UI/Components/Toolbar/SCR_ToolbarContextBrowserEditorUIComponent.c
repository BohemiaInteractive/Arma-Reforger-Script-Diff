class SCR_ToolbarContextBrowserEditorUIComponent : ScriptedWidgetComponent
{
	[Attribute("BrowserButton")]
	protected string m_sOpenContentBrowserButtonName;

	//------------------------------------------------------------------------------------------------
	protected void OpenAssetBrowserButton(Widget widget, float value, EActionTrigger actionTrigger)
	{
		SCR_ContentBrowserEditorComponent contentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent));
		if (contentBrowserManager)
			contentBrowserManager.OpenBrowser();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		Widget buttonWidget = w.FindAnyWidget(m_sOpenContentBrowserButtonName);
		if (buttonWidget)
			ButtonActionComponent.GetOnAction(buttonWidget).Insert(OpenAssetBrowserButton);
	}
}
