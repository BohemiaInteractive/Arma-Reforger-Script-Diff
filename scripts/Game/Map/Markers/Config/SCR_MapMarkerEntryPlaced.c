//------------------------------------------------------------------------------------------------
//! Marker entry which can be placed through map
[BaseContainerProps(), SCR_MapMarkerTitle()]
class SCR_MapMarkerEntryPlaced : SCR_MapMarkerEntryConfig
{
	[Attribute("#AR-MapMarker_CustomHint", desc: "Description in selection menu")]
	protected string m_sMenuDescription;
	
	[Attribute("{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset", UIWidgets.ResourcePickerThumbnail, desc: "Imageset resource", params: "imageset")]
	protected ResourceName m_sMenuImageset;
	
	[Attribute("settings", desc: "Imageset icon")]
	protected string m_sMenuIcon;
	
	[Attribute("", UIWidgets.Object, "Categories of marker icons for sorting")]
	protected ref array<ref SCR_MarkerIconCategory> m_aPlacedMarkerIconCategories;
	
	[Attribute("", UIWidgets.Object, "Icons which can be chosen when placing a custom marker")]
	protected ref array<ref SCR_MarkerIconEntry> m_aPlacedMarkerIcons;
	
	[Attribute("", UIWidgets.Object, "Colors which can be chosen when placing a custom marker")]
	protected ref array<ref SCR_MarkerColorEntry> m_aPlacedMarkerColors;
		
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
	Color GetColorEntry(int i)
	{
		if (!m_aPlacedMarkerColors.IsIndexValid(i))
			return Color.FromInt(Color.WHITE);
		
		return m_aPlacedMarkerColors[i].GetColor();
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetIconEntry(int i, out ResourceName imageset, out ResourceName imagesetGlow, out string imageQuad)
	{
		if (!m_aPlacedMarkerIcons.IsIndexValid(i))
			return false;
		
		m_aPlacedMarkerIcons[i].GetIconResource(imageset, imagesetGlow, imageQuad);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetIconCategoryID(int iconID)
	{
		if (!m_aPlacedMarkerIcons.IsIndexValid(iconID))
			return null;
		
		string identifier = m_aPlacedMarkerIcons[iconID].m_sCategoryIdentifier;
		
		foreach (int i, SCR_MarkerIconCategory category : m_aPlacedMarkerIconCategories)
		{
			if (identifier == category.m_sIdentifier)
				return i;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_MarkerColorEntry> GetColorEntries()
	{
		return m_aPlacedMarkerColors;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_MarkerIconEntry> GetIconEntries()
	{
		return m_aPlacedMarkerIcons;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_MarkerIconCategory> GetIconCategories()
	{
		return m_aPlacedMarkerIconCategories;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_EMapMarkerType GetMarkerType()
	{
	 	return SCR_EMapMarkerType.PLACED_CUSTOM;
	}
	
	//------------------------------------------------------------------------------------------------
	override void InitClientSettings(SCR_MapMarkerBase marker, SCR_MapMarkerWidgetComponent widgetComp, bool skipProfanityFilter = false)
	{
		super.InitClientSettings(marker, widgetComp);
				
		ResourceName imageset, imagesetGlow;
		string quad;
		GetIconEntry(marker.GetIconEntry(), imageset, imagesetGlow, quad);
		widgetComp.SetImage(imageset, quad);			
		widgetComp.SetText(marker.GetCustomText(), skipProfanityFilter);
		widgetComp.SetColor(GetColorEntry(marker.GetColorEntry()));
		if (imagesetGlow)
			widgetComp.SetGlowImage(imagesetGlow, quad);
		
		widgetComp.SetEventListening(true);
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
//! Placed marker color entry
[BaseContainerProps()]
class SCR_MarkerColorEntry
{
	[Attribute("1.0 1.0 1.0 1.0")]
	protected ref Color m_Color;
	
	[Attribute(desc: "Color name, used for pairing settings with predefined markers")]
	protected string m_sName;

	//------------------------------------------------------------------------------------------------
	Color GetColor()
	{
		return Color.FromInt(m_Color.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	//! \return Name
	string GetName()
	{
		return m_sName;
	}
	
}

//------------------------------------------------------------------------------------------------
//! Placed marker icon category
[BaseContainerProps()]
class SCR_MarkerIconCategory
{
	[Attribute("", desc: "Set this wihin the icon to link it to this category")]
	string m_sIdentifier;
	
	[Attribute("", desc: "Name of the category")]
	string m_sName;
}

//------------------------------------------------------------------------------------------------
//! Placed marker icon entry
[BaseContainerProps(), SCR_MapMarkerIconEntryTitle()]
class SCR_MarkerIconEntry
{
	[Attribute("general", desc: "Category tab within marker dialog")]
	string m_sCategoryIdentifier;
	
	[Attribute("{E23427CAC80DA8B7}UI/Textures/Icons/icons_mapMarkersUI.imageset", UIWidgets.ResourcePickerThumbnail, desc: "Image resource", params: "imageset")]
	protected ResourceName m_sIconImageset;
	
	[Attribute("{67B3A6DC2D712B52}UI/Textures/Icons/icons_mapMarkersUI-glow.imageset", UIWidgets.ResourcePickerThumbnail, desc: "Image resource", params: "imageset")]
	protected ResourceName m_sIconGlowImageset;
	
	[Attribute("", desc: "Imageset quad")]
	protected string m_sIconImagesetQuad;
	
	//------------------------------------------------------------------------------------------------
	void GetIconResource(out ResourceName imageset, out ResourceName imagesetGlow, out string imageQuad)
	{
		imageset = m_sIconImageset;
		imagesetGlow = m_sIconGlowImageset;
		imageQuad = m_sIconImagesetQuad;
	}
}

//------------------------------------------------------------------------------------------------ 
class SCR_MapMarkerTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{				
		typename tName = source.GetClassName().ToType();
		SCR_MapMarkerEntryConfig cfg = SCR_MapMarkerEntryConfig.Cast(tName.Spawn());
		if (cfg)
			title = source.GetClassName() + ": " + typename.EnumToString(SCR_EMapMarkerType, cfg.GetMarkerType());
				
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_MapMarkerIconEntryTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{			
		string quad, identifier;
		source.Get("m_sCategoryIdentifier", identifier);
		source.Get("m_sIconImagesetQuad", quad);
		
		title = identifier + ": " + quad; 
				
		return true;
	}
}
