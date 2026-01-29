[BaseContainerProps(configRoot: true)]
class SCR_MilitarySymbolConfig
{
	[Attribute()]
	protected ref array<ref SCR_MilitarySymbolLayout> m_aLayouts;
	
	/*!
	Create widgets for given military symbol.
	\param symbol Military symbol
	\param parentWidget Parent widget, must be OverlaySlot
	*/
	void CreateWidgets(notnull SCR_MilitarySymbol symbol, notnull Widget parentWidget)
	{
		foreach (SCR_MilitarySymbolLayout layout: m_aLayouts)
		{
			layout.CreateLayout(symbol, parentWidget);
		}
	}
};
[BaseContainerProps(), SCR_BaseContainerMilitaryLayout()]
class SCR_MilitarySymbolLayout
{
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolIdentity))]
	protected EMilitarySymbolIdentity m_Identity;
	
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolDimension))]
	protected ref array<ref EMilitarySymbolDimension> m_aDimensions;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_Layout;
	
	[Attribute()]
	protected ref array<ref SCR_MilitarySymbolArea> m_aAreas;
	
	void CreateLayout(notnull SCR_MilitarySymbol symbol, notnull Widget parentWidget)
	{
		if (m_Identity != symbol.GetIdentity() || !m_aDimensions.Contains(symbol.GetDimension()))
			return;
		
		if (!m_Layout)
		{
			Print(string.Format("Layout not defined for %1!", typename.EnumToString(EMilitarySymbolIdentity, m_Identity)), LogLevel.WARNING);
			return;
		}
		
		Widget widget = GetGame().GetWorkspace().CreateWidgets(m_Layout, parentWidget);
		
		foreach (SCR_MilitarySymbolArea area: m_aAreas)
		{
			area.CreateArea(symbol, widget);
		}
	}
};
[BaseContainerProps(), BaseContainerCustomTitleField("m_sAreaWidgetName")]
class SCR_MilitarySymbolArea
{
	[Attribute()]
	protected string m_sAreaWidgetName;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	ResourceName m_Layout;

	void CreateArea(notnull SCR_MilitarySymbol symbol, notnull Widget parentWidget);
};
[BaseContainerProps(), BaseContainerCustomTitleField("m_sAreaWidgetName")]
class SCR_MilitarySymbolAreaTextures: SCR_MilitarySymbolArea
{
	[Attribute()]
	protected ref array<ref SCR_MilitarySymbol_TextureBase> m_aTextures;
	
	override void CreateArea(notnull SCR_MilitarySymbol symbol, notnull Widget parentWidget)
	{
		if (!m_Layout)
		{
			Print("Layout not defined!", LogLevel.WARNING);
			return;
		}
		
		if (m_sAreaWidgetName)
			parentWidget = parentWidget.FindAnyWidget(m_sAreaWidgetName);
		
		ImageWidget imageWidget;
		foreach (SCR_MilitarySymbol_TextureBase texture: m_aTextures)
		{
			if (!texture.IsCompatible(symbol))
				continue;
			
			imageWidget = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidgets(m_Layout, parentWidget));
			if (!imageWidget)
				continue;
			
			imageWidget.LoadImageTexture(0, texture.GetTexture());
		}
	}
};
[BaseContainerProps(), BaseContainerCustomTitleField("m_sAreaWidgetName")]
class SCR_MilitarySymbolAreaImageSet: SCR_MilitarySymbolArea
{
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "imageset")]
	protected ResourceName m_ImageSet;
	
	[Attribute("%1")]
	protected string m_sTextureSetFormat;
	
	[Attribute()]
	protected ref array<ref SCR_MilitarySymbol_QuadBase> m_aQuads;
	
	override void CreateArea(notnull SCR_MilitarySymbol symbol, notnull Widget parentWidget)
	{
		if (!m_Layout)
		{
			Print("Layout not defined!", LogLevel.WARNING);
			return;
		}
		
		if (m_sAreaWidgetName)
			parentWidget = parentWidget.FindAnyWidget(m_sAreaWidgetName);
		
		//~ Todo: Hotfix to keep non-Combatants icons clear without any other icons
		if (symbol.HasIcon(EMilitarySymbolIcon.UNARMED))
			return;
		
		ImageWidget imageWidget;
		string textureSetName;
		
		foreach (SCR_MilitarySymbol_QuadBase quad: m_aQuads)
		{
			if (!quad.IsCompatible(symbol))
				continue;
			
			imageWidget = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidgets(m_Layout, parentWidget));
			if (!imageWidget)
				continue;
			
			textureSetName = string.Format(m_sTextureSetFormat, quad.GetImageSetName());
			imageWidget.LoadImageFromSet(0, m_ImageSet, textureSetName);
		}
	}
};
[BaseContainerProps(), BaseContainerCustomTitleField("m_sAreaWidgetName")]
class SCR_MilitarySymbolAreaConflict: SCR_MilitarySymbolArea
{

