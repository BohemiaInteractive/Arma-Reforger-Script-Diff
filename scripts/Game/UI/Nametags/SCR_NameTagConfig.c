//------------------------------------------------------------------------------------------------
//! Nametag zone config root
[BaseContainerProps(configRoot: true)]
class SCR_NameTagConfig : Managed
{	
	const int REFERENCE_FOV = 75;	// this should be decided and placed in constants
	const int MAX_ZONE_RANGE = 500;
	
	[Attribute("{703ABED1095791F4}UI/layouts/HUD/NameTag/NameTag.layout", UIWidgets.ResourceNamePicker, desc: "Nametag layout", params: "layout")]
	ResourceName m_sNametagLayout;
		
	[Attribute("", UIWidgets.Object, "Type of entity data collected")]
	ref SCR_NameTagCollectorBase m_aSourceEntities;
	
	[Attribute("", UIWidgets.Object, "Visibility ruleset module")]
	ref SCR_NameTagRulesetBase m_aVisibilityRuleset;
	
	[Attribute("false", UIWidgets.CheckBox, "This will adjust zone ranges relative to scope zoom, nametags will be displayed further")]
	bool m_bAdjustZoneRangeOnZoom;
		
	[Attribute("", UIWidgets.Object, desc: " Nametag zones \n ZONES NEED TO BE SORTED FROM CLOSEST TO FURTHEST")]
	ref array<ref SCR_NameTagZone> m_aZones;
		
	int m_iZoneCount;
	float m_fFarthestZoneRange;			// current farthest zone range
	float m_fFarthestZoneRangeOriginal;	// saved for reverting the changes to zone range
	float m_fFarthestZoneRangePow2;		// for distance calculations
		
	//------------------------------------------------------------------------------------------------
	//! Adjust zone range based on zoom
	//! \param FOVZoomed is the new FOV we are adjusting to
	void AdjustZoneRanges(float FOVZoomed)
	{
		if (FOVZoomed == 0 || !m_bAdjustZoneRangeOnZoom)
			return;
			
		float modifier = REFERENCE_FOV/FOVZoomed;
		
		for (int i = 0; i < m_iZoneCount; i++)	// individual zone adjustment
		{
			m_aZones[i].m_iZoneEndSq = Math.Pow(m_aZones[i].m_iZoneEnd * modifier, 2);
			if (m_aZones[i].m_iZoneEndSq > MAX_ZONE_RANGE)
				m_aZones[i].m_iZoneEndSq = MAX_ZONE_RANGE;
		}
		
		m_fFarthestZoneRange = m_fFarthestZoneRange * modifier;	// cached last zone adjustment
		if (m_fFarthestZoneRange > MAX_ZONE_RANGE)
			m_fFarthestZoneRange = MAX_ZONE_RANGE;
		
		m_fFarthestZoneRangePow2 = m_fFarthestZoneRange * m_fFarthestZoneRange;
	}
	
	//------------------------------------------------------------------------------------------------
	void RestoreZoneRanges()
	{
		for (int i = 0; i < m_iZoneCount; i++)
		{
			m_aZones[i].m_iZoneEndSq = m_aZones[i].m_iZoneEnd * m_aZones[i].m_iZoneEnd;
		}
		
		m_fFarthestZoneRange = m_fFarthestZoneRangeOriginal;
		m_fFarthestZoneRangePow2 = m_fFarthestZoneRangeOriginal * m_fFarthestZoneRangeOriginal;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_NameTagConfig()
	{
		m_iZoneCount = m_aZones.Count();
		
		int farthestZone;

		// Max distance
		for (int i = 0; i < m_iZoneCount; i++)
		{
			int zoneEnd = m_aZones[i].m_iZoneEnd;
			
			if (zoneEnd > farthestZone)
			{
				farthestZone = zoneEnd;
				m_fFarthestZoneRange = zoneEnd;
				m_fFarthestZoneRangeOriginal = zoneEnd;
				m_fFarthestZoneRangePow2 = zoneEnd * zoneEnd;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_NameTagConfig()
	{
		m_aZones = null;
	}
};

//------------------------------------------------------------------------------------------------
//! Custom names for defined zones to make config setup easier
class SCR_NameTagZoneTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{		
		source.Get("m_sZoneName", title);
		return true;
	}
};

//------------------------------------------------------------------------------------------------
//! Nametag zone config setup
[BaseContainerProps(), SCR_NameTagZoneTitle()]
class SCR_NameTagZone
{
	[Attribute("Zone1", UIWidgets.EditBox, "Optional name for the zone")]
	protected string m_sZoneName;
	
	[Attribute("0", UIWidgets.CheckBox, desc: " Metres \n Zone start ")]
	protected int m_iZoneStart;
	
	[Attribute("100", UIWidgets.CheckBox, desc: " Metres \n Zone end ")]
	int m_iZoneEnd;
		
	[Attribute("", UIWidgets.Object, "Nametag elements")]
	protected ref array<ref SCR_NTElementBase> m_aElements;
	
	int m_iZoneStartSq;
	int m_iZoneEndSq;		// used in distance calculations
	int m_iElementsCount;
	
	//------------------------------------------------------------------------------------------------
	//! Getter - nametag zone name
	//! \return string zone name
	string GetZoneName()
	{
		return m_sZoneName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Getter - minimum distance for the nametag to be drawn in this zone
	//! \return min draw distance 
	int GetZoneStart()
	{
		return m_iZoneStart;
	}
	
	//------------------------------------------------------------------------------------------------
	//! On nametag switching to different zone
	//! \param data is nametag struct 
	void OnZoneChanged(SCR_NameTagData data)
	{								
		for (int i = 0; i < m_iElementsCount; i++)
		{
			m_aElements[i].SetDefaults(data, i);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Iterate through and draw every element(widget) assigned to this zone
	//! \param data is nametag struct class
	void DrawElements(SCR_NameTagData data)
	{		
		for (int i = 0; i < m_iElementsCount; i++)
		{
			m_aElements[i].UpdateElement(data, i);
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Iterate through and set every element to default values according to their state
	//! \param data is nametag struct class
	void UpdateElementDefs(SCR_NameTagData data)
	{		
		for (int i = 0; i < m_iElementsCount; i++)
		{
			m_aElements[i].SetDefaults(data, i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clear elements upon zone switch since some may not be configured in the current zone anymore
	//! \param data is nametag struct class
	void HideElements(SCR_NameTagData data)
	{
		for (int i = 0; i < m_iElementsCount; i++)
		{
			Widget widget = data.m_NameTagWidget.FindAnyWidget(m_aElements[i].m_sWidgetName);
			if (widget)
				widget.SetVisible(false);
		}
	}
			
	//------------------------------------------------------------------------------------------------	
	void SCR_NameTagZone()
	{
		m_iZoneStartSq = m_iZoneStart * m_iZoneStart;
		m_iZoneEndSq = m_iZoneEnd * m_iZoneEnd;
		m_iElementsCount = m_aElements.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_NameTagZone()
	{
		if (m_aElements)
			m_aElements.Clear();
				
		m_aElements = null;
	}
};
