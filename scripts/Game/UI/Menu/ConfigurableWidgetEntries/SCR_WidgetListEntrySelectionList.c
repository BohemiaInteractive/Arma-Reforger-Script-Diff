/*!
Can hold multiple entries, setup and read them
*/
[BaseContainerProps(configRoot: true), SCR_WidgetListEntryCustomTitle()]
class SCR_WidgetListEntrySelectionList : SCR_WidgetListEntry
{
	[Attribute()]
	protected ref SCR_WidgetListEntry m_Element;
	
	[Attribute()]
	protected ResourceName m_sAddingElementPath;
	
	[Attribute()]
	protected string m_ElementType;
	
	[Attribute()]
	protected ref array<ref SCR_WidgetListEntry> m_aEntries;
	
	protected ref SCR_ModularButtonComponent m_AddingButton;
	
	protected ref ScriptInvokerVoid m_OnClickAddingButton;
	protected ref ScriptInvokerWidget m_OnAddElement;
	
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
	override protected void SetupHandlers()
	{
		if (!m_sAddingElementPath.IsEmpty())
		{
			Widget addingButton = GetGame().GetWorkspace().CreateWidgets(m_sAddingElementPath, m_EntryRoot);
			m_AddingButton = SCR_ModularButtonComponent.Cast(addingButton.FindHandler(SCR_ModularButtonComponent));
			
			if (m_AddingButton)
				m_AddingButton.m_OnClicked.Insert(OnClickAddingButton);
		}
	}
	
	//-------------------------------------------------------------------------------------------
	//! Search for elements with given names
	protected void FindElements(array<string> elementNames)
	{
		SCR_ConfigListEntries presets = GetEntriesDefinitions(ENTRY_DEFINITIONS);
		
		ScriptedWidgetEventHandler component;
		SCR_ConfigListEntry configEntry;
		SCR_WidgetListEntry entry;
		
		foreach (string name : elementNames)
		{ 
			Widget w = m_EntryRoot.FindAnyWidget(name);
			if (!w)
				continue;
			
			typename type = m_ElementType.ToType();
			
			component = w.FindHandler(type);
			if (!component)
				continue;
			
			configEntry = presets.FindEntryByComponentType(type);
			if (!configEntry)
				continue;
			
			entry = configEntry.GetWidgetListEntry();
			if (!entry)
				continue;
			
			m_aEntries.Insert(entry);
		}
	}
	
	//-------------------------------------------------------------------------------------------
	void AddElement(notnull SCR_WidgetListEntry entry)
	{
		array<string> values = GetElementValues();
		
		if (m_OnAddElement)
			m_OnAddElement.Invoke(entry.m_EntryRoot);
	}
	
	//-------------------------------------------------------------------------------------------
	void RemoveElementByName(string propertyName)
	{
		SCR_WidgetListEntry entryToRemove;
		
		foreach (SCR_WidgetListEntry entry : m_aEntries)
		{
			if (entry.GetPropertyName() != propertyName)
				continue;
			
			entryToRemove = entry;
			break;
		}
		
		if (!entryToRemove)
			return;
		
		Widget entryWidget = entryToRemove.GetEntryRoot();
		Widget widgetParent = entryWidget.GetParent();
		widgetParent.RemoveChild(entryWidget);
		
		m_aEntries.RemoveItem(entryToRemove);
	}
	
	//-------------------------------------------------------------------------------------------
	void RemoveElementByWidget(Widget widget)
	{
		SCR_WidgetListEntry entryToRemove;
		
		foreach (SCR_WidgetListEntry entry : m_aEntries)
		{
			if (entry.GetEntryRoot() != widget)
				continue;
			
			entryToRemove = entry;
			break;
		}
		
		if (!entryToRemove)
			return;
		
		Widget entryWidget = entryToRemove.GetEntryRoot();
		Widget widgetParent = entryWidget.GetParent();
		widgetParent.RemoveChild(entryWidget);
		
		m_aEntries.RemoveItem(entryToRemove);
	}
	
	//-------------------------------------------------------------------------------------------
	void GetEntries(out array<SCR_WidgetListEntry> entries)
	{
		entries = {};
		foreach (SCR_WidgetListEntry entry : m_aEntries)
		{
			entries.Insert(entry);
		}
	}
	
	//-------------------------------------------------------------------------------------------
	array<string> GetElementValues()
	{
		array<string> values = {};
		
		foreach (SCR_WidgetListEntry entry : m_aEntries)
		{
			string val = entry.ValueAsString();
			values.Insert(val);
		}
		
		return values;
	}
	
	//-------------------------------------------------------------------------------------------
	protected void OnClickAddingButton()
	{
		AddElement(m_Element);
		
		if (m_OnClickAddingButton)
			m_OnClickAddingButton.Invoke();
	}
	
	//-------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnAddingButtonElement()
	{
		if (!m_OnClickAddingButton)
			m_OnClickAddingButton = new ScriptInvokerVoid();
		
		return m_OnClickAddingButton;
	}
	
	//-------------------------------------------------------------------------------------------
	ScriptInvokerWidget GetOnAddElement()
	{
		if (!m_OnAddElement)
			m_OnAddElement = new ScriptInvokerWidget();
		
		return m_OnAddElement;
	}
	
	//-------------------------------------------------------------------------------------------
	SCR_WidgetListEntry GetElement()
	{
		return m_Element;
	}
}