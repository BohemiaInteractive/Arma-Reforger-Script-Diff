class SCR_SaveImageGalleryPickerComponent : SCR_ChangeableComponentBase
{
	[Attribute("PickerThumbnail", desc: "Name if widget used as thumbnail image picker")]
	protected string m_sThumbnailName;
	
	[Attribute("Gallery", desc: "Name of widget holding list of image pickers used for image gallery")]
	protected string m_sGalleryListName;
	
	[Attribute("Image", desc: "Name of widget for displaying picked")]
	protected string m_sImageName;
	
	/*
	protected SCR_ImagePickerComponent m_ThumbnailPicker;
	protected ref array<SCR_ImagePickerComponent> m_aGalleryPickers = {};
	*/
	
	protected ImageWidget m_ThumbnailImage;
	protected ref array<ImageWidget> m_aGalleryPickers = {};
	
	//-------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Thumbnail
		Widget thumbnailPicker = w.FindAnyWidget(m_sThumbnailName);
		if (thumbnailPicker)
		{
			//m_ThumbnailPicker = SCR_ImagePickerComponent.Cast(thumbnailPicker.FindHandler(SCR_ImagePickerComponent));
			m_ThumbnailImage = ImageWidget.Cast(thumbnailPicker.FindAnyWidget("Image"));
		}
		
		// Gallery
		Widget galleryList = w.FindAnyWidget(m_sGalleryListName);
		if (galleryList)
		{
			array<ref Widget> pickers = {};
			SCR_WidgetHelper.GetAllChildren(galleryList, pickers);
			
			foreach (Widget picker : pickers)
			{
				/*
				SCR_ImagePickerComponent pickerComponent = SCR_ImagePickerComponent.Cast(picker.FindHandler(SCR_ImagePickerComponent));
				if (!pickerComponent)
				{
					Print("Widget does not contains image picker!", LogLevel.WARNING);
					continue;
				}
				
				m_aGalleryPickers.Insert(pickerComponent);
				*/
			} 
		}
	}
	
	//-------------------------------------------------------------------------------------------
	void SetThumbnailImage(ResourceName image)
	{
		m_ThumbnailImage.SetVisible(!image.IsEmpty());
		
		if (!image.IsEmpty())
			m_ThumbnailImage.LoadImageTexture(0, image);
	}
	
	//-------------------------------------------------------------------------------------------
	void SetGalleryImages(array<ResourceName> images)
	{
		
	}
	
	/*
	//-------------------------------------------------------------------------------------------
	SCR_ImagePickerComponent GetThumbnailPicker()
	{
		return m_ThumbnailPicker;
	}
	*/
	
	/*
	//-------------------------------------------------------------------------------------------
	ResourceName GetThumbnailSourceImage()
	{
		if (!m_ThumbnailPicker)
			return "";
		
		return m_ThumbnailPicker.GetImageResource();
	}
	*/
	
	/*
	//-------------------------------------------------------------------------------------------
	// Returns array of all resource names used for image
	array<string> GetGalleryResources()
	{
		array<string> images = {};
		foreach (SCR_ImagePickerComponent picker : m_aGalleryPickers)
		{
			string res = picker.GetImageResource();
			
			if (!res.IsEmpty())
				images.Insert(picker.GetImageResource());
		}
		
		return images;
	}
	*/
}