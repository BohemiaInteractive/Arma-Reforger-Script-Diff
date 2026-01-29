//------------------------------------------------------------------------------------------------
class SCR_ButtonImageComponent : SCR_ButtonBaseComponent 
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Select edds texture or imageset", "edds imageset")]
	ResourceName m_sTexture;
	
	[Attribute()]
	string m_sImageName;
	
	ImageWidget m_wImage;
	
	ref ScriptInvoker<Widget> m_OnMouseEnter = new ScriptInvoker();
	ref ScriptInvoker<Widget> m_OnMouseLeave = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wImage = ImageWidget.Cast(w.FindAnyWidget("Image"));
		SetImage(m_sTexture, m_sImageName);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);

		m_OnMouseEnter.Invoke(w);
	
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
		
		m_OnMouseLeave.Invoke(w);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	ImageWidget GetImageWidget()
	{
		return m_wImage;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetImage(ResourceName texture, bool fromLocalStorage = false)
	{
		if (!m_wImage)
			return;
		
		bool show = texture != string.Empty;
		m_wImage.SetVisible(show);
		if (show)
		{
			int x, y;
			m_wImage.LoadImageTexture(0, texture, false, fromLocalStorage);
			m_wImage.GetImageSize(0, x, y);
			m_wImage.SetSize(x, y);
		}
		
		m_sTexture = texture;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetImageSaturation(float saturation)
	{
		m_wImage.SetSaturation(saturation);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetImageSaturation()
	{
		return m_wImage.GetSaturation();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetImage(ResourceName imageSet, string imageName)
	{
		if (!m_wImage)
			return;
		
		bool show = imageSet != string.Empty;
		m_wImage.SetVisible(show);
		
		if (!show)
			return;
		
		if (imageSet.EndsWith("imageset"))
			m_wImage.LoadImageFromSet(0, imageSet, imageName);
		else
			m_wImage.LoadImageTexture(0, imageSet);

		// Resize
		int x, y;
		m_wImage.GetImageSize(0, x, y);
		m_wImage.SetSize(x, y);
		
		m_sTexture = imageSet;
		m_sImageName = imageName;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetImage(out string imageSet)
	{
		if (m_sImageName == string.Empty)
			return m_sTexture;
		imageSet = m_sTexture;
		return m_sImageName;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ButtonImageComponent GetButtonImage(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_ButtonImageComponent.Cast(
			SCR_WLibComponentBase.GetComponent(SCR_ButtonImageComponent, name, parent, searchAllChildren)
		);
		return comp;
	}
};