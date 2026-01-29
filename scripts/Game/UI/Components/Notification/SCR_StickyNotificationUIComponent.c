class SCR_StickyNotificationUIComponent : ScriptedWidgetComponent
{
	[Attribute("StickyNotificationMessage")]
	protected string m_sTextName;
	
	[Attribute("OptionalMessageLayout")]
	protected string m_sOptionalMessageLayoutName;
	
	//Reference
	protected SCR_NotificationsLogComponent m_NotificationsLog;
	protected Widget m_Root;
	protected SCR_FadeUIComponent m_FadeUIComponent;
	
	[Attribute("1", desc: "Wether or not the notification message list shrinks if a new stick notification is added (and increases again on deactivate). Do not change in runtime unless you are certain the notification log knows this notification is not active!")]
	protected bool m_bInfluenceNotificationListSize;
	
	protected bool m_bStickyNotificationActive = false;
	
	protected TextWidget m_Text;
	protected Widget m_OptionalMessageLayout;
	
	protected ref ScriptInvoker Event_OnStickyActiveChanged = new ScriptInvoker();
	
	protected void SetStickyActive(bool newActive, bool fade = true)
	{
		if (newActive == m_bStickyNotificationActive)
		{
			//Still set visiblity
			if (!fade)
				SetVisible(newActive);
			
			return;
		}
			
		m_bStickyNotificationActive = newActive;
		
		Event_OnStickyActiveChanged.Invoke(m_bStickyNotificationActive);
		
		if (fade)
			Fade(newActive);
		else
			SetVisible(newActive);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get if this sticky notification is active
	//! \return is active or not
	//!
	bool isStickyActive()
	{
		return m_bStickyNotificationActive;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get if sticky notification influences list size. If true then the Notification log message history will decrease if a sticky notification is active
	//! \return if influences or not
	//!
	bool InfluenceNotificationListSize()
	{
		return m_bInfluenceNotificationListSize;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns scriptinvoker on sticky notification active changed
	//! \return Event_OnStickyActiveChanged on notification active changed
	//!
	ScriptInvoker GetOnStickyActiveChanged()
	{
		return Event_OnStickyActiveChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetVisible(bool visible)
	{
		if (visible)
			m_Root.SetOpacity(1);
		else 
			m_Root.SetOpacity(0);
		
		m_Root.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Fade(bool fadeIn)
	{				
		if (fadeIn)
		{
			if (!m_FadeUIComponent.IsFadingIn())
				m_FadeUIComponent.FadeIn(false);
		}
		else 
		{
			if (!m_Root.IsVisible() || !m_FadeUIComponent.IsFadingOut())
				m_FadeUIComponent.FadeOut(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnButton()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] notificationLog
	void OnInit(SCR_NotificationsLogComponent notificationLog)
	{		
		if (SCR_Global.IsEditMode() || !m_Root) 
			return;
		
		m_NotificationsLog = notificationLog;
		
		//Disable clicking on the UI
		if (!m_NotificationsLog.HasNotificationInput())
		{
			m_Root.SetEnabled(false);
			return;
		}
		
		SCR_EditorManagerEntity editorManager = notificationLog.GetEditorManager();
		if (!editorManager)
			return;
		
		m_Root.SetEnabled(editorManager.IsOpened());
		editorManager.GetOnOpened().Insert(EditorOpened);
		editorManager.GetOnClosed().Insert(EditorClosed);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDestroy()
	{
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EditorOpened()
	{
		if (m_Root)
			m_Root.SetEnabled(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EditorClosed()
	{
		if (m_Root)
			m_Root.SetEnabled(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode()) 
			return;
		
		m_Root = w;
		m_FadeUIComponent = SCR_FadeUIComponent.Cast(w.FindHandler(SCR_FadeUIComponent));
		if (!m_FadeUIComponent)
			Print("SCR_NotificationVotingInProgressUIComponent could not find SCR_FadeUIComponent! Make sure this component comes after the SCR_FadeUIComponent!", LogLevel.ERROR);
		
		m_Text = TextWidget.Cast(w.FindAnyWidget(m_sTextName));
		if (!m_Text)
			Print("SCR_NotificationVotingInProgressUIComponent could not find m_Text!", LogLevel.ERROR);
		
		m_OptionalMessageLayout = w.FindAnyWidget(m_sOptionalMessageLayoutName);
		
		ScriptInvoker onPressed = ButtonActionComponent.GetOnAction(w);
		if (onPressed) 
			onPressed.Insert(OnButton);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		OnDestroy();
		
		if (m_NotificationsLog)
		{
			SCR_EditorManagerEntity editorManager = m_NotificationsLog.GetEditorManager();
			if (editorManager)
			{
				editorManager.GetOnOpened().Remove(EditorOpened);
				editorManager.GetOnClosed().Remove(EditorClosed);
			}
		}
	}
}
