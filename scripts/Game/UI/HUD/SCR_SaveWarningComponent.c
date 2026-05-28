class SCR_SaveWarningComponent: ScriptedWidgetComponent
{
	protected Widget m_wRoot;
	protected SCR_LoadingSpinner m_LoadingSpinner;
	protected RichTextWidget m_Text;
	
	//---------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		
		m_LoadingSpinner = SCR_LoadingSpinner.Cast(w.FindAnyWidget("Spinner").FindHandler(SCR_LoadingSpinner));
		m_Text = RichTextWidget.Cast(w.FindAnyWidget("m_wText"));
	}
	
	void UpdateSpinner(float deltaTime)
	{
		m_LoadingSpinner.Update(deltaTime);
	}
	
	void SetText(string text)
	{
		m_Text.SetText(text);
	}
	
	void ShowWarning(bool state)
	{
		m_wRoot.SetVisible(state);
	}
	
	bool IsSaving()
	{
		return m_wRoot.IsVisible();
	}
}