//------------------------------------------------------------------------------------------------
//! Marker military symbol entry 
[BaseContainerProps(), SCR_MapMarkerTitle()]
class SCR_MapMarkerEntryMilitary : SCR_MapMarkerEntryConfig
{
	[Attribute("AR-MapMarker_Military", desc: "Description in selection menu")]
	protected string m_sMenuDescription;
	
	[Attribute("{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset", UIWidgets.ResourcePickerThumbnail, desc: "Imageset resource", params: "imageset")]
	protected ResourceName m_sMenuImageset;
	
	[Attribute("unknown_faction", desc: "Imageset icon")]
	protected string m_sMenuIcon;
	
	[Attribute(desc: "Predefined marker factions")]
	protected ref array<ref SCR_MarkerMilitaryFactionEntry> m_aMilitaryFactionEntries;
	
	[Attribute(desc: "Military dimension defines")]
	protected ref array<ref SCR_MarkerMilitaryDimension> m_aMilitaryDimensions;
	
	[Attribute(desc: "Military type defines")]
	protected ref array<ref SCR_MarkerMilitaryType> m_aMilitaryTypes;
	
	const int FACTION_DETERMINATOR = 100;
	const float DIMENSION_DETERMINATOR = 0.01;
	
	//------------------------------------------------------------------------------------------------
	string GetMenuDescription()
	{
	 	return m_sMenuDescription;
	}
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetMenuImageset()
	{
	 	return m_sMenuImageset;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetMenuIcon()
	{
	 	return m_sMenuIcon;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_MarkerMilitaryFactionEntry> GetMilitaryFactionEntries()
	{
		return m_aMilitaryFactionEntries;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_MarkerMilitaryFactionEntry GetFactionEntry(int i)
	{
		if (!m_aMilitaryFactionEntries.IsIndexValid(i))
			return null;
		
		return m_aMilitaryFactionEntries[i];
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFactionEntryID(EMilitarySymbolIdentity faction)
	{
		foreach (int id, SCR_MarkerMilitaryFactionEntry entry : m_aMilitaryFactionEntries)
		{
			if (entry.GetFactionIdentity() == faction)
				return id;
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_MarkerMilitaryDimension> GetMilitaryDimensions()
	{
		return m_aMilitaryDimensions;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_MarkerMilitaryDimension GetDimensionEntry(int i)
	{
		if (!m_aMilitaryDimensions.IsIndexValid(i))
			return null;
		
		return m_aMilitaryDimensions[i];
	}
	
	//------------------------------------------------------------------------------------------------
	int GetDimensionEntryID(EMilitarySymbolDimension dimension)
	{
		foreach (int id, SCR_MarkerMilitaryDimension entry : m_aMilitaryDimensions)
		{
			if (entry.GetDimension() == dimension)
				return id;
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_MarkerMilitaryType> GetMilitaryTypes()
	{
		return m_aMilitaryTypes;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_MarkerMilitaryType GetTypeEntry(int i)
	{
		if (!m_aMilitaryTypes.IsIndexValid(i))
			return null;
		
		return m_aMilitaryTypes[i];
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_EMapMarkerType GetMarkerType()
	{
	 	return SCR_EMapMarkerType.PLACED_MILITARY;
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitClientSettings(SCR_MapMarkerBase marker, SCR_MapMarkerWidgetComponent widgetComp, bool skipProfanityFilter = false)
	{
		super.InitClientSettings(marker, widgetComp);
		
		widgetComp.SetEventListening(true);
		widgetComp.SetMilitarySymbolMode(true);
		
		array<ref SCR_MarkerMilitaryFactionEntry> milFactionEntries = GetMilitaryFactionEntries();
		array<ref SCR_MarkerMilitaryDimension> milDimensions = GetMilitaryDimensions();
		array<ref SCR_MarkerMilitaryType> milTypes = GetMilitaryTypes();	
			
		int factionID = marker.GetMarkerConfigID() % FACTION_DETERMINATOR;
		int dimensionID = marker.GetMarkerConfigID() * DIMENSION_DETERMINATOR;
		
		if (!milFactionEntries || !milDimensions || !milFactionEntries.IsIndexValid(factionID) || !milDimensions.IsIndexValid(dimensionID))
			return;
		
		SCR_MilitarySymbol milSymbol = new SCR_MilitarySymbol();
		SCR_MarkerMilitaryFactionEntry factionEntry = milFactionEntries[factionID];
		milSymbol.SetIdentity(factionEntry.GetFactionIdentity());
		milSymbol.SetDimension(milDimensions[dimensionID].GetDimension());
		milSymbol.SetIcons(marker.GetFlags());
						
		widgetComp.UpdateMilitarySymbol(milSymbol);
		widgetComp.SetColor(factionEntry.GetColor());
		widgetComp.SetText(marker.GetCustomText());
		
		widgetComp.SetTimestampVisibility(marker.IsTimestampVisible());
		if (marker.IsTimestampVisible())
			widgetComp.SetTimestamp(marker.GetTimestamp());
		
		widgetComp.SetTypeIcon(1, milDimensions[dimensionID].GetTranslation());
		
		int typeFlags = marker.GetFlags();
		if (typeFlags == 0)
			return;
		
		bool secondType;	
		foreach (SCR_MarkerMilitaryType type : milTypes)
		{
			if (typeFlags & type.GetType())
			{
				if (secondType)
				{
					widgetComp.SetTypeIcon(3, type.GetTranslation());
					return;
				}
				else 
				{
					widgetComp.SetTypeIcon(2, type.GetTranslation());					
					if (typeFlags == type.GetType())	// return if singular flag
						return;
					
					secondType = true;
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapLayerChanged(notnull SCR_MapMarkerWidgetComponent widgetComp, int layerID)
	{
		if (layerID > 1) 
		{
			widgetComp.SetTextVisible(false);
		}
		else
		{
			widgetComp.SetTextVisible(true);	
		}	
	}
}

//------------------------------------------------------------------------------------------------
//! Marker military dimension
[BaseContainerProps(), SCR_MapMarkerMilitaryDimensionTitle()]
class SCR_MarkerMilitaryDimension
{
	[Attribute("", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolDimension))]
	protected EMilitarySymbolDimension m_eDimension;
		
	[Attribute("", desc: "Translation in UI")]
	protected string m_sTranslation;
		
	//------------------------------------------------------------------------------------------------
	EMilitarySymbolDimension GetDimension()
	{
		return m_eDimension;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTranslation()
	{
		return m_sTranslation;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_MapMarkerMilitaryDimensionTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{		
		
		EMilitarySymbolIcon symbol;
		source.Get("m_eDimension", symbol);

		title = typename.EnumToString(EMilitarySymbolDimension, symbol);
		
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Marker military type 
[BaseContainerProps(), SCR_MapMarkerMilitaryTypeTitle()]
class SCR_MarkerMilitaryType
{
	[Attribute("", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolIcon))]
	protected EMilitarySymbolIcon m_eType;
		
	[Attribute("", desc: "Translation in UI")]
	protected string m_sTranslation;
		
	//------------------------------------------------------------------------------------------------
	EMilitarySymbolIcon GetType()
	{
		return m_eType;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTranslation()
	{
		return m_sTranslation;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_MapMarkerMilitaryTypeTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{		
		
		EMilitarySymbolIcon symbol;
		source.Get("m_eType", symbol);

		title = typename.EnumToString(EMilitarySymbolIcon, symbol);
		
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Class container which holds faction specific predefined marker entries
[BaseContainerProps(), SCR_MapMarkerFactionTitle()]
class SCR_MarkerMilitaryFactionEntry
{
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolIdentity))]
	protected EMilitarySymbolIdentity m_FactionIdentity;
	
	[Attribute("1.0 1.0 1.0 1.0")]
	protected ref Color m_FactionColor;
	
	[Attribute("{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset", UIWidgets.ResourcePickerThumbnail, desc: "Imageset resource", params: "imageset")]
	protected ResourceName m_sIconImageset;
	
	[Attribute("unknown_faction", desc: "Imageset quad")]
	protected string m_sIconImagesetQuad;
	
	[Attribute("", desc: "Translation in UI")]
	protected string m_sTranslation;
	
	//------------------------------------------------------------------------------------------------
	EMilitarySymbolIdentity GetFactionIdentity()
	{
		return m_FactionIdentity;
	}
	
	//------------------------------------------------------------------------------------------------
	Color GetColor()
	{
		return Color.FromInt(m_FactionColor.PackToInt());
	}
	
	//------------------------------------------------------------------------------------------------
	void GetIconResource(out ResourceName imageset, out string imageQuad)
	{
		imageset = m_sIconImageset;
		imageQuad = m_sIconImagesetQuad;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTranslation()
	{
		return m_sTranslation;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_MapMarkerFactionTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{		
		EMilitarySymbolIdentity faction;
		source.Get("m_FactionIdentity", faction);
				
		title = typename.EnumToString(EMilitarySymbolIdentity, faction);
		
		return true;
	}
}
