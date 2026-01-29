// Handles setting the icon and glow, assumes to find two ImageWidgets named Icon and Glow
// Allows future changes to the hierarchy without losing overrides in children 

//------------------------------------------------------------------------------------------------
class SCR_DynamicIconComponent : SCR_ScriptedWidgetComponent 
{
	[Attribute(UIConstants.ICONS_IMAGE_SET, UIWidgets.ResourcePickerThumbnail, "Select edds texture or imageset", "edds imageset")]
	ResourceName m_sIconImageset;
	
	[Attribute(UIConstants.ICONS_GLOW_IMAGE_SET, UIWidgets.ResourcePickerThumbnail, "Select edds texture or imageset", "edds imageset")]
	ResourceName m_sGlowImageset;
	
	[Attribute("play")]
	string m_sImageName;
	
	[Attribute("32 32 0")]
	vector m_sImageSize;
	
	ImageWidget m_wIcon;
	ImageWidget m_wGlow;
	SizeLayoutWidget m_wSize;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget("Icon"));
		m_wGlow = ImageWidget.Cast(w.FindAnyWidget("Glow"));
		m_wSize = SizeLayoutWidget.Cast(w.FindAnyWidget("Size"));
		
		SetImage(m_sImageName);
	}

	//------------------------------------------------------------------------------------------------
	ImageWidget GetIconWidget()
	{
		return m_wIcon;
	}
	
	//------------------------------------------------------------------------------------------------
	ImageWidget GetGlowWidget()
	{
		return m_wGlow;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetImage(string imageName, ResourceName iconImageSet = string.Empty, ResourceName glowImageSet = string.Empty)
	{
		if (!m_wIcon || !m_wGlow)
			return;
		
		if (iconImageSet.IsEmpty())
			iconImageSet = m_sIconImageset;
		
		SetImage_Internal(m_wIcon, iconImageSet, imageName);

		if (glowImageSet.IsEmpty())
			glowImageSet = m_sGlowImageset;
		
		SetImage_Internal(m_wGlow, glowImageSet, imageName);
		
		m_sIconImageset = iconImageSet;
		m_sGlowImageset = glowImageSet;
		m_sImageName = imageName;
		
		if (m_wSize)
		{
			m_wSize.SetWidthOverride(m_sImageSize[0]);
			m_wSize.SetHeightOverride(m_sImageSize[1]);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetImage_Internal(ImageWidget image, ResourceName imageSet, string imageName)
	{
		if (imageSet.EndsWith("imageset"))
			image.LoadImageFromSet(0, imageSet, imageName);
		else
			image.LoadImageTexture(0, imageSet);
		
		image.SetSize(m_sImageSize[0], m_sImageSize[1]);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIconColor(Color color)
	{
		if (!color || !m_wIcon)
			return;
			
		m_wIcon.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGlowColor(Color color)
	{
		if (!color || !m_wIcon)
			return;
			
		m_wGlow.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVisible(bool visible)
	{
		if (m_wIcon)
			m_wIcon.SetVisible(visible);

		if (m_wGlow)
			m_wGlow.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns name and imageset
	string GetIcon(out string imageSet)
	{
		if (m_sImageName == string.Empty)
			return m_sIconImageset;
		imageSet = m_sIconImageset;
		return m_sImageName;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_DynamicIconComponent FindComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		SCR_DynamicIconComponent comp = SCR_DynamicIconComponent.Cast(
			SCR_WLibComponentBase.GetComponent(SCR_DynamicIconComponent, name, parent, searchAllChildren)
		);
		return comp;
	}
}