[BaseContainerProps(configRoot: true)]
class SCR_WidgetListEntryCheckList : SCR_WidgetListEntry
{
	[Attribute()]
	protected ResourceName m_sElementPath;
	
	protected ref array<ref SCR_LocalizedProperty> m_aProperties = {};
	protected ref array<ref SCR_ListBoxElementComponent> m_aCheckboxes = {};
	
	//-------------------------------------------------------------------------------------------
	//! Removed setting up of m_ChangeableComponent as it's not used for this case 
	override void CreateWidget(Widget parent)
	{
		// Create widget 
		m_EntryRoot = GetGame().GetWorkspace().CreateWidgets(m_sEntryLayout, parent);
	
		// Show 
		m_EntryRoot.SetVisible(m_bShow);
		if (!m_bShow)
			return;
		
		SetupHandlers();
		SetInteractive(m_bInteractive);
	}
	
	//-------------------------------------------------------------------------------------------
	void ToggleCheckbox(notnull SCR_ListBoxElementComponent checkBox, bool enabled)
	{
		if (!m_aCheckboxes.Contains(checkBox))
		{
			Print("WidgetListEntryCheckList Checkbox couldn't be found", LogLevel.WARNING);
			return;
		}	
		
		checkBox.SetToggled(enabled);
	}
	
	//-------------------------------------------------------------------------------------------
	//! Add new element and create it in the list
	SCR_ListBoxElementComponent AddElement(SCR_LocalizedProperty property, bool enabled)
	{
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sElementPath, m_EntryRoot);
		SCR_ListBoxElementComponent button = SCR_ListBoxElementComponent.Cast(w.FindHandler(SCR_ListBoxElementComponent));
		if (!button)
			return null;
		
		button.SetText(property.m_sLabel);
		button.SetToggled(enabled);
		
		button.m_OnClicked.Insert(OnCheckboxClick);
		
		m_aCheckboxes.Insert(button);
		m_aProperties.Insert(property);
		
		return button;
	}
	
	//-------------------------------------------------------------------------------------------
	array<string> EnabledCheckListPropertyNames()
	{
		array<string> enabled = {};
		foreach (int i, SCR_ModularButtonComponent checkbox : m_aCheckboxes)
		{
			if (checkbox.GetToggled())
				enabled.Insert(m_aProperties[i].m_sPropertyName);
		}
		
		return enabled;
	}
	
	//-------------------------------------------------------------------------------------------
	int CheckboxesCount()
	{
		return m_aCheckboxes.Count();
	}
	
	//-------------------------------------------------------------------------------------------
	SCR_ListBoxElementComponent FindCheckboxByPropertyName(string propertyName)
	{
		foreach (int i, SCR_LocalizedProperty property : m_aProperties)
		{
			if (property.m_sPropertyName == propertyName)
				return m_aCheckboxes[i];
		}
		
		return null;
	}
	
	//-------------------------------------------------------------------------------------------
	//! Handle toggling in script as checkbox can't be interacted without script
	protected void OnCheckboxClick(SCR_ModularButtonComponent button)
	{
		bool toggled = button.GetToggled();
		button.SetToggled(!toggled);
	}
}