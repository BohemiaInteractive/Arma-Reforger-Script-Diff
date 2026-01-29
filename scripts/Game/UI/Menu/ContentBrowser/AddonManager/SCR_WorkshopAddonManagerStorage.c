//! Meta data of one addon
[BaseContainerProps()]
class SCR_WorkshopAddonPresetAddonMeta
{
	//----------------------------------------------------------
	[Attribute()]
	protected string m_sGuid;
	
	[Attribute()]
	protected string m_sName;
	
	
	string GetName()
	{
		return m_sName;
	}
	
	string GetGuid()
	{
		return m_sGuid;
	}
	
	
	//---------------------------------------------------------------------------------------------------
	SCR_WorkshopAddonPresetAddonMeta Copy()
	{
		SCR_WorkshopAddonPresetAddonMeta a = (new SCR_WorkshopAddonPresetAddonMeta).Init(m_sGuid, m_sName);
		return a;
	}
	
	
	//---------------------------------------------------------------------------------------------------
	SCR_WorkshopAddonPresetAddonMeta Init(string guid, string name)
	{
		m_sGuid = guid;
		m_sName = name;
		
		return this;
	}
}



//! Preset of addons
[BaseContainerProps()]
class SCR_WorkshopAddonPreset
{
	[Attribute()]
	protected string m_sName;							// Preset name
	
	[Attribute()]
	protected ref array<ref SCR_WorkshopAddonPresetAddonMeta> m_aAddonsMeta;	// Array with addon GUIDs
	
	
	//---------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_sName;
	}
	
	//---------------------------------------------------------------------------------------------------
	void SetName(string name)
	{
		m_sName = name;
	}
	
	
	//---------------------------------------------------------------------------------------------------
	// We must keep constructor without arguments, otherwise JSON serializer has problems
	SCR_WorkshopAddonPreset Init(string name, notnull array<ref SCR_WorkshopAddonPresetAddonMeta> addons)
	{
		m_sName = name;
		m_aAddonsMeta = {};
		
		foreach (auto meta : addons)
			m_aAddonsMeta.Insert(meta.Copy());
		
		return this;
	}
	
	
	//---------------------------------------------------------------------------------------------------
	SCR_WorkshopAddonPreset Copy()
	{
		SCR_WorkshopAddonPreset a = (new SCR_WorkshopAddonPreset).Init(m_sName, m_aAddonsMeta);
		return a;
	}
	
		
	//---------------------------------------------------------------------------------------------------
	int GetAddonCount()
	{
		return m_aAddonsMeta.Count();
	}
	
	
	//---------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopAddonPresetAddonMeta> GetAddons()
	{
		array<ref SCR_WorkshopAddonPresetAddonMeta> outArray = {};
		foreach (auto m : m_aAddonsMeta)
			outArray.Insert(m);
		
		return outArray;
	}
}


// Class which manages storage of multiple addon presets
class SCR_WorkshopAddonManagerPresetStorage : ModuleGameSettings
{	
	//----------------------------------------------------------
	// Serialized variables
	
	[Attribute()]
	protected int m_iVersion; // Might be useful for future versioning
	
	[Attribute()]
	protected ref array<ref SCR_WorkshopAddonPreset> m_aPresets;
	
	[Attribute()]
	protected string m_sUsedPreset; 
	
	const int VERSION = 1;
	const string FILE_NAME = "$profile:.WorkshopPresets.json";
	
