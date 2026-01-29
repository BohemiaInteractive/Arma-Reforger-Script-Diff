class SCR_PlayerHubWarningEditorUIComponent : SCR_BaseEditorUIComponent
{	
	protected static SCR_PlayerHubWarningEditorUIComponent m_Self;
	protected Widget m_WidgetRef;
	
	protected bool m_bTimerSet;
	protected int m_iCurrentWarningTimer;
	protected SCR_PlayerHubAreaTriggerEntity m_LinkedHub;
	
	[Attribute()]
	protected string m_sTimerText;

	protected TextWidget m_TimerText;
	
	protected IEntity m_PlayerRef;
	
	//======================== UPDATE TIMER ========================\\

	//------------------------------------------------------------------------------------------------
	//Todo: this is not accurate
	protected void UpdateTimer()
	{
		m_iCurrentWarningTimer--;
		m_TimerText.SetText(m_iCurrentWarningTimer.ToString());
		
		if (m_iCurrentWarningTimer <= 0)
		{
			TimerDone();
			HideTimer();
		}
	}
	
	//======================== TIMER COMPLETED/CANCELED ========================\\

	//------------------------------------------------------------------------------------------------
	//Use: Replication.Time()
	protected void TimerDone(bool canceled = false)
	{
		//Remove Update
		GetGame().GetCallqueue().Remove(UpdateTimer);
		m_Self.m_bTimerSet = false;
		
		//Timer done
		m_LinkedHub.TimerDone(m_PlayerRef, canceled);
		
		HideTimer();
	}
	
	//======================== SET TIMER ========================\\

	//------------------------------------------------------------------------------------------------
	//! \param[in] newTimer
	//! \param[in] newHub
	//! \param[in] playerInTrigger
	static void SetWarningTimer(int newTimer, SCR_PlayerHubAreaTriggerEntity newHub, IEntity playerInTrigger)
	{		
		if (!m_Self)
			return;

		//Cancel prev timer
		if (m_Self.m_LinkedHub)
			m_Self.m_LinkedHub.TimerDone(m_Self.m_PlayerRef, true);
		
		//Set Hub and player ref
		m_Self.m_LinkedHub = newHub;
		m_Self.m_PlayerRef = playerInTrigger;

		
		//Set timer
		m_Self.m_iCurrentWarningTimer = newTimer;
		m_Self.m_bTimerSet = true;
		
		//Set Update
		GetGame().GetCallqueue().CallLater(m_Self.UpdateTimer, 1000, true);
		
		m_Self.ShowTimer();
	}
	
	//======================== CANCEL TIMER ========================\\

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] hub
	//! \param[in] hubDestroyed
	static void CancelWarningTimer(SCR_PlayerHubAreaTriggerEntity hub, bool hubDestroyed = false)
	{
		if (!m_Self)
			return;
		
		if (m_Self.m_bTimerSet && (!hubDestroyed || ( hubDestroyed && m_Self.m_LinkedHub == hub))){
			m_Self.TimerDone(true);
		}
	}
	
	
	//======================== VISIBILITY ========================\\

	//------------------------------------------------------------------------------------------------
	protected void ShowTimer()
	{
		m_TimerText.SetText(m_iCurrentWarningTimer.ToString());
		m_WidgetRef.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HideTimer()
	{
		m_WidgetRef.SetVisible(false);
	}

	//======================== INHERITED ========================\\

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{	
		m_WidgetRef = w;
		m_Self = this;
		m_TimerText = TextWidget.Cast(w.FindAnyWidget(m_sTimerText));
		
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_bTimerSet && m_Self)
			GetGame().GetCallqueue().Remove(m_Self.UpdateTimer);
	}
}
