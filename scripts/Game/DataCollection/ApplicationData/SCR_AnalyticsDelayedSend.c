[EnumLinear()]
enum SCR_EAnalyticsDelayedState
{
	INACTIVE,  // outside this menu
	IN_MENU,  // inside this menu
	OUT_MENU_WAITING,  // we just closed the menu
	OUT_MENU_SENDING  // it was a while since we closed the menu
}

class SCR_AnalyticsDelayedSend
{
	protected static const int ANALYTICS_DELAY_SECONDS = 10;
	protected SCR_EAnalyticsDelayedState m_eState = SCR_EAnalyticsDelayedState.INACTIVE;
	protected bool m_bIsAdditive = false;
	protected float m_fTimeSpentSeconds = 0;
	protected float m_fDelayLeftSeconds = 0;
	
	//------------------------------------------------------------------------------------------------
	//! Start this tracking time in this menu, adds it to previous time if we have not yet sended previous data
	void Start()
	{
		m_bIsAdditive = m_eState == SCR_EAnalyticsDelayedState.OUT_MENU_WAITING;
		m_eState = SCR_EAnalyticsDelayedState.IN_MENU;
		if (!m_bIsAdditive)
			m_fTimeSpentSeconds = 0;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Stop tracking time in this menu, prepare for sending data.
	void Stop()
	{
		m_eState = SCR_EAnalyticsDelayedState.OUT_MENU_WAITING;
		m_fDelayLeftSeconds = ANALYTICS_DELAY_SECONDS;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Are we adding the time to previous time, or starting fresh?
	//! \return true if we are adding time to previous time, false otherwise.
	bool IsAdditive()
	{
		return m_bIsAdditive;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates the delayed send by N seconds
	//! \param[in] deltaSeconds
	void Update(float deltaSeconds)
	{
		switch (m_eState)
		{
			case SCR_EAnalyticsDelayedState.INACTIVE:
				break;
			
			case SCR_EAnalyticsDelayedState.IN_MENU:
				m_fTimeSpentSeconds += deltaSeconds;
				break;
			
			case SCR_EAnalyticsDelayedState.OUT_MENU_WAITING:
				m_fDelayLeftSeconds -= deltaSeconds;
				if (m_fDelayLeftSeconds <= 0)
					m_eState = SCR_EAnalyticsDelayedState.OUT_MENU_SENDING;
				break;
			
			case SCR_EAnalyticsDelayedState.OUT_MENU_SENDING:
				break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Should we send our data already?
	bool IsReadyToSend()
	{
		return m_eState == SCR_EAnalyticsDelayedState.OUT_MENU_SENDING;
	}
	
	//------------------------------------------------------------------------------------------------
	//! We have sended our data, reset.
	void ResetReadyToSend()
	{
		m_eState = SCR_EAnalyticsDelayedState.INACTIVE;
	}

	//------------------------------------------------------------------------------------------------
	//! How much time we have spent in menu
	//! \return number of seconds rounded to the nearest whole number
	int GetTimeSpent()
	{
		return Math.Round(m_fTimeSpentSeconds);
	}
}
