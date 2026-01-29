class SCR_ButtonHintEditorUIComponent : MenuRootSubComponent
{
	//---- REFACTOR NOTE START: Conditions might be using more generic behavior? 
	
	protected const int CONDITION_DEVICE = 1;
	protected const int CONDITION_FOCUS = 2;
	
	protected int m_Show;
	
	//------------------------------------------------------------------------------------------------
	protected void RefreshVisibility()
	{
		bool show = m_Show == (CONDITION_DEVICE | CONDITION_FOCUS);
		
		GetWidget().SetEnabled(show);
		GetWidget().SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusGained()
	{
		m_Show |= CONDITION_FOCUS;
		
		RefreshVisibility();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusLost()
	{
		m_Show &= ~CONDITION_FOCUS;
		
		RefreshVisibility();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		if (isGamepad)
			m_Show |= CONDITION_DEVICE;
		else
			m_Show &= ~CONDITION_DEVICE;
		
		RefreshVisibility();
	}
	
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	override protected bool IsUnique()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		MenuRootBase menu = GetMenu();
		if (!menu)
			return;
		
		menu.GetOnMenuFocusGained().Insert(OnMenuFocusGained);
		menu.GetOnMenuFocusLost().Insert(OnMenuFocusLost);
		
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		OnMenuFocusGained();
		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		MenuRootBase menu = GetMenu();
		if (!menu)
			return;
		
		menu.GetOnMenuFocusGained().Remove(OnMenuFocusGained);
		menu.GetOnMenuFocusLost().Remove(OnMenuFocusLost);
		
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}
}
