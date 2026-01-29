//-------------------------------------------------------------------------------------------
//! Configurable editbox dialog base
class SCR_EditboxDialogUi : SCR_ConfigurableDialogUi
{
	const string WIDGET_EDITBOX = "EditBox";
	const string WIDGET_WARNING = "DialogWarning";
	const string WIDGET_TEXT_WARNING = "TxtDialogWarning";
	
	protected Widget m_wWarningMessage;
	protected TextWidget m_wTxtWarningMessage;
	
	protected SCR_EditBoxComponent m_Editbox;
	
	ref ScriptInvokerString m_OnTextChange = new ScriptInvokerString();
	ref ScriptInvokerString m_OnWriteModeLeave = new ScriptInvokerString();
	
	//-------------------------------------
	// Override functions 
	//-------------------------------------
	
	//----------------------------------------------------------------------------------------
	override protected void InitWidgets()
	{
		super.InitWidgets();
		
		// Get widget component
		Widget wEditbox = m_wRoot.FindAnyWidget(WIDGET_EDITBOX);
		if (wEditbox)
		{
			m_Editbox = SCR_EditBoxComponent.Cast(wEditbox.FindHandler(SCR_EditBoxComponent));
		}
		
		m_wWarningMessage = m_wRoot.FindAnyWidget(WIDGET_WARNING);
		if (m_wWarningMessage)
			m_wTxtWarningMessage = TextWidget.Cast(m_wWarningMessage.FindAnyWidget(WIDGET_TEXT_WARNING));
	}
	
	//----------------------------------------------------------------------------------------
	override protected void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen();

		if (m_Editbox)
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_Editbox.GetRootWidget());
			m_Editbox.m_OnTextChange.Insert(OnTextChange);
			m_Editbox.m_OnWriteModeLeave.Insert(OnWriteModeLeaveInternal);
		}
	}
	
	//----------------------------------------------------------------------------------------
	protected void OnWriteModeLeaveInternal(string text)
	{
		m_OnWriteModeLeave.Invoke(m_Editbox.GetValue());
	}
	
	//----------------------------------------------------------------------------------------
	protected void OnTextChange(string text)
	{
		m_OnTextChange.Invoke(text);
	}
	
	//-------------------------------------
	// Public functions 
	//-------------------------------------
	
	//----------------------------------------------------------------------------------------
	void DisplayWarningMessage(bool display)
	{
		if (m_wWarningMessage)
			m_wWarningMessage.SetVisible(display);
	}
	
	//----------------------------------------------------------------------------------------
	void SetWarningMessage(string message)
	{
		if (!m_wWarningMessage)
			return;
		
		m_wWarningMessage.SetVisible(!message.IsEmpty());
		
		if (m_wTxtWarningMessage)
			m_wTxtWarningMessage.SetText(message);
	}
	
	//-------------------------------------
	// Get and set 
	//-------------------------------------
	
	//----------------------------------------------------------------------------------------
	SCR_EditBoxComponent GetEditbox() 
	{ 
		return m_Editbox;
	}
};