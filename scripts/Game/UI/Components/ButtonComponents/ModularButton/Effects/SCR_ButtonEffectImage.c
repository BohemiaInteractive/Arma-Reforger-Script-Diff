//! This effect modifies texture of an image.
[BaseContainerProps(configRoot : true), SCR_ButtonEffectTitleAttribute("Image", "m_sWidgetName")]
class SCR_ButtonEffectImage : SCR_ButtonEffectWidgetBase
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Common image set", "imageset")]
	ResourceName m_ImageSet;
	
	[Attribute()]
	ref SCR_ButtonEffectImage_ImageOrImageSet m_Default;
	
	[Attribute()]
	ref SCR_ButtonEffectImage_ImageOrImageSet m_Hovered;
	
	[Attribute()]
	ref SCR_ButtonEffectImage_ImageOrImageSet m_Activated;
	
	[Attribute()]
	ref SCR_ButtonEffectImage_ImageOrImageSet m_ActivatedHovered;
	
	[Attribute()]
	ref SCR_ButtonEffectImage_ImageOrImageSet m_Disabled;
	
	[Attribute()]
	ref SCR_ButtonEffectImage_ImageOrImageSet m_DisabledActivated;
	
	[Attribute()]
	ref SCR_ButtonEffectImage_ImageOrImageSet m_FocusGained;
	
	[Attribute()]
	ref SCR_ButtonEffectImage_ImageOrImageSet m_FocusLost;
	
	[Attribute()]
	ref SCR_ButtonEffectImage_ImageOrImageSet m_ToggledOn;
	
	[Attribute()]
	ref SCR_ButtonEffectImage_ImageOrImageSet m_ToggledOff;
	
	//-------------------------------------------------------------------------------------------
	override void OnStateDefault(bool instant)
	{
		Apply(m_Default);
	}
	
	//-------------------------------------------------------------------------------------------
	override void OnStateHovered(bool instant)
	{
		Apply(m_Hovered);
	}
	
	//-------------------------------------------------------------------------------------------
	override void OnStateActivated(bool instant)
	{
		Apply(m_Activated);
	}
	
	//-------------------------------------------------------------------------------------------
	override void OnStateActivatedHovered(bool instant)
	{
		Apply(m_ActivatedHovered);
	}
	
	//-------------------------------------------------------------------------------------------
	override void OnStateDisabled(bool instant)
	{
		Apply(m_Disabled);
	}
	
	//-------------------------------------------------------------------------------------------
	override void OnStateDisabledActivated(bool instant)
	{
		Apply(m_DisabledActivated);
	}
	
	//-------------------------------------------------------------------------------------------
	override void OnFocusGained(bool instant)
	{
		Apply(m_FocusGained);
	}	
	
	//-------------------------------------------------------------------------------------------
	override void OnFocusLost(bool instant)
	{
		Apply(m_FocusLost);
	}
	
	//-------------------------------------------------------------------------------------------
	override void OnToggledOn(bool instant)
	{
		Apply(m_ToggledOn);
	}
	
	//-------------------------------------------------------------------------------------------
	override void OnToggledOff(bool instant)
	{
		Apply(m_ToggledOff);
	}

	//-------------------------------------------------------------------------------------------
	protected void Apply(SCR_ButtonEffectImage_ImageOrImageSet imgSpec)
	{
		ImageWidget w = ImageWidget.Cast(m_wTarget);
		
		if (!w || !imgSpec)
			return;
		
		if (!imgSpec.m_ImageName.IsEmpty() && !imgSpec.m_ImageOrImageSet.IsEmpty())
			w.LoadImageFromSet(0, imgSpec.m_ImageOrImageSet, imgSpec.m_ImageName);
		else if (!imgSpec.m_ImageName.IsEmpty() && !m_ImageSet.IsEmpty())
			w.LoadImageFromSet(0, m_ImageSet, imgSpec.m_ImageName);
		else if (!imgSpec.m_ImageName.IsEmpty())
			w.LoadImageTexture(0, imgSpec.m_ImageOrImageSet);
	}
}

[BaseContainerProps()]
class SCR_ButtonEffectImage_ImageOrImageSet
{
	[Attribute()]
	ResourceName m_ImageOrImageSet;
	
	[Attribute()]
	string m_ImageName;
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] resource
	//! \param[in] imageName
	void SCR_ButtonEffectImage_ImageOrImageSet(ResourceName resource = string.Empty, string imageName = string.Empty)
	{
		if (!resource.IsEmpty())
			m_ImageOrImageSet = resource;
		
		if (!imageName.IsEmpty())
			m_ImageName = imageName;
	}
}
