class SCR_TutorialFastTravelSpinBox : SCR_SpinBoxComponent
{
	protected ref ScriptInvokerBool m_OnFocus;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBool GetOnFocus()
	{
		if (!m_OnFocus)
			m_OnFocus = new ScriptInvokerBool;
		
		return m_OnFocus;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		
		m_OnFocus.Invoke(true);
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);

		m_OnFocus.Invoke(false);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool SetCurrentItem(int i, bool playSound = false, bool animate = false)
	{
		super.SetCurrentItem(i, playSound, animate);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetItemIndex(Managed item)
	{
		if (!m_aElementData)
			return -1;
		
		return m_aElementData.Find(item);
	}
}