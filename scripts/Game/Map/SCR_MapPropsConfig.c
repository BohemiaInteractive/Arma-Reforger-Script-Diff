//------------------------------------------------------------------------------------------------
//! Property type base
[BaseContainerProps()]
class SCR_MapPropsConfig
{
	//------------------------------------------------------------------------------------------------
	//! Set default values for props type
	void SetDefaults(MapLayer layer)
	{}
};

//------------------------------------------------------------------------------------------------
//! Configuration of building props
[BaseContainerProps()]
class SCR_BuildingPropsConfig : SCR_MapPropsConfig
{
	[Attribute("", UIWidgets.Object, "Building types", "")]
	ref array<ref SCR_BuildingTypeProps> m_aBuildingTypes;

	//------------------------------------------------------------------------------------------------
	override void SetDefaults(MapLayer layer)
	{
		MapBuildingProps buildingProps = layer.GetBuildingProps();

		if (m_aBuildingTypes.IsEmpty())
			return;

		foreach (SCR_BuildingTypeProps buildingType : m_aBuildingTypes)
		{
			buildingProps.SetOutlineWidth(buildingType.m_iType, buildingType.m_fOutlineWidth);
			buildingProps.SetLineColor(buildingType.m_iType, buildingType.m_FillColor);
			buildingProps.SetOutlineColor(buildingType.m_iType, buildingType.m_OutlineColor);
			buildingProps.SetLineType(buildingType.m_iType, buildingType.m_iOutlineStyle);

			if (layer.Index() <= buildingType.m_iViewLayerVisibility)	
				buildingProps.SetVisibility(buildingType.m_iType, true); 		// buildings are not visible by default
		}
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of visibility in layers per descriptor type
[BaseContainerProps(), SCR_BuildingViewTitle()]
class SCR_BuildingTypeProps
{
	[Attribute("0", UIWidgets.ComboBox, "Building type", "", ParamEnumArray.FromEnum(EMapBuildingType))]
	int m_iType;

	[Attribute("4", UIWidgets.EditBox, desc: "Layer ID, determines up to which layer is this descriptor type visible")]
	int m_iViewLayerVisibility;
	
	[Attribute("1", UIWidgets.EditBox, desc: "Width of the outline", "")]
	float m_fOutlineWidth;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, desc: "Outline color")]
	ref Color m_OutlineColor; 
	
	[Attribute("0", UIWidgets.ComboBox, "Building outline style", "", ParamEnumArray.FromEnum(EMapLineType))]
	int m_iOutlineStyle;
	
	[Attribute("0.5 0.5 0.5 1", UIWidgets.ColorPicker, desc: "Fill color")]
	ref Color m_FillColor; 
};

//------------------------------------------------------------------------------------------------
//! Custom names for defined buildings to make config setup easier
class SCR_BuildingViewTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		int type;
		source.Get("m_iType", type);
		title = typename.EnumToString(EMapBuildingType, type);

		return true;
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of grid props
[BaseContainerProps()]
class SCR_GridPropsConfig : SCR_MapPropsConfig
{		
	const int MAIN_GRID_SIZE = 100000;	// meters, has to be bigger than square size for grid coords to work
	