	protected ref ScriptInvoker<string, string> Event_OnUsedPresetChanged;

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnUsedPresetChanged(string arg0)
	{
		if (Event_OnUsedPresetChanged)
			Event_OnUsedPresetChanged.Invoke(arg0);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnUsedPresetChanged()
	{
		if (!Event_OnUsedPresetChanged)
			Event_OnUsedPresetChanged = new ScriptInvoker();

		return Event_OnUsedPresetChanged;
	}
	
	//---------------------------------------------------------------------------------------------------
	void SCR_WorkshopAddonManagerPresetStorage()
	{
		// When constructed, read all data from storage into RAM.
		// When presets will be modified, or created, we will override whole SCR_WorkshopAddonManagerPresetStorage object in storage.
		ReadPresetsFromStorage();
	}
	
	
	//-----------------------------------------------------------------------------------------
	// PUBLIC
	
	
	//---------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopAddonPreset> GetAllPresets()
	{
		array<ref SCR_WorkshopAddonPreset> arrayOut = {};

		foreach (SCR_WorkshopAddonPreset preset : m_aPresets)
		{
			arrayOut.Insert(preset);
		}

		return arrayOut;
	}
	
	
	//---------------------------------------------------------------------------------------------------
	void SavePreset(notnull SCR_WorkshopAddonPreset preset, string prevName = "")
	{
		string name = preset.GetName();
		
		int id = GetPresetId(name);
		
		preset.SetName(name.Trim());
		
		// Make a copy of provided data,
		// either append it to array or replace prev. preset with same name
		
		SCR_WorkshopAddonPreset presetCopy = preset.Copy();
		
		if (id != -1)
			m_aPresets[id] = presetCopy;
		else
			m_aPresets.Insert(presetCopy);

		if (CanChangePresetName(id, name, prevName))		
			SetUsedPreset(name);
		
		// Save everything to storage
		SavePresetsToStorage();
	}
	
	//---------------------------------------------------------------------------------------------------
	protected bool CanChangePresetName(int id, string name, string prevName)
	{
		// Brand new created 
		if (id == -1)
			return true;
		
		// Created new overriding existing name
		if (name == GetUsedPreset() || prevName == "")
			return true;
		
		return false;
	}
	
	//---------------------------------------------------------------------------------------------------
	void SetUsedPreset(string name)
	{
		m_sUsedPreset = name;
		SavePresetsToStorage();
		
		InvokeEventOnUsedPresetChanged(name);
	}
	
	//---------------------------------------------------------------------------------------------------
	void ClearUsedPreset()
	{
		SetUsedPreset("");
	}
	
	//---------------------------------------------------------------------------------------------------
	string GetUsedPreset()
	{
		return m_sUsedPreset;
	}
	
	//---------------------------------------------------------------------------------------------------
	void DeletePreset(string name)
	{
		int id = GetPresetId(name);
		
		if (id == -1)
			return;
		
		m_aPresets.RemoveOrdered(id);
		
		SavePresetsToStorage();
		
		if (name == m_sUsedPreset)
			ClearUsedPreset();
	}
	
	//---------------------------------------------------------------------------------------------------
	bool PresetExists(string name)
	{
		return GetPresetId(name) != -1;
	}
	
	
	//---------------------------------------------------------------------------------------------------
	SCR_WorkshopAddonPreset GetPreset(string name)
	{
		int id = GetPresetId(name);
		
		if (id == -1)
			return null;
		
		return m_aPresets[id];
	}
	
	//---------------------------------------------------------------------------------------------------
	array<ref SCR_WorkshopAddonPreset> GetPresets()
	{
		return m_aPresets;
	}
	
	//-----------------------------------------------------------------------------------------
	// PROTECTED

	//---------------------------------------------------------------------------------------------------
	// Reads all presets from storage
	protected bool ReadPresetsFromStorage()
	{
		BaseContainer allPresetsContainer = GetGame().GetGameUserSettings().GetModule("SCR_WorkshopAddonManagerPresetStorage");
		
		if (allPresetsContainer)
			BaseContainerTools.WriteToInstance(this, allPresetsContainer);
		else
		{
			m_aPresets = {};
			m_iVersion = VERSION;
		}
		
		return true;
	}	

	//---------------------------------------------------------------------------------------------------
	//! Saves all presets to storage
	protected void SavePresetsToStorage()
	{		
		m_iVersion = VERSION;
		
		BaseContainer allPresetsContainer = GetGame().GetGameUserSettings().GetModule("SCR_WorkshopAddonManagerPresetStorage");
		BaseContainerTools.ReadFromInstance(this, allPresetsContainer);
		GetGame().UserSettingsChanged();
	}
	
	
	//---------------------------------------------------------------------------------------------------
	protected int GetPresetId(string name)
	{
		foreach (int i, SCR_WorkshopAddonPreset preset : m_aPresets)
		{
			if (preset.GetName() == name)
				return i;
		}
		
		return -1;
	}
}