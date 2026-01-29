class SCR_TutorialFakeBaseComponentClass : ScriptComponentClass
{
}

class SCR_TutorialFakeBaseComponent : ScriptComponent
{
	[Attribute()]
	protected ref SCR_GraphLinesData m_GraphLinesData;
	
	protected SCR_MapEntity m_MapEntity;
	
	[Attribute("BASE NAME")];
	string m_sBaseName;
	
	[Attribute("BASE NAME LONG")];
	string m_sBaseNameLonger;
	
	[Attribute("AVRORA")];
	string m_sBaseCallSign;
	
	[Attribute("0.854994 0.031006 0.028000 1.000000", uiwidget: UIWidgets.ColorPicker)]
	ref Color m_BaseColor;
	
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EMilitarySymbolIdentity))]
	EMilitarySymbolIdentity m_eMilitarySymbolIdentity;
	
	[Attribute("2", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EMilitarySymbolDimension))]
	EMilitarySymbolDimension m_eMilitarySymbolDimension;
	
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EMilitarySymbolIcon))]
	EMilitarySymbolIcon m_eMilitarySymbolIcon;
	
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EMilitarySymbolAmplifier))]
	EMilitarySymbolAmplifier m_eMilitarySymbolAplifier;
	
	[Attribute("0", UIWidgets.Flags, "", enums: ParamEnumArray.FromEnum(SCR_EFakeBaseServices))]
	SCR_EFakeBaseServices m_eServices;
	
	[Attribute("Hostile_Installation_Focus_Land")]
	string m_sHighlight;
	
	[Attribute()]
	bool m_bAntennaWidgetVisible;
	
	[Attribute("1")]
	bool m_bAllowServices;
	
	[Attribute("1")]
	bool m_bAllowServicesSizeOverlay;
	
	[Attribute()]
	bool m_bVisible;
	
	[Attribute("38")]
	int m_iWidthOverride;
	
	[Attribute("38")]
	int m_iHeightOverride;
	
	[Attribute()]
	ref array <string> m_aBasesToLink;
	
	protected MapItem m_MapItem;
	protected ref array <MapLink> m_aLinks = {};
	
	//------------------------------------------------------------------------------------------------
	void RemoveLinkName(string link)
	{
		m_aBasesToLink.RemoveItem(link);
	}
	
	//------------------------------------------------------------------------------------------------
	void AddLinkName(string link)
	{
		m_aBasesToLink.Insert(link);
	}
	
	//------------------------------------------------------------------------------------------------
	void ColorLinks(bool isHoovering)
	{
		if (!m_aLinks || m_aLinks.IsEmpty())
			return;
		
		foreach (MapLink link : m_aLinks)
		{
			ColorMapLink(link, isHoovering);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearLinks()
	{
		if (m_MapItem)
			m_MapItem.ClearLinks();
		
		m_aLinks.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateLinks()
	{
		IEntity ent;
		MapDescriptorComponent mapDescriptor;
		
		foreach (string base : m_aBasesToLink)
		{
			ent = GetGame().GetWorld().FindEntityByName(base);
			if (!ent)
				continue;
			
			mapDescriptor = MapDescriptorComponent.Cast(ent.FindComponent(MapDescriptorComponent));
			if (!mapDescriptor)
				continue;
			
			MapLink link = m_MapItem.LinkTo(mapDescriptor.Item());
			ColorMapLink(link, false);
			
			m_aLinks.Insert(link);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ColorMapLink(notnull MapLink link, bool isHovering)
	{
		MapLinkProps props = link.GetMapLinkProps();

		if (!props)
			return;

		if (!m_GraphLinesData)
			return;

		props.SetLineWidth(m_GraphLinesData.GetLineWidth());

		Color c = Color.FromInt(props.GetLineColor().PackToInt());

		if (!c)
			return;

		if (isHovering)
			c.SetA(m_GraphLinesData.GetHighlightedAlpha());
		else
			c.SetA(m_GraphLinesData.GetDefaultAlpha());

		props.SetLineColor(c);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration config)
	{
		SCR_TutorialMapUI tutorialMapUI;
		foreach (SCR_MapUIBaseComponent mapUIComponent : config.Components)
		{
			tutorialMapUI = SCR_TutorialMapUI.Cast(mapUIComponent);
			if (tutorialMapUI)
				break;
		}
		
		if (tutorialMapUI)
			tutorialMapUI.InsertFakeBase(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{	
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		if (!m_MapEntity)
			return;
		
		m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		
		MapDescriptorComponent mapDescriptor = MapDescriptorComponent.Cast(owner.FindComponent(MapDescriptorComponent));
		if (!mapDescriptor)
			return;
		
		m_MapItem = mapDescriptor.Item();
		
		if (m_MapItem)
			m_MapItem.GetProps().SetIconVisible(false);
	}
}