	[Attribute("1", UIWidgets.EditBox, desc: "Line width", "")]
	float m_fLineWidth;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, desc: "Line color")]
	ref Color m_LineColor;
	
	[Attribute("{3E7733BAC8C831F6}UI/Fonts/RobotoCondensed/RobotoCondensed_Regular.fnt", UIWidgets.ResourceNamePicker, desc: "Font", params: "fnt")]
	ResourceName m_TextFont;
	
	[Attribute("20", UIWidgets.EditBox, desc: "Font size")]
	int m_iTextSize;
	
	[Attribute("0", UIWidgets.CheckBox, desc: "Use bold text")]
	bool m_bBoldText;
	
	[Attribute("0", UIWidgets.CheckBox, desc: "Use italic text")]
	bool m_bItalicText;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, desc: "Text color")]
	ref Color m_TextColor;
	
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(MapLayer layer)
	{
		// Grid configuration
		MapGridProps gridProps = layer.GetGridProps();
		gridProps.SetMainGridSize(MAIN_GRID_SIZE);
		gridProps.SetLineWidth(m_fLineWidth);
		gridProps.SetMainLineWidth(m_fLineWidth*2);
		gridProps.SetLineColor(m_LineColor);
		gridProps.SetFont(m_TextFont);
		gridProps.SetFontsize(m_iTextSize); 
		gridProps.SetTextColor(m_TextColor);
		gridProps.SetMainGridLineColor(Color.FromInt(Color.BLACK));
		
		if (m_bBoldText)
			gridProps.SetTextBold();
		
		if (m_bItalicText)
			gridProps.SetTextItalic();
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of legend props
[BaseContainerProps()]
class SCR_LegendPropsConfig : SCR_MapPropsConfig
{	
	[Attribute("5", UIWidgets.EditBox, desc: "Line width")]
	float m_fLineWidth;
	
	[Attribute("5", UIWidgets.EditBox, desc: "Number of segments")]
	int m_iSegmentCount;
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, desc: "Segment color A")]
	ref Color m_SegmentColorA;

	[Attribute("0 0 0 1", UIWidgets.ColorPicker, desc: "Segment color B")]
	ref Color m_SegmentColorB;
	
	[Attribute("{E2CBA6F76AAA42AF}UI/Fonts/Roboto/Roboto_Regular.fnt", UIWidgets.ResourceNamePicker, desc: "Font", params: "fnt")]
	ResourceName m_TextFont;
	
	[Attribute("16", UIWidgets.EditBox, desc: "Font size")]
	int m_iTextSize;
	
	[Attribute("0", UIWidgets.CheckBox, desc: "Use bold text")]
	bool m_bBoldText;
	
	[Attribute("0", UIWidgets.CheckBox, desc: "Use italic text")]
	bool m_bItalicText;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, desc: "Distance text color")]
	ref Color m_TextColor;

	[Attribute("40", UIWidgets.EditBox, desc: "Offset percentage in X", "")]
	float m_fOffsetX;
	
	[Attribute("40", UIWidgets.EditBox, desc: "Offset percentage in Y", "")]
	float m_fOffsetY;

	[Attribute("0", UIWidgets.CheckBox, desc: "Set alignment from default left to the right")]
	bool m_bAlignRight;
	
	[Attribute("0", UIWidgets.CheckBox, desc: "Set alignment from default bot to the top")]
	bool m_bAlignTop;

	//------------------------------------------------------------------------------------------------
	override void SetDefaults(MapLayer layer)
	{		
		MapLegendProps legendProps = layer.GetLegendProps();
		legendProps.SetLineWidth(m_fLineWidth);
		legendProps.SetSegmentNumber(m_iSegmentCount);
		legendProps.SetSegment1Color(m_SegmentColorA);
		legendProps.SetSegment2Color(m_SegmentColorB); 
		legendProps.SetFont(m_TextFont); 
		legendProps.SetDistanceTextSize(m_iTextSize); 
		legendProps.SetUnitTextSize(m_iTextSize); 
		legendProps.SetUnitTextColor(m_TextColor); 
		legendProps.SetDistanceTextColor(m_TextColor);
		legendProps.SetOffsetX(m_fOffsetX);
		legendProps.SetOffsetY(m_fOffsetY);
		legendProps.SetAlignRight(m_bAlignRight);
		legendProps.SetAlignTop(m_bAlignTop);		
		
		if (m_bBoldText)
			legendProps.SetTextBold();
		
		if (m_bItalicText)
			legendProps.SetTextItalic();
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of contour props
[BaseContainerProps()]
class SCR_ContourPropsConfig : SCR_MapPropsConfig
{
	[Attribute("3", UIWidgets.EditBox, desc: "Global setting for all contour types. Contours are visible up to this layer, 0 for not visible", "0 100")]
	int m_iViewLayerVisibility;
		
	[Attribute("1.2", UIWidgets.EditBox, desc: "Contour width")]
	float m_fContourWidth;
		
	[Attribute("1.8", UIWidgets.EditBox, desc: "Major contour width")]
	float m_fMajorContourWidth;
	
	[Attribute("3", UIWidgets.EditBox, desc: "Coastal contour width")]
	float m_fCoastalContourWidth;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, desc: "Coastal contour color")]
	ref Color m_CoastalColor;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, desc: "Ocean contour color")]
	ref Color m_OceanColor;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, desc: "Land contour color")]
	ref Color m_LandColor;
	
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(MapLayer layer)
	{
		// Contour configuration
		MapContourProps contourProps = layer.GetContourProps();
		contourProps.SetMajorWidth(m_fMajorContourWidth);
		contourProps.SetContourWidth(m_fContourWidth);
		contourProps.SetCostalWidth(m_fCoastalContourWidth);
		
		contourProps.SetCoastColor(m_CoastalColor);
		contourProps.SetOceanColor(m_OceanColor);
		contourProps.SetLandColor(m_LandColor);
		
		if (layer.Index() > m_iViewLayerVisibility)	
			contourProps.EnableVisualisation(false); 
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of road props
[BaseContainerProps()]
class SCR_RoadPropsConfig : SCR_MapPropsConfig
{
	[Attribute("", UIWidgets.Object, "Road types", "")]
	ref array<ref SCR_RoadTypeProps> m_aRoadTypes;
	
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(MapLayer layer)
	{
		MapRoadProps roadProps = layer.GetRoadProps();
		
		if (m_aRoadTypes.IsEmpty())
			return;
				
		foreach (SCR_RoadTypeProps roadType : m_aRoadTypes)
		{			
			roadProps.SetLineWidth(roadType.m_iType, roadType.m_fWidth); 
			roadProps.SetBorderLineWidth(roadType.m_iType, roadType.m_fOutlineWidth);
			roadProps.SetColor(roadType.m_iType, roadType.m_Color);
			roadProps.SetBorderColor(roadType.m_iType, roadType.m_fOutlineColor);
			roadProps.SetLineType(roadType.m_iType, roadType.m_iStyle);
			
			if (roadType.m_iStyle == EMapLineType.LT_DASHED)	// there are only applied to dashed style road types
			{
				roadProps.SetDashedLineLength(roadType.m_iType, roadType.m_fDashedLineLength);
				roadProps.SetDashedLineGapLength(roadType.m_iType, roadType.m_fDashedLineGapLength);
				roadProps.SetSecondaryColor(roadType.m_iType, roadType.m_DashedSecondaryColor);
			}
			
			if (layer.Index() > roadType.m_iVisibility)	
				roadProps.SetVisibility(roadType.m_iType, false); 		// roads are visible by default
			
			if (layer.Index() <= roadType.m_iOutlineVisibility)
				roadProps.SetBorderVisibility(roadType.m_iType, true);	// outlines are invisible by default
		}		
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of visibility in layers per descriptor type
[BaseContainerProps(), SCR_RoadViewTitle()]
class SCR_RoadTypeProps
{
	[Attribute("0", UIWidgets.ComboBox, "Road type", "", ParamEnumArray.FromEnum(RoadType))]
	int m_iType;
	
	[Attribute("4", UIWidgets.EditBox, desc: "Layer ID, determines up to which layer is this road type visible, 0 means not visible")]
	int m_iVisibility;
		
	[Attribute("0", UIWidgets.ComboBox, "Road line style", "", ParamEnumArray.FromEnum(EMapLineType))]
	int m_iStyle;
	
	[Attribute("2.5", UIWidgets.EditBox, desc: "Runway width")]
	float m_fWidth;
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, desc: "Runway color")]
	ref Color m_Color;
	
	[Attribute("0", UIWidgets.EditBox, desc: "Layer ID, determines up to which layer is this road type outline visible, 0 means not visible")]
	int m_iOutlineVisibility;
	
	[Attribute("1", UIWidgets.EditBox, desc: "Runway outline width")]
	float m_fOutlineWidth;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, desc: "Runway outline color")]
	ref Color m_fOutlineColor;
	
	[Attribute("10", UIWidgets.EditBox, desc: "Dashed Line only! Segment length")]
	float m_fDashedLineLength;
	
	[Attribute("1", UIWidgets.EditBox, desc: "Dashed Line only! Gap length")]
	float m_fDashedLineGapLength;
	
	[Attribute("0 0 0 0", UIWidgets.ColorPicker, desc: "Dashed Line only! Secondary segment color")]
	ref Color m_DashedSecondaryColor;
};

