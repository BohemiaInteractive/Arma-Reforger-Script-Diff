//------------------------------------------------------------------------------------------------
//! Map config root
[BaseContainerProps(configRoot: true)]
class SCR_MapConfig
{			
	[Attribute("0", UIWidgets.ComboBox, "Map mode", "", ParamEnumArray.FromEnum(EMapEntityMode), category: "Map Config")]
	int m_iMapMode;
		
	[Attribute("", UIWidgets.Object, desc: "Layers config", "conf class=SCR_MapLayersBase")]
	ref SCR_MapLayersBase m_LayersConfig;
				
	[Attribute("", UIWidgets.Object, desc: "Map properties config", "conf class=SCR_MapPropsBase")]
	ref SCR_MapPropsBase m_MapPropsConfig;
	
	[Attribute("", UIWidgets.Object, "Map modules")]
	ref array<ref SCR_MapModuleBase> m_aModules;
	
	[Attribute("", UIWidgets.Object, "Map UI components")]
	ref array<ref SCR_MapUIBaseComponent> m_aUIComponents;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Legend scale component")]	// TODO use UIWidget.Flags here EMapOtherComponents
	bool m_bEnableLegendScale;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Grid component")]
	bool m_bEnableGrid;
			
	[Attribute("", UIWidgets.Object, desc: "Descriptor visibility config", "conf class=SCR_MapDescriptorVisibilityBase")]
	ref SCR_MapDescriptorVisibilityBase m_DescriptorVisibilityConfig;
	
	[Attribute("", UIWidgets.Object, desc: "Descriptor defaults config", "conf class=SCR_MapDescriptorDefaults")]
	ref SCR_MapDescriptorDefaults m_DescriptorDefaultsConfig;
};
//------------------------------------------------------------------------------------------------
//! Configuration of visibility in layers per descriptor type
[BaseContainerProps(), SCR_DescriptorViewTitle()]
class SCR_DescriptorViewLayer
{
	[Attribute("0", UIWidgets.ComboBox, "Descriptor type", "", ParamEnumArray.FromEnum(EMapDescriptorType))]
	int m_iDescriptorType;
	
	[Attribute("0", UIWidgets.EditBox, desc: "Layer ID, determines up to which layer is this descriptor type visible, 0 means not visible")]
	int m_iViewLayer;
};

//------------------------------------------------------------------------------------------------
//! Custom names for defined zones to make config setup easier
class SCR_DescriptorViewTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{		
		int type;
		source.Get("m_iDescriptorType", type);
		title = typename.EnumToString(EMapDescriptorType, type);
				
		return true;
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_LayerConfiguration
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Determines value at which layer switches to a higher one \n Value is in 1000*units (kilometers) visible per 2000 pixels", params: "0.1 100")]
	float m_fLayerCeiling;
	
	[Attribute(defvalue: "100", uiwidget: UIWidgets.EditBox, desc: "Set map grid square size")]
	float m_fGridSquareSize;
	
	[Attribute(defvalue: "50", uiwidget: UIWidgets.EditBox, desc: "Set map legend scale size in meters")]
	float m_fLegendScaleSize;
	
	[Attribute(defvalue: "10", uiwidget: UIWidgets.EditBox, desc: "Set density of minor countours, distance between the contour lines, lower layers should have lower numbers")]
	float m_fContourDensity;
	
	[Attribute(defvalue: "50", uiwidget: UIWidgets.EditBox, desc: "Set density of major contours, distance between the contour lines")]
	float m_fMajorContourDensity;
	
	//------------------------------------------------------------------------------------------------
	void SetLayerProps(MapLayer layer)
	{
		layer.SetCeiling(2 / m_fLayerCeiling);  // 2000/1000 px as a reference val, divided by configured units(kilometers)
		
		MapGridProps gridProps = layer.GetGridProps();
		gridProps.SetGridStepSize(m_fGridSquareSize);
				
		MapContourProps contProps = layer.GetContourProps();
		contProps.SetContourDensity(m_fContourDensity);
		contProps.SetMajorDensity(m_fMajorContourDensity);
		
		MapLegendProps legendProps = layer.GetLegendProps();
		legendProps.SetTotalSegmentLength(m_fLegendScaleSize);
	}
};

//------------------------------------------------------------------------------------------------
//! Map props root
[BaseContainerProps(configRoot: true)]
class SCR_MapPropsBase
{
	[Attribute("", UIWidgets.Object, "Map properties configuration")]
	ref array<ref SCR_MapPropsConfig> m_aMapPropConfigs;
};

//------------------------------------------------------------------------------------------------
//! Layer config root
[BaseContainerProps(configRoot: true)]
class SCR_MapLayersBase
{
	[Attribute("", UIWidgets.Object, "Layers")]
	ref array<ref SCR_LayerConfiguration> m_aLayers;
};

//------------------------------------------------------------------------------------------------
//! Descriptor visibility config root
[BaseContainerProps(configRoot: true)]
class SCR_MapDescriptorVisibilityBase
{
	[Attribute("", UIWidgets.Object, "Descriptor layers", "")]
	ref array<ref SCR_DescriptorViewLayer> m_aDescriptorViewLayers;
};