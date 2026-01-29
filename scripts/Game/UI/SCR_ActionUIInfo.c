class SCR_ActionUIInfo : UIInfo
{
	[Attribute(UIConstants.ICONS_IMAGE_SET, UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_sInteractionIcon;
	
	[Attribute(UIConstants.ICONS_GLOW_IMAGE_SET, UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_sInteractionGlowIcon;

	[Attribute(desc: "If 'Icon' is an ImageSet, define the name of the wanted Icon here.")]
	protected string m_sIconName;

	//------------------------------------------------------------------------------------------------
	//! Set icon to given image widgets.
	//! \param[in] ImageWidget Target image widget
	//! \param[in] ImageWidget Target glow widget. Can be null
	//! \return True when the image was set
	bool SetIconTo(ImageWidget imageWidget, ImageWidget glowWidget = null)
	{
		if (!imageWidget)
			return false;

		bool interactionIconEmpty = m_sInteractionIcon.IsEmpty();
		if (interactionIconEmpty || (!interactionIconEmpty && m_sIconName.IsEmpty()))
			return false;

		string ext;
		FilePath.StripExtension(m_sInteractionIcon, ext);
		if (ext == "imageset")
		{
			imageWidget.LoadImageFromSet(0, m_sInteractionIcon, m_sIconName);

			if (glowWidget)
				glowWidget.LoadImageFromSet(0, m_sInteractionGlowIcon, m_sIconName);
		}
		else
		{
			imageWidget.LoadImageTexture(0, m_sInteractionIcon);
			
			if (glowWidget)
				glowWidget.LoadImageTexture(0, m_sInteractionGlowIcon);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Set the ImageSet to be used for the Image widget.
	//! \param[in] ResourceName imageset
	void SetImageSet(ResourceName imageset)
	{
		m_sInteractionIcon = imageset;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the icon name if defined
	//! \return string IconName
	string GetIconName()
	{
		return m_sIconName;
	}
}
