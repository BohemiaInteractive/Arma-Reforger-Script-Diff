//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
//! Config for default values set per descriptor type
class SCR_MapDescriptorDefaults
{
	[Attribute("128", UIWidgets.EditBox, desc: "Size of icons in imageset, it is important for internal size calculations!", params: "16 256")]
	int m_iImagesetIconSize;
	
	[Attribute("1", UIWidgets.EditBox, desc: "Sets the reference PixPerUnit for setting sizes which by default is = 1 \n", params: "0.01 10")]
	float m_fReferencePixPerUnit;
	
	[Attribute("", UIWidgets.Object, "Faction color default settings", "")]
	ref array<ref SCR_FactionColorDefaults> m_aFactionColors;
	
	[Attribute("", UIWidgets.Object, "Descriptor default settings", "")]
	ref array<ref SCR_DescriptorDefaultsBase> m_aDescriptorDefaults;	
		
	//------------------------------------------------------------------------------------------------
	void SCR_MapDescriptorDefaults()
	{
		SCR_DescriptorDefaultsBase.SetGlobals(m_iImagesetIconSize, m_fReferencePixPerUnit);
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of descriptor defaults
[BaseContainerProps(), SCR_FactionViewTitle()]
class SCR_FactionColorDefaults
{
	[Attribute("0", UIWidgets.ComboBox, "Faction", "", ParamEnumArray.FromEnum(EFactionMapID))]
	int m_iFaction;
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, desc: "Icon color")]
	ref Color m_vColorIcon; 
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, desc: "Text color")]
	ref Color m_vColorText; 
				
	//------------------------------------------------------------------------------------------------
	//! Set default color values
	void SetColors(MapDescriptorProps props)
	{
		props.SetFrontColor(m_vColorIcon);
		props.SetTextColor(m_vColorText);
	}
};

//------------------------------------------------------------------------------------------------
//! Configuration of descriptor defaults
[BaseContainerProps(), SCR_DescriptorViewTitle()]
class SCR_DescriptorDefaultsBase
{
	[Attribute("0", UIWidgets.ComboBox, "Descriptor type", "", ParamEnumArray.FromEnum(EMapDescriptorType))]
	int m_iDescriptorType;
	
	[Attribute("0", UIWidgets.EditBox, desc: "Priority in displaying the Descriptor", "0 inf 1")]
	int m_iPriority;
	
