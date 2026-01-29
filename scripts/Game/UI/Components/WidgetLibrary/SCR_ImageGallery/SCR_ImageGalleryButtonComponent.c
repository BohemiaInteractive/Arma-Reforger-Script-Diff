
class SCR_ImageGalleryButtonComponent : SCR_WLibComponentBase 
{
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Image for the gallery", "edds imageset")]
	private ResourceName m_Texture;
	
	ref ScriptInvoker m_OnClicked = new ScriptInvoker();
	Widget m_wSizeContent;
	ImageWidget m_Picture;
	ImageWidget m_SelectedBar;
	string m_sPictureString = string.Empty;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_Picture = ImageWidget.Cast(m_wRoot.FindAnyWidget("Picture"));
		m_SelectedBar = ImageWidget.Cast(m_wRoot.FindAnyWidget("SelectedBar"));
		m_wSizeContent = m_wRoot.FindAnyWidget("TileOverlay");
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		if (button != 0)
			return false;
		
		m_OnClicked.Invoke(this);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetImage(ResourceName name)
	{
		m_Texture = name;
		m_Picture.LoadImageTexture(0, m_Texture.GetPath());
		//m_Picture.SetImageTexture()
	}
	
	//------------------------------------------------------------------------------------------------
	string GetImage()
	{
		
		return m_Texture;
	}

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Magic numbers
	
	//------------------------------------------------------------------------------------------------
	void Select()
	{
		//m_SelectedBar.SetVisible(true);
		m_wRoot.SetColor(Color.FromInt(Color.WHITE));
		float padding[4] = {4, 4, 4, 4};
		AnimateWidget.Padding(m_wSizeContent, padding, 10);
		//m_wRoot.SetZOrder(1000);
	}
	
	//------------------------------------------------------------------------------------------------
	void Deselect()
	{
		//m_SelectedBar.SetVisible(false);
		
		float padding[4] = {0, 0, 0, 0};
		AnimateWidget.Padding(m_wSizeContent, padding, 10);
		//m_wRoot.SetZOrder(1000);
	}
	
//---- REFACTOR NOTE END ----
};