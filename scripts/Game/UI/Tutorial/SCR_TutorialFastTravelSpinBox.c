class SCR_TutorialFastTravelSpinBox : SCR_SpinBoxComponent
{
	ref ScriptInvoker m_OnItemSet = new ScriptInvoker;
	//------------------------------------------------------------------------------------------------
	override bool SetCurrentItem(int i, bool playSound = false, bool animate = false)
	{
		super.SetCurrentItem(i, playSound, animate);
		
		m_OnItemSet.Invoke();
		
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