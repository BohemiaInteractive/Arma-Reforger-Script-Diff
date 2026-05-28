[BaseContainerProps(configRoot: true)]
class SCR_MainMenuConfiguration : Managed
{
	[Attribute("", UIWidgets.Object, "MainMenu tutorial header", "conf")]
	ref SCR_MainMenuConfigEntry m_TutorialScenario;
	
	[Attribute("", UIWidgets.ResourceAssignArray, "MainMenu custom tiles", "layout")]
	ref array<ResourceName> m_aMainMenuCustomTiles;	
	
	[Attribute("", UIWidgets.Object, "MainMenu scenarios headers", "conf")]
	ref array<ref SCR_MainMenuConfigEntry> m_aMainMenuScenarios;
	
	[Attribute("8", UIWidgets.Slider, "How many scenarios will be displayed", "0 16 1")]
	int m_iMaxDisplayedScenarios;
}

[BaseContainerProps(configRoot: false), SCR_BaseContainerCustomTitleResourceName("m_sScenarioName")]
class SCR_MainMenuConfigEntry : Managed
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Scenario header", "conf")]
	ResourceName m_sScenarioName;
	
	[Attribute("", UIWidgets.CheckBox, "Available during loading")]
	bool m_bScenarioAvailableWhenDownloadingContent;
}