//------------------------------------------------------------------------------------------------
//! Custom names for defined zones to make config setup easier
class SCR_RoadViewTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{		
		int type;
		source.Get("m_iType", type);
		title = typename.EnumToString(RoadType, type);
				
		return true;
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of powerline props
[BaseContainerProps()]
class SCR_PowerlinePropsConfig : SCR_MapPropsConfig
{
	[Attribute("", UIWidgets.Object, "Powerline types", "")]
	ref array<ref SCR_PowerlineTypeProps> m_aPowerlineTypes;
	
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(MapLayer layer)
	{
		MapPowerLineProps powerlineProps = layer.GetPowerLineProps();
		
		if (m_aPowerlineTypes.IsEmpty())
			return;
				
		foreach (SCR_PowerlineTypeProps powerlineType : m_aPowerlineTypes)
		{			
			powerlineProps.SetLineWidth(powerlineType.m_iType, powerlineType.m_fLineWidth);
			powerlineProps.SetLineColor(powerlineType.m_iType, powerlineType.m_LineColor);
			
			powerlineProps.SetIconViewLayer(powerlineType.m_iType, powerlineType.m_iIconVisibility);
			powerlineProps.SetIconQuality(powerlineType.m_iType, powerlineType.m_iIconQuality);
			powerlineProps.SetIconRadius(powerlineType.m_iType, powerlineType.m_fIconRadius);
			powerlineProps.SetIconSize(powerlineType.m_iType, powerlineType.m_fIconSize);
						
			if (layer.Index() > powerlineType.m_iVisibility)	
				powerlineProps.SetVisibility(powerlineType.m_iType, false); 		// powerlines are visible by default
		}
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of visibility in layers per descriptor type
[BaseContainerProps(), SCR_PowerlineViewTitle()]
class SCR_PowerlineTypeProps
{
	[Attribute("0", UIWidgets.ComboBox, "Powrline type", "", ParamEnumArray.FromEnum(EMapPowerlineType))]
	int m_iType;
	
	[Attribute("4", UIWidgets.EditBox, desc: "Layer ID, determines up to which layer is this powerline type visible, 0 means not visible")]
	int m_iVisibility;
	
	[Attribute("2.2", UIWidgets.EditBox, desc: "Line width")]
	float m_fLineWidth;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, desc: "Line color")]
	ref Color m_LineColor;
	
