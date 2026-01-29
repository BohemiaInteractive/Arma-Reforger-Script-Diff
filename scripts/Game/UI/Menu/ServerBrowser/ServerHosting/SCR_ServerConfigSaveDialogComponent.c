class SCR_ServerConfigSaveDialog : SCR_ConfigurableDialogUi
{
	protected const int MIN_NAME_LENGTH = 5;
	
	protected const string WIDGET_FILE_NAME = "FileName";
	protected const string WIDGET_NAV_CONFIRM = "confirm";
	
	protected SCR_EditBoxComponent m_FileName;
	protected SCR_InputButtonComponent m_navConfirm;
	
	//------------------------------------------------------------------------------------------------
	override protected void Init(Widget root, SCR_ConfigurableDialogUiPreset preset, MenuBase proxyMenu)
	{
		super.Init(root, preset, proxyMenu);
		m_navConfirm = FindButton(WIDGET_NAV_CONFIRM);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_FileName = SCR_EditBoxComponent.GetEditBoxComponent(WIDGET_FILE_NAME, w);
		if (!m_FileName)
			return;
		
		m_FileName.m_OnChanged.Insert(OnFileNameChanged);
		
		Widget focus = m_FileName.GetRootWidget();
			if (focus)
				GetGame().GetWorkspace().SetFocusedWidget(focus);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFileNameText(string text)
	{
		if (!m_FileName)
			return;
		
		m_FileName.SetValue(text);
	}
	
	//------------------------------------------------------------------------------------------------
	string GetFileNameText()
	{
		if (!m_FileName)
			return "";
		
		return m_FileName.GetValue();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Allow name saving based on given text 
	protected void OnFileNameChanged(SCR_EditBoxComponent editBox, string text)
	{
		if (m_navConfirm)
			m_navConfirm.SetEnabled(text.Length() >= MIN_NAME_LENGTH);
	}
}