/*!
Scripted DS config to link all server config part together
Override for all DS config part to scripted behvaior
*/

//------------------------------------------------------------------------------------------------
//! Handler for json api struct values 
class SCR_JsonApiStructHandler
{
	//------------------------------------------------------------------------------------------------
	//! Store value has to be called in OnPack only
	static void StoreValue(JsonApiStruct json, string variableName, string value, EJsonApiStructValueType type)
	{
		// Check json api struct
		if (!json || variableName.IsEmpty() || value.IsEmpty())
			return; 
		
		Print(string.Format("store: %1 = %2 (%3)", variableName, value, type));
		
		// Store valu based on type
		switch(type)
		{
			// Booleans
			case EJsonApiStructValueType.TYPE_BOOL:
			{
				bool bValue = ToBool(value);
				json.StoreBoolean(variableName, bValue);
				break;
			}
			
			// Integers 
			case EJsonApiStructValueType.TYPE_INT: 
			{
				int iValue = value.ToInt();
				json.StoreInteger(variableName, iValue);
				break;
			}
			
			// Floating decimals 
			case EJsonApiStructValueType.TYPE_FLOAT:
			{
				int fValue = value.ToFloat();
				json.StoreFloat(variableName, fValue);
				break;
			}
			
			// Strings
			case EJsonApiStructValueType.TYPE_STRING:
			{
				json.StoreString(variableName, value);
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected static bool ToBool(string value)
	{
		return value == "1" || value == "true";
	}
	
	//------------------------------------------------------------------------------------------------
	static bool StringToBool(string str)
	{
		switch (str)
		{
			case "true": return true;
			case "false": return false;
			case "1": return true;
			case "0": return false;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	static string BoolToString(bool boolean)
	{
		if (boolean)
			return "1";
		
		return "0";
	}
};

//------------------------------------------------------------------------------------------------
enum EJsonApiStructValueType
{
	TYPE_BOOL,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRING,
};

//------------------------------------------------------------------------------------------------
//! Script class aggregating all dedicated server config part together
class SCR_CombinedDSConfig
{
	// Consts 
	protected const string DEFAULT_CONFIG_NAME = "Server setup";
	
	// Attributes 
	[Attribute("config", UIWidgets.EditBox, "Group tag name for base server configuration")]
	protected string m_sCategoryConfig;
	
	[Attribute("", UIWidgets.EditBox, "Group tag name for generic game settings")]
	protected string m_sCategoryGameConfig;
	
	[Attribute("", UIWidgets.EditBox, "Group tag name for specific game (scenario) properties")]
	protected string m_sCategoryGameProperties;
	
	// File properties 
	protected string m_sConfigName;
	
	// Config properties containers 
	protected ref SCR_DSConfig m_DSConfig = new SCR_DSConfig(); 							// Config base
	protected ref DSGameConfig m_DSGameConfig = new DSGameConfig();					// Generic game settings
	protected ref DSGameProperties m_DSGameProperties = new DSGameProperties(); 	// Scenario 
	protected ref array<DSMod> m_aMods = {};
	
	// Unified entries
	protected ref array<ref Tuple2<string, ref SCR_WidgetListEntry>> m_aDSConfigEntries = {}; // string - tag
	
	//------------------------------------------------------------------------------------------------
	//! Pass entries and define entries map
	void DefineEntriesMap(array<ref SCR_WidgetListEntry> entries)
	{
		for (int i = 0, count = entries.Count(); i < count; i++)
		{
			m_aDSConfigEntries.Insert(
				new Tuple2<string, ref SCR_WidgetListEntry>(
					entries[i].GetGroupTag(),
					entries[i]
				)
			);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Store value API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Store all values for selected category
	void StoreCategoryToJson(JsonApiStruct json, string tag)
	{
		for (int i = 0, count = m_aDSConfigEntries.Count(); i < count; i++)
		{
			// Skip different tag
			if (m_aDSConfigEntries[i].param1 != tag)
				continue;
			
			SCR_WidgetListEntry entry = m_aDSConfigEntries[i].param2;
			
			// Store value
			SCR_JsonApiStructHandler.StoreValue(
				json,
				entry.GetPropertyName(),
				entry.ValueAsString(),
				entry.GetType()
			);
			
			/*
			Print(string.Format(
				"Save - %1 > %2 : %3", 
				m_aDSConfigEntries[i].param1,
				entry.GetPropertyName(),
				entry.ValueAsString()
			));
			*/
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Store values for all categories
	void StoreFullJson()
	{		
		//Print("-------------------------------------");
		
		for (int i = 0, count = m_aDSConfigEntries.Count(); i < count; i++)
		{
			SCR_WidgetListEntry entry = m_aDSConfigEntries[i].param2;
			if (!entry)
				continue;
			
			// Debug
			//Print(entry.GetPropertyName() + " : " + entry.ValueAsString());
		}
		
		//Print("-------------------------------------");
	}
	
	//------------------------------------------------------------------------------------------------
	// Protected 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Return default config name with number that should be unique to other other configs 
	protected string DefaultConfigName()
	{
		string name = DEFAULT_CONFIG_NAME;
		
		// Todo: counting and comparing of config names 
		
		return name;
	}
};