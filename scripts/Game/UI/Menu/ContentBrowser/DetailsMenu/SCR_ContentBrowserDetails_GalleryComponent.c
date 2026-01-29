/*
Component of a gallery in addon details menu.
*/

class SCR_ContentBrowserDetails_GalleryComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("", UIWidgets.Auto, "Names of tile widgets")]
	protected ref array<string> m_aTileWidgetNames;

	// Array with image preview widgets
	protected ref array<Widget> m_aTiles = {};
	protected ref array<SCR_BackendImageComponent> m_aTileComponents = {};
	protected ref array<BackendImage> m_aImages = {};
	protected SCR_ContentBrowser_GalleryDialog m_GalleryDialog;
	
	protected ref SCR_ContentBrowser_GalleryWidgets m_Widgets = new SCR_ContentBrowser_GalleryWidgets();
	
	protected int m_iCurrentItem;
	protected int m_iSelectedItem;
	
	protected const int UPDATE_DELAY = 250;
	
	//------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_Widgets.Init(w);

		// Find image gallery widgets
		foreach (string widgetName : m_aTileWidgetNames)
		{
			Widget wImageTile = w.FindAnyWidget(widgetName);
			if (!wImageTile)
				continue;
			
			m_aTiles.Insert(wImageTile);
			
			SCR_ModularButtonComponent comp = SCR_ModularButtonComponent.FindComponent(wImageTile);
			comp.m_OnClicked.Insert(OnTileClick);

			SCR_BackendImageComponent backendImgComp = SCR_BackendImageComponent.Cast(wImageTile.FindHandler(SCR_BackendImageComponent));
			m_aTileComponents.Insert(backendImgComp);
		}

		if (!SCR_Global.IsEditMode())
		{
			m_Widgets.m_SpinBoxComponent.ClearAll();
			
			GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_RIGHT, EActionTrigger.DOWN, OnNextAction);
			GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnPrevAction);
		}
		
		m_Widgets.m_NextButtonComponent.m_OnClicked.Insert(OnNextButton);
		m_Widgets.m_PrevButtonComponent.m_OnClicked.Insert(OnPrevButton);
		
		m_Widgets.m_SpinBoxComponent.SetCurrentItem(0);
		
		UpdateAllWidgets();
		
		GetGame().GetCallqueue().CallLater(Update, UPDATE_DELAY, true);
	}
	
	//------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (!SCR_Global.IsEditMode())
		{
			GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_RIGHT, EActionTrigger.DOWN, OnNextAction);
			GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnPrevAction);
		}
		
		GetGame().GetCallqueue().Remove(Update);
	}
	
	// --- Public ---
	//------------------------------------------------------------------------------------------
	void SetImages(array<BackendImage> images)
	{
		m_Widgets.m_SpinBoxComponent.ClearAll();
		m_aImages.Clear();
		m_aImages = images;
		
		int total = m_aImages.Count();
		foreach (int i, BackendImage img : m_aImages)
		{
			m_Widgets.m_SpinBoxComponent.AddItem(string.Empty, i == total - 1);
		}
			
		UpdateAllWidgets();
		
		if (m_GalleryDialog)
			m_GalleryDialog.SetImages(m_aImages, m_iSelectedItem);
	}
	
	// --- Protected ---
	//------------------------------------------------------------------------------------------
	protected void Update()
	{
		// Update current highlighted dot
		int tileId = m_aTiles.Find(GetGame().GetWorkspace().GetFocusedWidget());
		if (tileId != -1)
		{
			int spinBoxItemId = m_iCurrentItem + tileId;
			if (m_Widgets.m_SpinBoxComponent.GetCurrentIndex() != spinBoxItemId)
				m_Widgets.m_SpinBoxComponent.SetCurrentItem(spinBoxItemId);
		}
		else
		{
			int currentItem = m_Widgets.m_SpinBoxComponent.GetCurrentIndex();
			int spinBoxItemId = Math.ClampInt(currentItem, m_iCurrentItem, m_iCurrentItem + m_aTiles.Count() - 1);
			if (m_Widgets.m_SpinBoxComponent.GetCurrentIndex() != spinBoxItemId)
				m_Widgets.m_SpinBoxComponent.SetCurrentItem(spinBoxItemId);
		}
	}
	
	//------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		UpdateTiles();
		
		m_Widgets.m_NextButtonComponent.SetEnabled(m_iCurrentItem < (m_aImages.Count() - m_aTiles.Count()));
		m_Widgets.m_PrevButtonComponent.SetEnabled(m_iCurrentItem > 0);
	}
	
	//------------------------------------------------------------------------------------------
	//! Updates images on tiles according to currently selected item
	protected void UpdateTiles()
	{
		int iImage;
		int nImages = m_aImages.Count();
		
		foreach (int iTile, Widget tile : m_aTiles)
		{
			iImage = m_iCurrentItem + iTile;
			
			if (iImage < nImages && iImage >= 0)
				ShowImageOnTile(iTile, m_aImages[iImage]);
			else
				ShowImageOnTile(iTile, null);
		}
	}
	
	//------------------------------------------------------------------------------------------
	//! Shows image on one of preview image boxes
	//! Preview image box is of this layout:
	//! {F7F8BD8128A6CEAF}UI/layouts/Menus/ContentBrowser/DetailsMenu/Gallery/ContentBrowser_Gallery_ImageWithBackground.layout
	protected void ShowImageOnTile(int id, BackendImage backendImage)
	{
		SCR_BackendImageComponent comp = m_aTileComponents[id];
		if (!comp)
			return;
		
		Widget tile = m_aTiles[id];
		ImageWidget wImage = ImageWidget.Cast(tile.FindAnyWidget("Image"));
		
		// If we want to show no image, hide image widget, disable the tile so it can't be focused
		wImage.SetVisible(backendImage != null);
		tile.SetEnabled(backendImage != null);
		
		comp.SetImage(backendImage);
	}
	
	//------------------------------------------------------------------------------------------
	//! Offset - how much to move current item ID right/left
	protected void OffsetCurrentItem(int offset)
	{
		m_iCurrentItem = m_iCurrentItem + offset;
		int maxItemId = m_aImages.Count() - m_aTiles.Count();
		if (maxItemId < 0)
			maxItemId = 0;
		m_iCurrentItem = Math.ClampInt(m_iCurrentItem, 0, maxItemId);
		UpdateAllWidgets();
	}
	
	//------------------------------------------------------------------------------------------
	//! Called when a tile is clicked
	protected void OnTileClick(SCR_ModularButtonComponent comp)
	{
		// Bail if we have no images at all
		if (m_aImages.IsEmpty())
			return;
		
		// Find what image was selected in this tile
		int tileId = m_aTiles.Find(comp.GetRootWidget());
		if (tileId == -1)
			return;
		
		m_iSelectedItem = Math.Clamp(m_iCurrentItem + tileId, 0, m_aImages.Count() - 1);
		m_GalleryDialog = SCR_ContentBrowser_GalleryDialog.CreateForImages(m_aImages, m_iSelectedItem);
	}
	
	//------------------------------------------------------------------------------------------
	protected void OnNextAction()
	{
		if (GetGame().GetWorkspace().GetFocusedWidget() != m_aTiles[m_aTiles.Count()-1])
			return;
		
		OffsetCurrentItem(1);		
	}

	//------------------------------------------------------------------------------------------
	protected void OnPrevAction()
	{
		if (GetGame().GetWorkspace().GetFocusedWidget() != m_aTiles[0])
			return;
		
		OffsetCurrentItem(-1);		
	}

	//------------------------------------------------------------------------------------------
	protected void OnNextButton()
	{
		OffsetCurrentItem(1);
		GetGame().GetWorkspace().SetFocusedWidget(m_aTiles[m_aTiles.Count()-1]);
	}
	
	//------------------------------------------------------------------------------------------
	protected void OnPrevButton()
	{
		OffsetCurrentItem(-1);
		GetGame().GetWorkspace().SetFocusedWidget(m_aTiles[0]);
	}
	
	//------------------------------------------------------------------------------------------
	override bool OnUpdate(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return super.OnUpdate(w);
		
		if (w != m_wRoot)
			return false;
		
		GetGame().GetCallqueue().Call(UpdateSize);
		
		return super.OnUpdate(w);
	}
	
	//------------------------------------------------------------------------------------------
	protected void UpdateSize()
	{
		if (m_aTiles.IsEmpty())
			return;
		
		// Resize the height of all images, we must keep same aspect ratio
		float sizex, sizey;
		m_aTiles[0].GetScreenSize(sizex, sizey);
		float sizexUnscaled = GetGame().GetWorkspace().DPIUnscale(sizex);
		m_Widgets.m_ImagesHeightSize.EnableHeightOverride(true);
		m_Widgets.m_ImagesHeightSize.SetHeightOverride(sizexUnscaled / SCR_WorkshopUiCommon.IMAGE_SIZE_RATIO);
	}
}