	[Attribute("%1")]
	protected string m_sTextureSetFormat;
	
	[Attribute()]
	protected ref array<ref SCR_MilitarySymbol_QuadBase> m_aQuads;
	
	override void CreateArea(notnull SCR_MilitarySymbol symbol, notnull Widget parentWidget)
	{
		if (!m_Layout)
		{
			Print("Layout not defined!", LogLevel.WARNING);
			return;
		}
		
		if (m_sAreaWidgetName)
			parentWidget = parentWidget.FindAnyWidget(m_sAreaWidgetName);
		
		ImageWidget imageWidget;
		string textureSetName;
		foreach (SCR_MilitarySymbol_QuadBase quad: m_aQuads)
		{
			if (!quad.IsCompatible(symbol))
				continue;

			imageWidget = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidgets(m_Layout, parentWidget));
			if (!imageWidget)
				continue;
			textureSetName = string.Format(m_sTextureSetFormat, quad.GetImageSetName());

			imageWidget.LoadImageTexture(0, quad.GetImageSetName());
		}
	}
};
[BaseContainerProps()]
class SCR_MilitarySymbol_QuadBase
{
	[Attribute()]
	protected string m_sTextureSetName;
	
	string GetImageSetName()
	{
		return m_sTextureSetName;
	}
	
	bool IsCompatible(notnull SCR_MilitarySymbol symbol);
};
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EMilitarySymbolIcon, "m_Icon")]
class SCR_MilitarySymbol_QuadIcon: SCR_MilitarySymbol_QuadBase
{
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolIcon))]
	protected EMilitarySymbolIcon m_Icon;
	
	override bool IsCompatible(notnull SCR_MilitarySymbol symbol)
	{
		return symbol.HasIcon(m_Icon);
	}
};
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EMilitarySymbolAmplifier, "m_Amplifier")]
class SCR_MilitarySymbol_QuadAmplifier: SCR_MilitarySymbol_QuadBase
{
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolAmplifier))]
	protected EMilitarySymbolAmplifier m_Amplifier;
	
	override bool IsCompatible(notnull SCR_MilitarySymbol symbol)
	{
		return m_Amplifier == symbol.GetAmplifier();
	}
};
[BaseContainerProps()]
class SCR_MilitarySymbol_TextureBase
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "imageset")]
	protected ResourceName m_Texture;
	
	ResourceName GetTexture()
	{
		return m_Texture;
	}
	
	bool IsCompatible(notnull SCR_MilitarySymbol symbol);
};
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EMilitarySymbolIcon, "m_Icon")]
class SCR_MilitarySymbol_TextureIcon: SCR_MilitarySymbol_TextureBase
{
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolIcon))]
	protected EMilitarySymbolIcon m_Icon;
	
	override bool IsCompatible(notnull SCR_MilitarySymbol symbol)
	{
		return symbol.HasIcon(m_Icon);
	}
};
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EMilitarySymbolAmplifier, "m_Amplifier")]
class SCR_MilitarySymbol_TextureAmplifier: SCR_MilitarySymbol_TextureBase
{
	[Attribute("0", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EMilitarySymbolAmplifier))]
	protected EMilitarySymbolAmplifier m_Amplifier;
	
	override bool IsCompatible(notnull SCR_MilitarySymbol symbol)
	{
		return m_Amplifier == symbol.GetAmplifier();
	}
};

class SCR_BaseContainerMilitaryLayout: BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		EMilitarySymbolIdentity identity;
		source.Get("m_Identity", identity);
		title = typename.EnumToString(EMilitarySymbolIdentity, identity);
		
		array<EMilitarySymbolDimension> dimensions = {};
		source.Get("m_aDimensions", dimensions);
		int count = dimensions.Count();
		if (count > 0)
		{
			title += ": ";
			for (int i = 0; i < count; i++)
			{
				if (i > 0)
					title += " & ";
				
				title += typename.EnumToString(EMilitarySymbolDimension, dimensions[i]);
			}
		}
		
		return true;
	}
};