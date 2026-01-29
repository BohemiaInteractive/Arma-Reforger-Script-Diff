//------------------------------------------------------------------------------------------------
class SCR_ImageGalleryComponent : ScriptedWidgetComponent
{
	[Attribute("", UIWidgets.ResourceAssignArray, "")]
	protected ref array<ref ResourceName> m_aResourceNames;
	
	protected ref array<ref SCR_ImageGalleryButtonComponent> m_aImages = new array <ref SCR_ImageGalleryButtonComponent>;
	
	private Widget m_wPagingSpinbox;
	private SCR_SpinBoxPagingComponent m_wPagingSpinboxComponent;
	
	
	//[Attribute(defvalue: "", desc: "When the texture is an image set, define the quad name here")]
	//private string m_sImageName;
	
	protected ref array<Widget> m_aElementWidgets = new array<Widget>();
	protected SCR_ImageGalleryButtonComponent m_SelectedItem;
	
	//protected ScrollLayoutWidget m_wScrollLayout;
	
	Widget m_wRoot;
	
	Widget m_wGalleryGrid;
	
	protected int m_iSelectedItemIndex;
	protected ImageWidget m_MainImage;
	
	protected WorkspaceWidget m_Workspace;
	protected bool m_bItemSelected = false;
	
	protected const ResourceName LAYOUT_GALLERY_ELEMENT = "{F7C52551E0B04BE1}UI/layouts/WidgetLibrary/ImageWidgets/WLib_ImageGalleryItem.layout";
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wRoot = w;
		Init();
	}
	
	//------------------------------------------------------------------------------------------------
	void Init()
	{
		if (!m_wRoot)
			return;
		
		m_wGalleryGrid = m_wRoot.FindAnyWidget("GalleryGrid");
		m_MainImage = ImageWidget.Cast(m_wRoot.FindAnyWidget("MainImage"));
		m_wPagingSpinbox = m_wRoot.FindAnyWidget("PagingSpinbox");
		if (m_wPagingSpinbox)
		{
			m_wPagingSpinboxComponent = SCR_SpinBoxPagingComponent.Cast(m_wPagingSpinbox.FindHandler(SCR_SpinBoxPagingComponent));
			
			if (m_wPagingSpinboxComponent)
				m_wPagingSpinboxComponent.m_OnChanged.Insert(Paging);
		}
		m_Workspace = GetGame().GetWorkspace();
		
		if (!m_Workspace)
			return;
		
		if (!m_aResourceNames)
			return;
		

		SetPagingLayout(0, m_aResourceNames.Count());
		
		/*
		foreach (ResourceName content : m_aResourceNames)
		{
			// Create tab button
			Widget button = m_Workspace.CreateWidgets(LAYOUT_GALLERY_ELEMENT, m_wGalleryGrid);
			
			if (!button)
				continue;
			
			SCR_ImageGalleryButtonComponent galleryButtonComponent = SCR_ImageGalleryButtonComponent.Cast(button.FindHandler(SCR_ImageGalleryButtonComponent));
			
			if (!galleryButtonComponent)
				continue;
			
			galleryButtonComponent.SetImage(content);
			galleryButtonComponent.m_OnClicked.Insert(OnImageSelected);
			HorizontalLayoutSlot.SetPadding(button, 4, 4, 4, 4);
			m_aImages.Insert(galleryButtonComponent);
			if (!m_aImages.IsEmpty() && !m_bItemSelected)
				OnImageSelected(m_aImages[0]);

		}
		*/
		
	
	}
	
	//------------------------------------------------------------------------------------------------
	int AddItem(string item)
	{
		int i = -1;
		if (!m_aResourceNames)
			m_aResourceNames = new array<ref ResourceName>;
		
		i = m_aResourceNames.InsertAt(item, 0);
		
		
		SetPagingLayout(0, m_aResourceNames.Count());
		return i;
	}
	
		//------------------------------------------------------------------------------------------------
	void RemoveItem(int item)
	{
		if (!m_aResourceNames || item < 0 || item > m_aResourceNames.Count())
			return;

		//m_aElementWidgets[item].RemoveFromHierarchy();
		m_aResourceNames.Remove(item);
		SetPagingLayout(0, m_aResourceNames.Count());
		//m_iSelectedItemIndex = -1;
		
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentIndex()
	{
		return m_iSelectedItemIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetCurrentItem()
	{
		if (!m_aResourceNames || m_iSelectedItemIndex < 0 || m_iSelectedItemIndex >= m_aResourceNames.Count())
			return string.Empty;
		
		return m_aResourceNames[m_iSelectedItemIndex];
	}
	
	//------------------------------------------------------------------------------------------------
	void OnImageSelected(string item)
	{
			m_MainImage.LoadImageTexture(0,item, false, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPagingLayout(int index, int maxIndex)
	{
		
		m_wPagingSpinboxComponent.SetCurrentItem(index);
		m_wPagingSpinboxComponent.SetPageCount(maxIndex);
		if (m_aResourceNames && !m_aResourceNames.IsEmpty())
			OnImageSelected(m_aResourceNames[0]);
	}
	
	//------------------------------------------------------------------------------------------------
	void Paging()
	{
		if (!m_aResourceNames.IsEmpty())
	 	OnImageSelected(m_aResourceNames[m_wPagingSpinboxComponent.GetCurrentIndex()]);
	}
	
	void SetCanNavigate(bool canNavigate)
	{
		m_wPagingSpinboxComponent.SetCanNavigate(canNavigate);
	}
	
	
	
	
};