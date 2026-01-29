//------------------------------------------------------------------------------------------------
class SCR_ProgressDialog : ScriptedWidgetComponent
{
	// Widget Names 
	const string WIDGET_TEXT_PROGRESS = "TxtProgress";
	const string WIDGET_PROGRESS_BAR = "ProgressDownload";
	
	// Widgets 
	protected TextWidget m_wTxtProgress;
	protected SCR_WLibProgressBarComponent m_ProgressBar;

	//------------------------------------------------------------------------------------------------
	// Dialog override functions 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Get widgets 
		m_wTxtProgress = TextWidget.Cast(w.FindAnyWidget(WIDGET_TEXT_PROGRESS));
		
		Widget wProgressBar = w.FindAnyWidget(WIDGET_PROGRESS_BAR);
		if (wProgressBar)
			m_ProgressBar = SCR_WLibProgressBarComponent.Cast(wProgressBar.FindHandler(SCR_WLibProgressBarComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	// Public API
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void SetProgress(float progress)
	{	
		// Progress bar 
		if (m_ProgressBar)
		{
			m_ProgressBar.SetValue(progress);
			
			// Get proper value in percent
			float progValue = m_ProgressBar.GetValue() - m_ProgressBar.GetMin();
			progValue /= m_ProgressBar.GetMax() - m_ProgressBar.GetMin();
			progValue *= 100;
			progValue = Math.Floor(progValue);
			
			// Progress text
			if (m_wTxtProgress) 
				m_wTxtProgress.SetText(progValue.ToString() + "%");
		}
	}
	
};



