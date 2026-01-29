//! Commands config root
[BaseContainerProps(configRoot: true)]
class SCR_PlayerCommandsConfig : Managed
{
	[Attribute("", UIWidgets.Object, "Available commands")]
	protected ref array<ref SCR_BaseRadialCommand> m_aCommands;
	
	array<ref SCR_BaseRadialCommand> GetCommands()
	{
		return m_aCommands;
	}
};

//! Commanding menu config root
[BaseContainerProps(configRoot: true)]
class SCR_PlayerCommandingMenuConfig : Managed
{
	[Attribute(desc: "Commanding configs and actions pairs")]
	protected ref SCR_PlayerCommandingMenuCategoryElement m_RootCategory;
	
	SCR_PlayerCommandingMenuCategoryElement GetRootCategory()
	{
		return m_RootCategory;
	}
};

//! Commanding menus config to setup different menus to different keybinds
[BaseContainerProps(configRoot: true)]
class SCR_PlayerCommandingMenuActionsSetup : Managed
{
	[Attribute("", UIWidgets.Object, "Commanding configs and actions pairs")]
	ref array<ref SCR_PlayerCommandingConfigActionPair> m_aActionConfigPairs;
};

[BaseContainerProps()]
class SCR_PlayerCommandingConfigActionPair : Managed
{
	[Attribute("", UIWidgets.EditBox, "Name of the action in ChimeraInputCommon.conf that triggers menu with selected config" )]
	protected string m_sActionName;
	
	[Attribute()]
	protected ResourceName m_sCommandingMenuConf;
	
	//------------------------------------------------------------------------------------------------
	string GetActionName()
	{
		return m_sActionName;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetConfig()
	{
		return m_sCommandingMenuConf;
	}
}

//! Commanding menu commanding element class
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("m_sCommandName")]
class SCR_PlayerCommandingMenuCommand : SCR_PlayerCommandingMenuBaseElement
{
	[Attribute("", UIWidgets.EditBox, "Name of the command used from SCR_PlayerCommandsConfig" )]
	protected string m_sCommandName;
	
	protected string m_sCommandCustomDisplayText;
	
	//------------------------------------------------------------------------------------------------
	string GetCommandName()
	{
		return m_sCommandName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCommandName(string name)
	{
		m_sCommandName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCommandCustomName()
	{
		return m_sCommandCustomDisplayText;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCommandCustomName(string customName)
	{
		m_sCommandCustomDisplayText = customName;
	}
};

//! Commanding menu base element class
[BaseContainerProps()]
class SCR_PlayerCommandingMenuBaseElement : Managed
{

};

//! Commanding menu base element class
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("m_sCategoryDisplayText")]
class SCR_PlayerCommandingMenuCategoryElement : SCR_PlayerCommandingMenuBaseElement
{
	[Attribute("", UIWidgets.EditBox, "Name of the category that is displayed in the menu" )]
	protected string m_sCategoryDisplayText;
	
	[Attribute("", UIWidgets.Object, "Elements in the given category")]
	protected ref array<ref SCR_PlayerCommandingMenuBaseElement> m_aElements;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Imageset source from which icon will be selected. If this field is left empty then default icon will be used.")]
	protected ResourceName m_sImageset;

	[Attribute("", UIWidgets.EditBox, "Name of the icon associated to the category, taken from appropriate imageset set in the radial menu.\nIf this field is left empty then default icon will be used")]
	protected string m_sIconName;

	[Attribute()]
	protected bool m_bShowOnMap;
	
	//------------------------------------------------------------------------------------------------
	string GetCategoryDisplayText()
	{
		return m_sCategoryDisplayText;
	}

	//------------------------------------------------------------------------------------------------
	void SetCategoryDisplayText(string displayText)
	{
		m_sCategoryDisplayText = displayText;
	}	
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_PlayerCommandingMenuBaseElement> GetCategoryElements()
	{
		return m_aElements;
	}	
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetIconImageset()
	{
		return m_sImageset;
	}

	//------------------------------------------------------------------------------------------------
	string GetIconName()
	{
		return m_sIconName;
	}

	//------------------------------------------------------------------------------------------------
	bool GetCanShowOnMap()
	{
		return m_bShowOnMap;
	}
};

class SCR_BaseGroupCommandTitleField : BaseContainerCustomTitleField
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		// Make sure variable exists
		int index = source.GetVarIndex("m_sCommandName");
		if (index == -1)
			return false;

		// Tag string
		source.Get("m_sCommandName", title);

		return true;
	}
}