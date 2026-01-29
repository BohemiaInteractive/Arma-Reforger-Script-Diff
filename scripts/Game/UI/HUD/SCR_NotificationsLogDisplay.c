/*
Handles the SCR_NotificationsLogComponent when Player is initialized.
*/
class SCR_NotificationsLogDisplay : SCR_InfoDisplayExtended
{
	[Attribute(desc: "Should the Notifications go from top to bottom instead of bottom to top?")]
	protected bool m_bInsertFromTop;

	protected SCR_NotificationsLogComponent m_NotificationsHud;
	protected SCR_InfoDisplaySlotHandler m_slotHandler;
	protected SCR_HUDSlotUIComponent m_HUDSlotComponent;
	/*
	Sets the height of 1 notification to the available spac in the slot can be set correctly. 
	!CANNOT BE 0!
	*/
	protected const int HEIGHT_DIVIDER = 50;

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;

		m_NotificationsHud = SCR_NotificationsLogComponent.Cast(m_wRoot.FindHandler(SCR_NotificationsLogComponent));

		m_slotHandler = SCR_InfoDisplaySlotHandler.Cast(GetHandler(SCR_InfoDisplaySlotHandler));
		if (!m_slotHandler)
			return;
		
		m_HUDSlotComponent = m_slotHandler.GetSlotUIComponent();
		if (!m_HUDSlotComponent)
			return;

		m_HUDSlotComponent.GetOnResize().Insert(OnSlotUIResize);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		// Assign it again in case the SlotUIComponent has changed
		m_HUDSlotComponent = m_slotHandler.GetSlotUIComponent();
		if (!m_HUDSlotComponent)
			return;
		
		m_HUDSlotComponent.GetOnResize().Remove(OnSlotUIResize);
	}
	
	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{		
		if (m_HUDSlotComponent != m_slotHandler.GetSlotUIComponent())
		{
			if (m_HUDSlotComponent)
				m_HUDSlotComponent.GetOnResize().Remove(OnSlotUIResize);
			
			m_HUDSlotComponent = m_slotHandler.GetSlotUIComponent();
			if (!m_HUDSlotComponent)
				return;
			
			m_HUDSlotComponent.GetOnResize().Insert(OnSlotUIResize);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Calculate the amount of lines that can be displayed dependent on how much space the Layout has
	void OnSlotUIResize()
	{
		// Assign it again in case the SlotUIComponent has changed
		m_HUDSlotComponent = m_slotHandler.GetSlotUIComponent();
		if (!m_HUDSlotComponent)
			return;
		
		int maxNotifications = (int)m_HUDSlotComponent.GetHeight() / HEIGHT_DIVIDER;
		if (maxNotifications < 1)
			maxNotifications = 1;

		m_NotificationsHud.OnSlotResize(maxNotifications);
		m_NotificationsHud.ChangeInsertOrder(m_bInsertFromTop);
	}
}
