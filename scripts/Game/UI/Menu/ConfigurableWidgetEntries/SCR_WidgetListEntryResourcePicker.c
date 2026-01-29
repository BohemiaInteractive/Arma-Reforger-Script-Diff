[BaseContainerProps(configRoot: true), SCR_WidgetListEntryCustomTitle()]
class SCR_WidgetListEntryResourcePicker : SCR_WidgetListEntry
{
	protected SCR_ImagePickerComponent m_ResourcePicker;
	
	//-------------------------------------------------------------------------------------------
	override protected void SetupHandlers()
	{
		m_ResourcePicker = SCR_ImagePickerComponent.Cast(m_EntryRoot.FindHandler(SCR_ImagePickerComponent));
	}
	
	//-------------------------------------------------------------------------------------------
	override void SetValue(string str)
	{
		m_ResourcePicker.SetImage(str);
	}
	
	//-------------------------------------------------------------------------------------------
	override string ValueAsString()
	{
		return m_ResourcePicker.GetResourcePath();
	}
	
	//-------------------------------------------------------------------------------------------
	SCR_ImagePickerComponent GetResourcePicker()
	{
		return m_ResourcePicker;
	}
}