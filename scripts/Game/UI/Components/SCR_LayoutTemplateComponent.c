class SCR_LayoutTemplateComponent: ScriptedWidgetComponent
{
	[Attribute("", UIWidgets.Auto, "Template identifier (in case there are multiple)")]
	protected string m_sName;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Template layout", "layout")]
	protected ResourceName m_Layout;
	
	ResourceName GetName()
	{
		return m_sName;
	}
	ResourceName GetLayout()
	{
		if (m_Layout == "") Print("Layout not defined in SCR_LayoutTemplateComponent component!", LogLevel.ERROR);
		return m_Layout;
	}
	
	static ResourceName GetLayout(Widget w, string name = "")
	{		
		if (!w)
		{
			Print("Cannot find SCR_LayoutTemplateComponent, widget is null!", LogLevel.WARNING);
			return ResourceName.Empty;
		}
		int componentsCount = w.GetNumHandlers();
		for (int i = 0; i < componentsCount; i++)
		{
			SCR_LayoutTemplateComponent component = SCR_LayoutTemplateComponent.Cast(w.GetHandler(i));
			if (component && component.GetName() == name) return component.GetLayout();
		}
		
		Print(string.Format("Cannot find SCR_LayoutTemplateComponent component in widget '%1'!", w.GetName()), LogLevel.ERROR);
		return ResourceName.Empty;
	}
};