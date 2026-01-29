[BaseContainerProps(configRoot: true)]
class SCR_GroupNameConfig
{
	[Attribute()]
	protected ref array<ref SCR_GroupNameConfigEntry> m_aEntries;
	
	/*!
	Get name of a group based on its symbol
	\param symbol Military symbol
	\return Group name
	*/
	LocalizedString GetGroupName(SCR_MilitarySymbol symbol)
	{
		foreach (SCR_GroupNameConfigEntry entry: m_aEntries)
		{
			if (entry.IsCompatible(symbol))
				return entry.GetName();
		}
		return LocalizedString.Empty;
	}
	/*!
	Get name of a group based on name ID from the list
	\param id Name ID
	\return Group name
	*/
	LocalizedString GetGroupName(int id)
	{
		if (id >= 0)
			return m_aEntries[id].GetName();
		else
			return string.Empty;
	}
	/*!
	Get name ID of a group based on its symbol
	\param symbol Military symbol
	\return Group name ID
	*/
	int GetGroupNameID(SCR_MilitarySymbol symbol)
	{
		foreach (int id, SCR_GroupNameConfigEntry entry: m_aEntries)
		{
			if (entry.IsCompatible(symbol))
				return id;
		}
		return -1;
	}
};
[BaseContainerProps(), SCR_BaseContainerGroupNameConfigEntry()]
class SCR_GroupNameConfigEntry
{
	[Attribute(uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sName;
	
	[Attribute("1", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolDimension))]
	protected EMilitarySymbolDimension m_Dimension;
	
	[Attribute("0", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EMilitarySymbolIcon))]
	protected EMilitarySymbolIcon m_Icons;
	
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolAmplifier))]
	protected EMilitarySymbolAmplifier m_Amplifier;
	
	string GetName()
	{
		return m_sName;
	}
	bool IsCompatible(SCR_MilitarySymbol symbol)
	{
		return (symbol.GetDimension() == m_Dimension || m_Dimension == EMilitarySymbolDimension.NONE)
			&& (symbol.GetIcons() == m_Icons || m_Icons == 0)
			&& (symbol.GetAmplifier() == m_Amplifier || m_Amplifier == EMilitarySymbolAmplifier.NONE);
	}
};

class SCR_BaseContainerGroupNameConfigEntry: BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{		
		EMilitarySymbolDimension dimension;
		source.Get("m_Dimension", dimension);
		
		EMilitarySymbolIcon icons;
		source.Get("m_Icons", icons);
		
		EMilitarySymbolAmplifier amplifier;
		source.Get("m_Amplifier", amplifier);
		
		string dimensionName = typename.EnumToString(EMilitarySymbolDimension, dimension);
		string iconNames = SCR_Enum.FlagsToString(EMilitarySymbolIcon, icons);
		string amplifierName = typename.EnumToString(EMilitarySymbolAmplifier, amplifier);
		
		title = string.Format("%1 | %2 | %3", dimensionName, iconNames, amplifierName);
		return true;
	}
};