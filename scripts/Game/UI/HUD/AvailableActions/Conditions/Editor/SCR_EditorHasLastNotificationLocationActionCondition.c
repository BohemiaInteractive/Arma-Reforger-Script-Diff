//! Returns true if has last notification location and time was less then given var
[BaseContainerProps(), BaseContainerCustomStringTitleField("Has notification location")]
class SCR_EditorHasLastNotificationLocationActionCondition: SCR_AvailableActionCondition
{	
	[Attribute("5", desc: "How long will the hint be active if it was valid in seconds")]
	protected int m_DisplayTime;
	
	protected SCR_NotificationsComponent m_NotificationsComponent;
	protected vector m_vLocation = vector.Zero;
	protected bool m_bShowLocation;
	protected bool m_bIsListeningToDisable;
	
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		if (!m_NotificationsComponent)
		{
			m_NotificationsComponent = SCR_NotificationsComponent.GetInstance();
			if (!m_NotificationsComponent)
				return false;
		}
		
		vector location;
		m_NotificationsComponent.GetLastNotificationLocation(location);
		if (location != m_vLocation)
		{
			m_vLocation = location;
			m_bShowLocation = true;
			
			if (m_DisplayTime > 0)
			{
				if (m_bIsListeningToDisable)
					GetGame().GetCallqueue().Remove(SetLocationDisabled);
				
				m_bIsListeningToDisable = true;
				GetGame().GetCallqueue().CallLater(SetLocationDisabled, m_DisplayTime * 1000);
			}
		}
		
		return GetReturnResult(m_vLocation != vector.Zero && m_bShowLocation);
	}
	
	protected void SetLocationDisabled()
	{
		m_bShowLocation = false;
		m_bIsListeningToDisable = false;
	}
};