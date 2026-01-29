[BaseContainerProps()]
class UIInfoAssetCard : SCR_UIInfo
{
	[Attribute("0.0 0.0 0.0 1.0", uiwidget: UIWidgets.ColorPicker)]
	private ref Color m_TextColor;
	
	[Attribute(params: "edds imageset")]
	private ResourceName m_TitleBackground;
	
	[Attribute(params: "edds imageset")]
	private ResourceName m_Image;
	
	[Attribute("1.0 1.0 1.0 1.0", uiwidget: UIWidgets.ColorPicker)]
	private ref Color m_FactionColor;
	
	[Attribute(params: "edds imageset")]
	private ResourceName m_IconBackground;
	
	[Attribute()]
	private bool m_bModded;
	
	[Attribute(params: "edds imageset")]
	private ResourceName m_ModsetIcon;
	
	[Attribute(params: "edds imageset")]
	private ResourceName m_ModsetFrame;
	
	[Attribute(params: "edds imageset")]
	private ResourceName m_ModsetLabel;
	
	[Attribute(params: "edds imageset")]
	private ResourceName m_CardBackground;
	
	[Attribute("1", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EAssetTrait))]
	private ref array<EAssetTrait> m_aTraits;

	//------------------------------------------------------------------------------------------------
	//! \return the color variable at which the title text is supposed to be set
	Color GetTextColor()
	{
		return Color.FromInt(m_TextColor.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	//! \return the path to the background of the title
	ResourceName GetTitleBackgroundPath()
	{
		return m_TitleBackground;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the path to the entity texture
	ResourceName GetImagePath()
	{
		return m_Image;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the faction color of the entity
	Color GetFactionColor()
	{
		return Color.FromInt(m_FactionColor.PackToInt());
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the path to the icon background texture
	ResourceName GetIconBackgroundPath()
	{
		return m_IconBackground;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if an entity is modded
	bool GetModded()
	{
		return m_bModded;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the path to the modset icon texture
	ResourceName GetModsetIcon()
	{
		return m_ModsetIcon;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the path to the modset frame texture
	ResourceName GetModsetFrame()
	{
		return m_ModsetFrame;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the path to the modset label texture
	ResourceName GetModsetLabel()
	{
		return m_ModsetLabel;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return the path to the card background texture
	ResourceName GetCardBackground()
	{
		return m_CardBackground;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return an array of traits linked to the entity.
	int GetTraits(out notnull array<EAssetTrait> outTraits)
	{
		if (!m_aTraits) return 0;
		return outTraits.Copy(m_aTraits);
	}
}

enum EAssetTrait 
{
	ANTI_TANK,
	ANTI_AIR,
	MEDICAL,
	REFUELING,
	TRANSPORT,
	FAST
}
