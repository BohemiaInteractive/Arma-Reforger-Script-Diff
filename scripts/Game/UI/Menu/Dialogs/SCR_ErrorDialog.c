//------------------------------------------------------------------------------------------------
class SCR_ErrorDialog : ScriptedWidgetComponent
{
	// Widget Names 
	const string WIDGET_TEXT_DETAIL = "TxtDetail";
	const string WIDGET_IMG_LINE = "ImgLine";
	
	// Widgets 
	protected TextWidget m_wTxtDetail;
	protected ImageWidget m_wImgLine;
	
	protected string m_sErrorMessage;

	//------------------------------------------------------------------------------------------------
	// Dialog override functions 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Get widgets 
		m_wTxtDetail = TextWidget.Cast(w.FindAnyWidget(WIDGET_TEXT_DETAIL));
		m_wImgLine = ImageWidget.Cast(w.FindAnyWidget(WIDGET_IMG_LINE));
		
		if (m_wTxtDetail)
			m_wTxtDetail.SetVisible(false);
		
		if (m_wImgLine)
			m_wImgLine.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	// Public API
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void SetErrorMessage(string message)
	{
		m_sErrorMessage = message;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetErrorMessage()
	{
		return m_sErrorMessage;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetErrorDetail(string strDetail)
	{	
		// Setup text 
		if (m_wTxtDetail)
		{
			m_wTxtDetail.SetText(strDetail);
			m_wTxtDetail.SetVisible(true);
		}
		
		// Show line
		if (m_wImgLine)
			m_wImgLine.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ErrorDialog FindErrorComponent(notnull Widget w)
	{
		return SCR_ErrorDialog.Cast(w.FindHandler(SCR_ErrorDialog));
	}
};