	[Attribute("0", UIWidgets.EditBox, desc: "imageset Index, determines to which imageset should be referred for its image", "0 100 1")]
	int m_iImageSetIndex;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Faction based colorization")]
	bool m_bUseFactionColors;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "This descriptor is configured but not visible without being activated by something else")]
	bool m_bIsDefaultInvisible;

	[Attribute("0", UIWidgets.ComboBox, "Descriptor group type", "", ParamEnumArray.FromEnum(EMapDescriptorGroup))]
	int m_iDescriptorGroupType;

	[Attribute("1.0", UIWidgets.EditBox, desc: "Group scale factor", "0.1 10.0 0.01")]
	float m_fGroupScale;
		
	static int s_iImageSetIconSize;		// real icon size in imageset
	static float s_fReferencePPU;		// refernce PixelPerUnit value for setting size
	
	//------------------------------------------------------------------------------------------------
	//! Set global descriptor defaults values
	static void SetGlobals(int size, float refPPU)
	{ 
		s_iImageSetIconSize = size; 
		s_fReferencePPU = refPPU;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set default values per descriptor type
	void SetDefaults(MapDescriptorProps props)
	{
		props.SetGroupType(m_iDescriptorGroupType);
		props.SetGroupScale(m_fGroupScale);
		props.SetPriority(m_iPriority);
		
		if (m_bIsDefaultInvisible)
			props.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set default colors per descriptor type
	void SetColors(MapDescriptorProps props)
	{}
};

//------------------------------------------------------------------------------------------------
//! Descriptor invisible type, for cursor interaction logic support while UI is handled by another system
[BaseContainerProps(), SCR_DescriptorViewTitle()]
class SCR_DescriptorDefInvisible : SCR_DescriptorDefaultsBase
{	
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(MapDescriptorProps props)
	{
		super.SetDefaults(props);

		props.SetIconVisible(false);
		props.SetTextVisible(false);
	}
};

//------------------------------------------------------------------------------------------------
//! Descriptor icon type defaults
[BaseContainerProps(), SCR_DescriptorViewTitle()]
class SCR_DescriptorDefIcon : SCR_DescriptorDefaultsBase
{		
	[Attribute("", UIWidgets.EditBox, desc: "imageset quad")]
	string m_sImageQuad;
	
	[Attribute("32", UIWidgets.EditBox, desc: "icon size in pixels when pixel per unit ratio = 1")]
	int m_iIconSize;
	
	[Attribute("0.75", UIWidgets.EditBox, desc: "icon minimum scale, multiplier of the size", params: "0.01 1")]
	float m_fIconMinScale;
	
	[Attribute("1.5", UIWidgets.EditBox, desc: "icon maximum scale, multiplier of the size", params: "1 100")]
	float m_fIconMaxScale;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Align icon with parent entity")]
	bool m_bAlignWithParent;

	[Attribute("1 1 1 1", UIWidgets.ColorPicker, desc: "Front color")]
	ref Color m_vColor; 
		
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(MapDescriptorProps props)
	{
		super.SetDefaults(props);

		float scale = (m_iIconSize / s_iImageSetIconSize) / s_fReferencePPU; // unscaled vals			
		float minScale = m_fIconMinScale * scale;
		float maxScale = m_fIconMaxScale * scale;
		
		props.SetImageDef(m_sImageQuad);
		props.SetIconSize(scale,minScale,maxScale);
		props.SetAlignWithParent(m_bAlignWithParent);
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetColors(MapDescriptorProps props)
	{
		props.SetFrontColor(m_vColor);
	}
};

//------------------------------------------------------------------------------------------------
//! Descriptor icon with text type defaults
[BaseContainerProps(), SCR_DescriptorViewTitle()]
class SCR_DescriptorDefIconText : SCR_DescriptorDefIcon
{		
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Specific font that will be used for this descriptor", params: "fnt")]
	ResourceName m_TextFont;
	
	[Attribute("25", UIWidgets.EditBox, desc: "text size", "1 1000 1")]
	float m_fTextSize;
		
	[Attribute("0.75", UIWidgets.EditBox, desc: "text minimum scale, multiplier of the size", params: "0.01 1")]
	float m_fTextMinScale;
	
	[Attribute("1.5", UIWidgets.EditBox, desc: "text maximum scale, multiplier of the size", params: "1 100")]
	float m_fTextMaxScale;
	
	[Attribute("0", UIWidgets.CheckBox, desc: "Use bold text")]
	bool m_bBoldText;
	
	[Attribute("0", UIWidgets.CheckBox, desc: "Use italic text")]
	bool m_bItalicText;
	
	[Attribute("0", UIWidgets.EditBox, desc: "text percentage X offset based on length", "-2.0 2.0 0.1")]
	float m_fTextOffsetX;
	
	[Attribute("0", UIWidgets.EditBox, desc: "text percentage Y offset based on height", "-2.0 2.0 0.1")]
	float m_fTextOffsetY;
	
	[Attribute("-1", UIWidgets.EditBox, desc: "text percentage horizontal alignment with icon", "-2.0 2.0 0.1")]
	float m_fIconTextAlignH;
	
	[Attribute("0.5", UIWidgets.EditBox, desc: "text percentage vertical alignment with icon", "-2.0 2.0 0.1")]
	float m_fIconTextAlignV;
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, desc: "Text color")]
	ref Color m_vTextColor; 
	
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(MapDescriptorProps props)
	{
		super.SetDefaults(props);
		
		props.SetFont(m_TextFont);
		props.SetTextSize(m_fTextSize, m_fTextMinScale * m_fTextSize, m_fTextMaxScale * m_fTextSize);
		props.SetTextOffsetX(m_fTextOffsetX);
		props.SetTextOffsetY(m_fTextOffsetY);
		props.SetIconTextAlignH(m_fIconTextAlignH);
		props.SetIconTextAlignV(m_fIconTextAlignV);
		
		if (m_bBoldText)
			props.SetTextBold();
		
		if (m_bItalicText)
			props.SetTextItalic();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetColors(MapDescriptorProps props)
	{
		super.SetColors(props);
		
		props.SetTextColor(m_vTextColor);
	}
};

//------------------------------------------------------------------------------------------------
//! Descriptor text type defaults
[BaseContainerProps(), SCR_DescriptorViewTitle()]
class SCR_DescriptorDefText : SCR_DescriptorDefaultsBase
{		
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Specific font that will be used for this descriptor", params: "fnt")]
	ResourceName m_TextFont;
	
	
	[Attribute("25", UIWidgets.EditBox, desc: "text size", "1 1000 1")]
	float m_fTextSize;
	
	[Attribute("0.75", UIWidgets.EditBox, desc: "text minimum scale, multiplier of the size", params: "0.01 1")]
	float m_fTextMinScale;
	
	[Attribute("1.5", UIWidgets.EditBox, desc: "text maximum scale, multiplier of the size", params: "1 100")]
	float m_fTextMaxScale;

	[Attribute("0", UIWidgets.CheckBox, desc: "Use bold text")]
	bool m_bBoldText;
	
	[Attribute("0", UIWidgets.CheckBox, desc: "Use italic text")]
	bool m_bItalicText;

	[Attribute("0", UIWidgets.EditBox, desc: "text rotation angle", "-359.0 359.0 0.0")]
	float m_fTextAngle;

	[Attribute("0", UIWidgets.EditBox, desc: "text percentage X offset based on length", "-2.0 2.0 0.1")]
	float m_fTextOffsetX;
	
	[Attribute("0", UIWidgets.EditBox, desc: "text percentage Y offset based on height", "-2.0 2.0 0.1")]
	float m_fTextOffsetY;
	
	[Attribute("-1", UIWidgets.EditBox, desc: "text percentage horizontal alignment with icon", "-2.0 2.0 0.1")]
	float m_fIconTextAlignH;
	
	[Attribute("0.5", UIWidgets.EditBox, desc: "text percentage vertical alignment with icon", "-2.0 2.0 0.1")]
	float m_fIconTextAlignV;
	
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, desc: "Text color")]
	ref Color m_vColor; 
		
	//------------------------------------------------------------------------------------------------
	override void SetDefaults(MapDescriptorProps props)
	{
		super.SetDefaults(props);

		props.SetFont(m_TextFont);
		props.SetTextSize(m_fTextSize, m_fTextMinScale * m_fTextSize, m_fTextMaxScale * m_fTextSize);		
		props.SetTextOffsetX(m_fTextOffsetX);
		props.SetTextOffsetY(m_fTextOffsetY);
		props.SetIconTextAlignH(m_fIconTextAlignH);
		props.SetIconTextAlignV(m_fIconTextAlignV);
		
		if (m_bBoldText)
			props.SetTextBold();
		
		if (m_bItalicText)
			props.SetTextItalic();

		props.SetTextAngle(m_fTextAngle);

		props.SetIconVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetColors(MapDescriptorProps props)
	{		
		props.SetTextColor(m_vColor);
	}
};

//------------------------------------------------------------------------------------------------
//! Custom names for defined zones to make config setup easier
class SCR_FactionViewTitle: BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{		
		int type;
		source.Get("m_iFaction", type);
		title = typename.EnumToString(EFactionMapID, type);
				
		return true;
	}
};