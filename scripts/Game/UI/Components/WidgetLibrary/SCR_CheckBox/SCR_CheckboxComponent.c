//! This is refactored checkbox preserving its own class and API for compatability purposes.

class SCR_CheckboxComponent : SCR_ToolboxComponent
{
	//------------------------------------------------------------------------------------------------
	bool IsChecked() 
	{
		return m_iSelectedItem;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetChecked(bool value, bool animate = true, bool playSound = true)
	{
		int index = value;
		SetCurrentItem(index, animate, playSound);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnElementChanged(SCR_ButtonBaseComponent comp, bool state)
	{
		if (!state)
			return;
		
		bool selected = m_aSelectionElements.Find(comp);
		m_OnChanged.Invoke(this, selected);
	}
	
	static SCR_CheckboxComponent GetCheckboxComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_CheckboxComponent.Cast(
			SCR_WLibComponentBase.GetComponent(SCR_CheckboxComponent, name, parent, searchAllChildren)
		);
		return comp;
	}
};