
class SCR_DropdownWithParamAttributeUIComponent: SCR_DropdownEditorAttributeUIComponent
{	
	//~ Param used in array
	protected LocalizedString m_sParam;
	
	protected override void CreateDropdownEntries()
	{
		if (m_aDropdownArray.IsEmpty())
			return;
		
		m_sParam = m_aDropdownArray[0];
		
		m_aDropdownArray.RemoveOrdered(0);
		
		super.CreateDropdownEntries();
	}
	
	protected override string GetFullDropdownEntryText(string text)
	{
		return WidgetManager.Translate(text, m_sParam);
	}
};