	[Attribute("0", UIWidgets.EditBox, desc: "Layer ID, determines up to which layer is this road type icon visible, 0 means not visible")]
	int m_iIconVisibility;
	
	[Attribute("1", UIWidgets.EditBox, desc: "radius of the icon")]
	float m_fIconRadius;
	
	[Attribute("12", UIWidgets.EditBox, desc: "quality of the icon")]
	float m_iIconQuality;
	
	[Attribute("1.5", UIWidgets.EditBox, desc: "size of the icon")]
	float m_fIconSize;
};

//------------------------------------------------------------------------------------------------
//! Custom names for defined zones to make config setup easier
class SCR_PowerlineViewTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{		
		int type;
		source.Get("m_iType", type);
		title = typename.EnumToString(EMapPowerlineType, type);
				
		return true;
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of water body props
[BaseContainerProps()]
class SCR_WaterBodyPropsConfig : SCR_MapPropsConfig
{
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, desc: "River and lake water color")]
	ref Color m_LandWaterColor;
	
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(MapLayer layer)
	{
		MapWaterBodyProps waterProps = layer.GetWaterBodyProps();
		waterProps.SetWaterColor(m_LandWaterColor);
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of hill props
[BaseContainerProps()]
class SCR_HillPropsConfig : SCR_MapPropsConfig
{
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, desc: "Icon color")]
	ref Color m_IconColor;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, desc: "Text color")]
	ref Color m_TextColor;
	
	[Attribute("200", UIWidgets.EditBox, desc: "Major hill floor limit")]
	float m_fMajorHillFloorLimit;
	
	[Attribute("100", UIWidgets.EditBox, desc: "Significant hill floor limit")]
	float m_fSignificantHillFloorLimit;
	
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(MapLayer layer)
	{
		MapHillProps hillProps = layer.GetHillProps();
		hillProps.SetMajorHillFloorLimit(m_fMajorHillFloorLimit);
		hillProps.SetSignificantHillFloorLimit(m_fSignificantHillFloorLimit);
		hillProps.SetColor(m_IconColor);
		hillProps.SetTextColor(m_TextColor);
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of area props
[BaseContainerProps()]
class SCR_AreaPropsConfig : SCR_MapPropsConfig
{	
	[Attribute("", UIWidgets.Object, "Area types", "")]
	ref array<ref SCR_AreaTypeProps> m_aAreaTypes;
	
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(MapLayer layer)
	{
		MapAreaProperties areaProps = layer.GetAreaProps();
		
		if (m_aAreaTypes.IsEmpty())
			return;
				
		foreach (SCR_AreaTypeProps areaType : m_aAreaTypes)
		{			
			areaProps.SetAreaColor(areaType.m_iType, areaType.m_Color);
			areaProps.SetIndividualColor(areaType.m_iType, areaType.m_ColorIndividual);
						
			if (layer.Index() > areaType.m_iVisibility)	
				areaProps.SetVisibility(areaType.m_iType, false); 	// areas are visible by default
			
			if (areaType.m_iVisibilityIndividual != 0 && layer.Index() < areaType.m_iVisibilityIndividual)
				areaProps.SetVisualizationType(areaType.m_iType, EMapAreaVisualizationType.VT_INDIVIDUAL);
			else 
				areaProps.SetVisualizationType(areaType.m_iType, EMapAreaVisualizationType.VT_AREA);
		}
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of visibility in layers per descriptor type
[BaseContainerProps(), SCR_AreaViewTitle()]
class SCR_AreaTypeProps
{
	[Attribute("0", UIWidgets.ComboBox, "Area type", "", ParamEnumArray.FromEnum(EMapAreaType))]
	int m_iType;
	
	[Attribute("6", UIWidgets.EditBox, desc: "Layer ID, determines up to which layer is this area type visible, 0 means not visible")]
	int m_iVisibility;
	
	[Attribute("0.5 0.5 0.5 1", UIWidgets.ColorPicker, desc: "Area color")]
	ref Color m_Color;
	
	[Attribute("0", UIWidgets.EditBox, desc: "Layer ID, if set, determines up to which layer is this area type visible in individual mode, 0 to disable")]
	int m_iVisibilityIndividual;
	
	[Attribute("0.5 0.5 0.5 1", UIWidgets.ColorPicker, desc: "Color when in individual object display")]
	ref Color m_ColorIndividual;
};

//------------------------------------------------------------------------------------------------
//! Custom names for defined zones to make config setup easier
class SCR_AreaViewTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{		
		int type;
		source.Get("m_iType", type);
		title = typename.EnumToString(EMapAreaType, type);
				
		return true;
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of drawing props
[BaseContainerProps()]
class SCR_DrawingPropsConfig : SCR_MapPropsConfig
{
	[Attribute("4", UIWidgets.EditBox, desc: "Layer ID, determines up to which layer is this descriptor type visible")]
	int m_iViewLayerVisibility;
	
	[Attribute("1", UIWidgets.CheckBox, desc: "Set if shape interior is filled or hollow")]
	bool m_bIsHollow;
	
	[Attribute("0", UIWidgets.ComboBox, "Shape type", "", ParamEnumArray.FromEnum(EMapDrawingShapeType))]
	int m_iShapeType;
	
	[Attribute("96", UIWidgets.EditBox, desc: "Level of detail", "0 255")]
	int m_iDetail;
	
	[Attribute("0", UIWidgets.ComboBox, "Shape outline type", "", ParamEnumArray.FromEnum(EMapLineType))]
	int m_iOutlineType;
	
	[Attribute("1", UIWidgets.EditBox, desc: "Width of the outline", "")]
	float m_fOutlineWidth;
	
	[Attribute("0 0 0 1", UIWidgets.ColorPicker, desc: "Outline color")]
	ref Color m_OutlineColor; 
	
	[Attribute("0.5 0.5 0.5 1", UIWidgets.ColorPicker, desc: "Fill color")]
	ref Color m_FillColor; 
	
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(MapLayer layer)
	{
		MapDrawingProps drawingProps = layer.GetMapDrawingProps();
		if (layer.Index() > m_iViewLayerVisibility)	
			drawingProps.SetVisible(false); 
		
		drawingProps.SetHollow(m_bIsHollow); 
		drawingProps.SetShapeType(m_iShapeType);
		drawingProps.SetDetail(m_iDetail);
		drawingProps.SetOutlineType(m_iOutlineType);
		drawingProps.SetOutlineWidth(m_fOutlineWidth);
		drawingProps.SetOutlineColor(m_OutlineColor);
		drawingProps.SetInteriorColor(m_FillColor);
	}
};