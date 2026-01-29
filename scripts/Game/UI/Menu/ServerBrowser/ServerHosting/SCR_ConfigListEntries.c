[BaseContainerProps(configRoot: true)]
class SCR_ConfigListEntries
{	
	[Attribute()]
	protected ref array<ref SCR_ConfigListEntry> m_aEntryDefinitions;
	
	//------------------------------------------------------------------------------------------------
	SCR_ConfigListEntry FindEntryByTypeTag(string tag)
	{
		foreach (SCR_ConfigListEntry entry : m_aEntryDefinitions)
		{
			if (entry.GetTypeTag() == tag)
				return entry;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ConfigListEntry FindEntryByWidgetListEntry(SCR_WidgetListEntry widgetListEntry)
	{
		foreach (SCR_ConfigListEntry entry : m_aEntryDefinitions)
		{
			if (entry.GetWidgetListEntry() == widgetListEntry)
				return entry;
		}	
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ConfigListEntry FindEntryByComponentType(typename componentType)
	{
		foreach (SCR_ConfigListEntry entry : m_aEntryDefinitions)
		{
			if (entry.GetScriptedWidgetComponentType() == componentType.ToString())
				return entry;
		}
		
		return null;
	}
}

/*!
Define SCR_WidgetListEntry layout, type. 
Based on configuration entries and widgets can be created.
E.g. there is list of widget, one has SCR_EditboxComponent. If it's needed to define it in unified data list it's found that SCR_WidgetListEntryEditBox needs to be created.
*/
[BaseContainerProps(), SCR_ConfigListEntryCustomTitle()]
class SCR_ConfigListEntry
{
	[Attribute(desc: "Name that can closer specify type of entry. E.g. Regular text field and ip text field")]
	protected string m_sTypeTag;
	
	[Attribute(desc: "Template that should be used for the entry type")]
	ref protected SCR_WidgetListEntry m_WidgetListEntry;
	
	[Attribute(desc: "Should be a type of SCR_ScriptedWidgetComponent.")]
	protected string m_sScriptedWidgetComponentType;
	
	//------------------------------------------------------------------------------------------------
	string GetTypeTag()
	{
		return m_sTypeTag;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_WidgetListEntry GetWidgetListEntry()
	{
		return m_WidgetListEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetScriptedWidgetComponentType()
	{
		return m_sScriptedWidgetComponentType;
	}
}

class SCR_ConfigListEntryCustomTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		// Tag
		string tag = "";
		source.Get("m_sGroupTag", tag);
		
		// Entry type 
		SCR_WidgetListEntry entry;
		source.Get("m_WidgetListEntry", entry);
		
		string entryTypeName = "*undefined*";
		if (entry)
			entryTypeName = entry.Type().ToString();
		
		// Widget component
		typename widgetComponentType;
		source.Get("m_ScriptedWidgetComponentType", widgetComponentType);
		string widgetComponentTypeName = "*underfined*";
		if (widgetComponentType)
			widgetComponentTypeName = widgetComponentType.ToString();
		
		// Setup title string 
		title = string.Format("[tag: %1] %2 : %3", tag, entryTypeName, widgetComponentTypeName);
		
		return true;
	}
};