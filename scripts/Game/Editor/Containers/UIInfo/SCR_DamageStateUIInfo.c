[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_DamageStateUIInfo : SCR_UIInfo
{	
	[Attribute("1.0 1.0 1.0 1.0", desc: "Main color of icon")]
	protected ref Color m_Color;
	
	[Attribute("1.0 1.0 1.0 1.0", desc: "Main color of background")]
	protected ref Color m_BackgroundColor;

	[Attribute("1.0 1.0 1.0 1.0", desc: "Main color of outline")]
	protected ref Color m_OutlineColor;

	[Attribute("1.0 1.0 1.0 1.0", desc: "Regeneration color of background")]
	protected ref Color m_BackgroundColorRegen;

	[Attribute("1.0 1.0 1.0 1.0", desc: "Regeneration color of outline")]
	protected ref Color m_OutlineColorRegen;

	[Attribute("{B9199157B90D6216}UI/Textures/InventoryIcons/Medical/Medical-icons.imageset", params: "edds, imageset", uiwidget: UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_sOutlineImage;

	[Attribute("Hitzone-outline_UI", desc: "Quad name of the outline image (if using an imageset")]
	protected string m_sOutlineQuadName;
	
	[Attribute("{B9199157B90D6216}UI/Textures/InventoryIcons/Medical/Medical-icons.imageset", params: "edds, imageset", uiwidget: UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_sBackgroundImage;
	
	[Attribute("Hitzone-BG_UI", desc: "Quad name of the background image (if using an imageset")]
	protected string m_sBackgroundQuadName;

	[Attribute("", desc: "Used for access of multiple icons from a single imageset")]
	protected ref array<string> m_aIconNames;

	//------------------------------------------------------------------------------------------------
	//! Get color of Icon and Outline
	//! \return icon/outline colour
	Color GetColor()
	{
		return Color.FromInt(m_Color.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	//! Get color of background
	//! \return background colour
	Color GetBackgroundColor()
	{
		return Color.FromInt(m_BackgroundColor.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	//! Get color of outline
	//! \return outline colour
	Color GetOutlineColor()
	{
		return Color.FromInt(m_OutlineColor.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	//! Get regeneration color of background
	//! \return background's regeneration colour
	Color GetBackgroundColorRegen()
	{
		return Color.FromInt(m_BackgroundColorRegen.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	//! Get regeneration color of outline
	//! \return
	Color GetOutlineColorRegen()
	{
		return Color.FromInt(m_OutlineColorRegen.PackToInt());
	}

	//------------------------------------------------------------------------------------------------
	//! Get ResourceName of outline image
	//! \return Outline image
	ResourceName GetOutlineImage()
	{
		return m_sOutlineImage;
	}

	//------------------------------------------------------------------------------------------------
	//! Get ResourceName of outline image
	//! \return Outline image
	ResourceName GetBackgroundImage()
	{
		return m_sBackgroundImage;
	}

	//------------------------------------------------------------------------------------------------
	//! Get name of outline image quad (if is from imageset)
	//! \return Outline image quad name
	string GetOutlineQuadName()
	{
		return m_sOutlineQuadName;
	}

	//------------------------------------------------------------------------------------------------
	//! Get name of background image quad (if is from imageset)
	//! \return Background image quad name
	string GetBackgroundQuadName()
	{
		return m_sBackgroundQuadName;
	}

	//------------------------------------------------------------------------------------------------
	//! Set image to given image widget.
	//! \param imageWidget Target image widget
	//! \param imageRes Source image resource
	//! \return True when the image was set
	bool SetImageTo(ImageWidget imageWidget, ResourceName imageRes, string quadName = string.Empty)
	{
		if (!imageWidget)
			return false;

		string ext;
		FilePath.StripExtension(imageRes, ext);
		if (ext == "imageset")
			return imageWidget.LoadImageFromSet(0, imageRes, quadName);
		else
			return imageWidget.LoadImageTexture(0, imageRes);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! SetIconTo() overload that accepts an index when m_aIconNames is not empty
	//! \param imageWidget Target image widget
	//! \param iconIndex Index of an icon from m_aIconNames array
	//! \return True when the image was set
	bool SetIconTo(ImageWidget imageWidget, int iconIndex)
	{
		if (!imageWidget || Icon.IsEmpty())
			return false;

		string ext;
		FilePath.StripExtension(Icon, ext);
		if (ext == "imageset")
			imageWidget.LoadImageFromSet(0, Icon, GetIconSetName(iconIndex));
		else
			imageWidget.LoadImageTexture(0, GetIconPath());

		return true;
	}	

	//------------------------------------------------------------------------------------------------
	//! GetIconSetName() overload specified by an array index
	//! \param iconIndex Index of an icon from m_aIconNames array
	//! \return
	string GetIconSetName(int iconIndex)
	{
		if (!m_aIconNames || !m_aIconNames.IsIndexValid(iconIndex))
			return GetIconSetName();

		return m_aIconNames[iconIndex];
	}
};
