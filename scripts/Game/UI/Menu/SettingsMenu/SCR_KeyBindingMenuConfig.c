/*
Configuration of keybindings visible in the keybindings menu.
*/

//------------------------------------------------------------------------------------------------


[BaseContainerProps(configRoot: true)]
class SCR_KeyBindingMenuConfig : Managed
{			
	[Attribute("", UIWidgets.Object, "Description", "")]
	ref array<ref SCR_KeyBindingCategory> m_KeyBindingCategories;
	
	[Attribute("", UIWidgets.Object, "Input filters available", "")]
	ref array<ref SCR_KeyBindingFilter> m_aInputFilters;
	
	[Attribute("", UIWidgets.Object, "Addition binds able to be manually bound", "")]
	ref array<ref SCR_KeyBindingBind> m_aInputBinds;
	
	[Attribute("", UIWidgets.Object, "Keys that are available to be bound as combos i.e.: Ctrl, Shift", "")]
	ref array<ref SCR_KeyBindingCombo> m_aComboKeys;
};

class SCR_KeyBindingEntryTitleField : BaseContainerCustomTitleField
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		// Make sure variable exists
		int index = source.GetVarIndex("m_sActionName");
		if (index == -1)
			return false;

		// Tag string
		source.Get("m_sActionName", title);
		if (title == "separator")
		{
			title = "= ";
			LocalizedString name = "";
			source.Get("m_sDisplayName", name);
			title += name;
		}
		else
		{
			title = " - " + title;
		}

		string preset = "";
		source.Get("m_sPreset", preset);

		// Setup title string
		if (!preset.IsEmpty())
			title += " (" + preset + ")";

		return true;
	}
}

[BaseContainerProps("", "Entry of one key binding"), SCR_KeyBindingEntryTitleField("m_sActionName")]
class SCR_KeyBindingEntry
{
	[Attribute("", UIWidgets.EditBox, "Action name as defined in the ActionManager config")]
	string m_sActionName;
	
	[Attribute("", UIWidgets.EditBox, "Name of gamepad action to be used instead of main one, for instance Aim has separate action for controller")]
	string m_sActionNameGamepadOptional;
	
	[Attribute("", UIWidgets.LocaleEditBox, "Visible name of the action, long texts are not supported, name shouldn't be over 15 characters ")]
	LocalizedString m_sDisplayName;
	
	[Attribute("", UIWidgets.EditBox, "Show only Action sources with this preset.\nUsed for example when multiple directions are mapped to one action, like moving forward with value +1 and backward with value -1.")]
	string m_sPreset;
	
	[Attribute("", UIWidgets.EditBox, "Same as the Preset above, but this one should be filled only if it differs for the gamepad optional action.")]
	string m_sPresetGamepadOptional;
	
	[Attribute(SCR_Enum.GetDefault(SCR_EActionPrefixType.NON_CONTINUOUS), UIWidgets.ComboBox, "Type of prefix that can be applied to this action", "", ParamEnumArray.FromEnum(SCR_EActionPrefixType))]
	SCR_EActionPrefixType m_ePrefixType;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc: "Displayed only on specified platforms if defined", enums: ParamEnumArray.FromEnum(EPlatform))]
	ref array<EPlatform> m_aPlatforms;
};

[BaseContainerProps("", "Category of key bindings"), BaseContainerCustomTitleField("m_sName")]
class SCR_KeyBindingCategory
{
	[Attribute("", UIWidgets.EditBox, "Technical name of the category")]
	string m_sName;
	
	[Attribute("", UIWidgets.LocaleEditBox, "Visible name of the category")]
	LocalizedString m_sDisplayName;
	
	[Attribute("", UIWidgets.Object, "Description", "")]
	ref array<ref SCR_KeyBindingEntry> m_KeyBindingEntries;
};

[BaseContainerProps("", "Input filters"), BaseContainerCustomTitleField("m_sFilterString")]
class SCR_KeyBindingFilter
{	
	[Attribute("", UIWidgets.Auto, "Name of the filter use the same name as defined in enfusion", "")]
	string m_sFilterString;
	
	[Attribute("", UIWidgets.Auto, "Display name of filter", "")]
	string m_sFilterDisplayName;
	
	[Attribute("0", UIWidgets.ComboBox, "PrefixType", "", ParamEnumArray.FromEnum(SCR_EActionPrefixType))]
	SCR_EActionPrefixType m_eFilterType;
	
	//------------------------------------------------------------------------------------------------
	SCR_EActionPrefixType GetFilterType()
	{
		return m_eFilterType;
	}
}

[BaseContainerProps("", "Custom keybinds"), BaseContainerCustomTitleField("m_sBindString")]
class SCR_KeyBindingBind
{	
	[Attribute("", UIWidgets.Auto, "Name of the bind use the same name as defined in enfusion", "")]
	string m_sBindString;
	
	[Attribute("", UIWidgets.Auto, "Display name of bind", "")]
	string m_sBindDisplayName;
}

[BaseContainerProps("", "Custom bindable combo keys"), BaseContainerCustomTitleField("m_sComboString")]
class SCR_KeyBindingCombo
{
	[Attribute("", UIWidgets.Auto, "Name of the combo bind, for instance keyboard:KC_LCONTROL", "")]
	string m_sComboString;
	
	[Attribute("", UIWidgets.Auto, "Display name of combo", "")]
	string m_sComboDisplayName;
}

enum SCR_EActionPrefixType
{
	NON_CONTINUOUS,
	CONTINUOUS,
	AMBIGUOUS
}