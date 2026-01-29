//! Controller presets config root
[BaseContainerProps(configRoot: true)]
class SCR_ControllerPresetsConfig : Managed
{
	[Attribute("", UIWidgets.Object, "Available presets.")]
	protected ref array<ref SCR_ControllerPreset> m_aPresets;
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_ControllerPreset> GetPresets()
	{
		return m_aPresets;
	}
};

//! Controller preset config class
[BaseContainerProps()]
class SCR_ControllerPreset
{
	[Attribute("", desc: "GUID of the preset config, if left empty the preset will set controls to default", params: "conf")]
	protected ResourceName m_sPreset;
	
	[Attribute("", desc: "Display name to show in settings")]
	protected string m_sPresetDisplayName;
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetResourceName()
	{
		return m_sPreset;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetDisplayName()
	{
		return m_sPresetDisplayName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetResourceName(string preset)
	{
		m_sPreset = preset;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDisplayName(string name)
	{
		m_sPresetDisplayName = name;
	}
}

