/*
Dialog which shows big image gallery over whole screen.
*/

class SCR_ContentBrowser_GalleryDialog : ChimeraMenuBase
{
	protected ref SCR_ContentBrowser_GalleryDialogWidgets m_Widgets = new SCR_ContentBrowser_GalleryDialogWidgets;
	
	protected ref array<BackendImage> m_aImages = {};
	
	protected int m_iSelectedItem;
	
	//----------------------------------------------------------------------------------
	static SCR_ContentBrowser_GalleryDialog CreateForImages(array<BackendImage> images, int selectedImageIndex)
	{
		MenuBase menuBase = GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.GalleryDialog);
		SCR_ContentBrowser_GalleryDialog galleryDialog = SCR_ContentBrowser_GalleryDialog.Cast(menuBase);
		galleryDialog.SetImages(images, selectedImageIndex);
		return galleryDialog;
	}
	
	//----------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		m_Widgets.Init(GetRootWidget());
		
		m_Widgets.m_BackButtonComponent.m_OnActivated.Insert(OnBackButton);
		m_Widgets.m_NextButtonComponent.m_OnClicked.Insert(OnNextButton);
		m_Widgets.m_PrevButtonComponent.m_OnClicked.Insert(OnPrevButton);
		
		// Close this dialog when user clicks outside
		SCR_ModularButtonComponent backgroundButtonComponent = SCR_ModularButtonComponent.FindComponent(GetRootWidget());
		backgroundButtonComponent.m_OnClicked.Insert(OnBackButton);
		
		// Also close this when user clicks on cross at top-right
		m_Widgets.m_CloseButtonComponent.m_OnClicked.Insert(OnBackButton);
		
		// Listen to inputs
		if (!SCR_Global.IsEditMode())
		{
			GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_RIGHT, EActionTrigger.DOWN, OnNextButton);
			GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnPrevButton);
		}
		
		GetGame().GetWorkspace().SetFocusedWidget(GetRootWidget());
	}
	
	
	//----------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		if (!SCR_Global.IsEditMode())
		{
			GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_RIGHT, EActionTrigger.DOWN, OnNextButton);
			GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnPrevButton);
		}
	}
	
	//----------------------------------------------------------------------------------
	void SetImages(array<BackendImage> images, int selectedImageIndex)
	{
		m_Widgets.m_SpinBoxComponent0.ClearAll();
		m_aImages.Clear();
		
		if (images.IsEmpty())
			return;
		
		m_aImages = images;
		int total = m_aImages.Count();
		foreach (int i, BackendImage img : m_aImages)
		{
			m_Widgets.m_SpinBoxComponent0.AddItem(string.Empty, i == total - 1);	
		}
		
		m_iSelectedItem = selectedImageIndex;
		ShowImage(selectedImageIndex);
	}
	
	//----------------------------------------------------------------------------------
	protected void ShowImage(int id)
	{
		if (m_aImages.IsEmpty())
			return;
		
		if (id < 0 || id >= m_aImages.Count())
			 id = m_aImages.Count() - 1;
		
		BackendImage backendImage = m_aImages[id];

		m_Widgets.m_BackendImageComponent.SetImage(backendImage);
		m_Widgets.m_SpinBoxComponent0.SetCurrentItem(id);
		
		UpdateButtons();
	}
	
	//----------------------------------------------------------------------------------
	protected void UpdateButtons()
	{
		m_Widgets.m_NextButtonComponent.SetEnabled(m_iSelectedItem < (m_aImages.Count() - 1));
		m_Widgets.m_PrevButtonComponent.SetEnabled(m_iSelectedItem > 0);
	}

	//------------------------------------------------------------------------------------------
	//! Offset - how much to move current item ID right/left
	protected void OffsetCurrentItem(int offset)
	{
		m_iSelectedItem = m_iSelectedItem + offset;
		m_iSelectedItem = Math.ClampInt(m_iSelectedItem, 0, m_aImages.Count() - 1);
		ShowImage(m_iSelectedItem);
	}
		
	//----------------------------------------------------------------------------------
	protected void OnBackButton()
	{
		Close();
	}
	
	//------------------------------------------------------------------------------------------
	protected void OnNextButton()
	{
		OffsetCurrentItem(1);
	}
	
	//------------------------------------------------------------------------------------------
	protected void OnPrevButton()
	{
		OffsetCurrentItem(-1);
	}
}