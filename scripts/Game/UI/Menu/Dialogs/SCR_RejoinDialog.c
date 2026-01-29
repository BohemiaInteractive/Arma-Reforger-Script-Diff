//------------------------------------------------------------------------------------------------
class SCR_RejoinDialog : SCR_ErrorDialog
{
	// Widget Names 
	const string WIDGET_TXT_TIMER = "Timer";
	const string WIDGET_LOADING = "Loading";
	
	// Widgets 
	protected TextWidget m_wTimer;
	protected Widget m_wLoading;
	
	protected int m_iTimer;
	
	// Invokers 
	protected ref ScriptInvoker<int> Event_OnTimerChanged;

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnTimerChanged(int arg0)
	{
		if (Event_OnTimerChanged)
			Event_OnTimerChanged.Invoke(this, arg0);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnTimerChanged()
	{
		if (!Event_OnTimerChanged)
			Event_OnTimerChanged = new ScriptInvoker();

		return Event_OnTimerChanged;
	}

	//------------------------------------------------------------------------------------------------
	// Dialog override functions 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wTimer = TextWidget.Cast(w.FindAnyWidget(WIDGET_TXT_TIMER));
		m_wLoading = w.FindAnyWidget(WIDGET_LOADING);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DecreaseTime()
	{
		m_iTimer--;
		if (m_iTimer > 0)
			GetGame().GetCallqueue().CallLater(DecreaseTime, 1000);
		
		InvokeEventOnTimerChanged(m_iTimer);
	}
	
	//------------------------------------------------------------------------------------------------
	// Public API
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void RunTimer(bool run)
	{
		// Stop
		GetGame().GetCallqueue().Remove(DecreaseTime);
		
		// Run next cycle
		if (run)
			GetGame().GetCallqueue().CallLater(DecreaseTime, 1000);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTimer(int time)
	{
		m_iTimer = time;
		InvokeEventOnTimerChanged(m_iTimer);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTimer()
	{
		return m_iTimer;
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowLoading(bool show)
	{
		if (m_wLoading)
			m_wLoading.SetVisible(show);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_RejoinDialog FindRejoinComponent(notnull Widget w)
	{
		return SCR_RejoinDialog.Cast(w.FindHandler(SCR_RejoinDialog));
	}
};