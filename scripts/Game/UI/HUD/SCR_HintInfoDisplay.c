//! Handles SCR_HintUIComponent when using InfoDisplay and HudManager slotting
class SCR_HintInfoDisplay : SCR_InfoDisplayExtended
{
	[Attribute(desc: "Name of the widget to which the SCR_HintUIComponent is attached")]
	protected string m_sHintComponentWidget;
	
	protected SCR_HintUIComponent m_HintComponent;
	protected SCR_InfoDisplaySlotHandler m_SlotHandler;
	protected SCR_HUDSlotUIComponent m_HUDSlotComponent;

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;

		Widget hintWidget = m_wRoot.FindAnyWidget(m_sHintComponentWidget);
		
		if (!hintWidget)
			return;
		
		m_HintComponent = SCR_HintUIComponent.Cast(hintWidget.FindHandler(SCR_HintUIComponent));
		if (!m_HintComponent)
			return;

		m_SlotHandler = SCR_InfoDisplaySlotHandler.Cast(GetHandler(SCR_InfoDisplaySlotHandler));
		if (!m_SlotHandler)
			return;
		
		//! Change visibility to false until Hint is Shown.
		Show(false);
		
		m_HUDSlotComponent = m_SlotHandler.GetSlotUIComponent();
		if (!m_HUDSlotComponent)
			return;

		m_HintComponent.GetOnHintShown().Insert(OnHintShown);
		m_HUDSlotComponent.GetOnResize().Insert(OnSlotUIResize);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		if (m_HUDSlotComponent)
			m_HUDSlotComponent.GetOnResize().Remove(OnSlotUIResize);

		if (m_HintComponent)
			m_HintComponent.GetOnHintShown().Remove(OnHintShown);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{		
		//Check if the SlotUIComponent is still valid otherwise change to the new one
		if (m_HUDSlotComponent != m_SlotHandler.GetSlotUIComponent())
		{
			if (m_HUDSlotComponent)
				m_HUDSlotComponent.GetOnResize().Remove(OnSlotUIResize);
			
			m_HUDSlotComponent = m_SlotHandler.GetSlotUIComponent();
			if (!m_HUDSlotComponent)
				return;
			
			m_HUDSlotComponent.GetOnResize().Insert(OnSlotUIResize);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Make it visible / invisible to trigger rezising of the Group
	protected void OnHintShown(bool isShown)
	{
		Show(isShown);
	}

	//------------------------------------------------------------------------------------------------
	//! Make InfoDisplay in-/visible when menu is changed
	protected void OnSlotUIResize()
	{
		Show(m_HintComponent && m_HintComponent.IsHintShown());
	}
}
