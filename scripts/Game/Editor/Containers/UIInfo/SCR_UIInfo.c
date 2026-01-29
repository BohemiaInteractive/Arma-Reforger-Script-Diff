//! Class used to hold UI information.
//! Data are intentionally *READ ONLY*, because the class is often used on prefabs, not instances.
//! Instead of adding SetXXX() functions here, consider using specialized inherited class.
//! Inspired by engine-driven UIInfo, but not related to it.
//! \see UIInfo
[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_UIInfo : SCR_UIDescription
{
	[Attribute(params: "edds imageset", uiwidget: UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName Icon;

	[Attribute(desc: "When 'Icon' is an image set, this defines name of the image in the set.")]
	protected string IconSetName;

	//------------------------------------------------------------------------------------------------
	//! Get icon.
	//! When using it to fill ImageWIdget, use SetIconTo() if possible.
	//! Especially important since the image can point to image set instead of plain texture!
	//! \return Image path
	ResourceName GetIconPath()
	{
		string ext;
		FilePath.StripExtension(Icon, ext);
		if (ext == "imageset")
		{
			Print(string.Format("Unable to return texture path, it's an image set '%1'! Use SCR_UIInfo.SetIconTo() instead.", Icon), LogLevel.WARNING);
			return ResourceName.Empty;
		}
		else
		{
			return Icon;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Image Set.
	//! When using it to fill ImageWIdget, use SetIconTo() if possible.
	//! Especially important since the image can point to image set instead of plain texture!
	//! \return Image path
	ResourceName GetImageSetPath()
	{
		string ext;
		FilePath.StripExtension(Icon, ext);
		if (ext == "imageset")
		{
			return Icon;
		}
		else
		{
			Print(string.Format("Unable to return ImageSet path, it's a Texture '%1'! Use SCR_UIInfo.SetIconTo() instead.", Icon), LogLevel.WARNING);
			return ResourceName.Empty;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns Icon Set name if imageset
	//! \return string empty if not a image set
	string GetIconSetName()
	{
		string ext;
		FilePath.StripExtension(Icon, ext);

		if (ext == "imageset")
			return IconSetName;
		else
			return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the info has an icon defined.
	//! \return True when the icon is defined
	bool HasIcon()
	{
		if (Icon.IsEmpty())
			return false;

		//Check if image set name is assigned
		string ext;
		FilePath.StripExtension(Icon, ext);
		if (ext == "imageset")
			return !GetIconSetName().IsEmpty();

		return true;
	}

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----	
// This class is meant to hold data, not manipulate widgets! Such methods should be in SCR_WidgetTools or SCR_WidgetHelper
	
	//------------------------------------------------------------------------------------------------
	//! Set icon to given image widget.
	//! Use this function instead of retrieving the texture using GetIconPath() and setting it manually!
	//! When the texture is an image set, manual setting would not work.
	//! \param imageWidget Target image widget
	//! \return True when the image was set
	bool SetIconTo(ImageWidget imageWidget)
	{
		if (!imageWidget || Icon.IsEmpty())
			return false;

		string ext;
		FilePath.StripExtension(Icon, ext);
		if (ext == "imageset")
			imageWidget.LoadImageFromSet(0, Icon, GetIconSetName());
		else
			imageWidget.LoadImageTexture(0, GetIconPath());

		return true;
	}

//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//!
	//! \param prefix
	//! \param logLevel
	override void Log(string prefix = string.Empty, LogLevel logLevel = LogLevel.VERBOSE)
	{
		Print(string.Format(prefix + "%1: \"%2\", \"%3\", \"%4\"", Type(), Name, Description, Icon), logLevel);
	}

	//------------------------------------------------------------------------------------------------
	//! Get UIInfo from a source object.
	//! \param source Source object, most commonly entity or component source
	//! \param varName Variable name of UIInfo
	//! \return UIInfo class
	static SCR_UIInfo GetInfo(BaseContainer source, string varName)
	{
		BaseContainer infoSource = source.GetObject(varName);
		if (infoSource)
			return SCR_UIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(infoSource));
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Create SCR_UIInfo from engine UIInfo.
	//! \param source Source info
	//! \return SCR_UIInfo class
	static SCR_UIInfo CreateInfo(UIInfo source)
	{
		if (!source)
			return null;

		SCR_UIInfo info = new SCR_UIInfo();
		info.Name = source.GetName();
		info.Description = source.GetDescription();
		info.Icon = source.GetIconPath();
		return info;
	}

	//------------------------------------------------------------------------------------------------
	//! Create SCR_UIInfo from basic params.
	//! \param name
	//! \param description
	//! \param icon
	//! \param iconSetName
	//! \return SCR_UIInfo class
	static SCR_UIInfo CreateInfo(LocalizedString name, LocalizedString description = LocalizedString.Empty, ResourceName icon = ResourceName.Empty, string iconSetName = string.Empty)
	{
		SCR_UIInfo info = new SCR_UIInfo();
		info.Name = name;
		info.Description = description;
		info.Icon = icon;
		info.IconSetName = iconSetName;
		return info;
	}

	//------------------------------------------------------------------------------------------------
	//! Get placeholder info
	//! \param type inherited from SCR_UIInfo
	//! \return UIInfo class
	static SCR_UIInfo CreatePlaceholderInfo(typename type)
	{
		SCR_UIInfo info = SCR_UIInfo.Cast(type.Spawn());
		info.Name = string.Format("ERROR: Missing info for %1", type);
		return info;
	}

	//------------------------------------------------------------------------------------------------
	//--- Protected, to be overridden and/or made public by inherited classes
	protected override void CopyFrom(SCR_UIName source)
	{
		SCR_UIInfo sourceInfo = SCR_UIInfo.Cast(source);
		if (sourceInfo)
		{
			Icon = sourceInfo.Icon;
			IconSetName = sourceInfo.IconSetName;
		}

		super.CopyFrom(source);
	}
}
