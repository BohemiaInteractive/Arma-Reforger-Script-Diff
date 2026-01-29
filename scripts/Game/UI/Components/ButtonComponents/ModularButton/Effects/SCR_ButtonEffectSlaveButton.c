//! Effect which passes all subscribed events to another modular button.
[BaseContainerProps(configRoot : true), SCR_ButtonEffectTitleAttribute("SlaveButton", "m_sWidgetName")]
class SCR_ButtonEffectSlaveButton : SCR_ButtonEffectWidgetBase
{
	protected SCR_ModularButtonComponent m_targetComponent;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] targetWidget
	void SetTargetWidget(Widget targetWidget)
	{
		m_wTarget = targetWidget;
		SCR_ModularButtonComponent targetComp = SCR_ModularButtonComponent.Cast(m_wTarget.FindHandler(SCR_ModularButtonComponent));
		m_targetComponent = targetComp;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnHandlerAttached(Widget w)
	{
		super.OnHandlerAttached(w);
		
		if (m_wTarget)
			m_targetComponent = SCR_ModularButtonComponent.Cast(m_wTarget.FindHandler(SCR_ModularButtonComponent));
	}

	//------------------------------------------------------------------------------------------------
	override void OnStateDefault(bool instant)
	{
		if (!m_targetComponent)
			return;
		
		m_targetComponent.Internal_OnMasterButtonEvent(EModularButtonEventFlag.STATE_DEFAULT, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStateHovered(bool instant)
	{
		if (!m_targetComponent)
			return;
		
		m_targetComponent.Internal_OnMasterButtonEvent(EModularButtonEventFlag.STATE_HOVERED, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStateActivated(bool instant)
	{
		if (!m_targetComponent)
			return;
		
		m_targetComponent.Internal_OnMasterButtonEvent(EModularButtonEventFlag.STATE_ACTIVATED, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStateActivatedHovered(bool instant)
	{
		if (!m_targetComponent)
			return;
		
		m_targetComponent.Internal_OnMasterButtonEvent(EModularButtonEventFlag.STATE_ACTIVATED_HOVERED, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnStateDisabled(bool instant)
	{
		if (!m_targetComponent)
			return;
		
		m_targetComponent.Internal_OnMasterButtonEvent(EModularButtonEventFlag.STATE_DISABLED, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateDisabledActivated(bool instant)
	{
		if (!m_targetComponent)
			return;
		
		m_targetComponent.Internal_OnMasterButtonEvent(EModularButtonEventFlag.STATE_DISABLED_ACTIVATED, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnClicked(bool instant)
	{
		if (!m_targetComponent)
			return;
		
		m_targetComponent.Internal_OnMasterButtonEvent(EModularButtonEventFlag.EVENT_CLICKED, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnFocusGained(bool instant)
	{
		if (!m_targetComponent)
			return;
		
		m_targetComponent.Internal_OnMasterButtonEvent(EModularButtonEventFlag.EVENT_FOCUS_GAINED, instant);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFocusLost(bool instant)
	{
		if (!m_targetComponent)
			return;
		
		m_targetComponent.Internal_OnMasterButtonEvent(EModularButtonEventFlag.EVENT_FOCUS_LOST, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnToggledOn(bool instant)
	{
		if (!m_targetComponent)
			return;
		
		m_targetComponent.Internal_OnMasterButtonEvent(EModularButtonEventFlag.EVENT_TOGGLED_ON, instant);
	}

	//------------------------------------------------------------------------------------------------
	override void OnToggledOff(bool instant)
	{
		if (!m_targetComponent)
			return;
		
		m_targetComponent.Internal_OnMasterButtonEvent(EModularButtonEventFlag.EVENT_TOGGLED_OFF, instant);
	}
}
