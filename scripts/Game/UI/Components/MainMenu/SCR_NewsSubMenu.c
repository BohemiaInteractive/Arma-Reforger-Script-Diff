[BaseContainerProps()]
class SCR_NewsEntry
{
	NewsFeedItem m_Item;
	SCR_NewsTileComponent m_Tile;
	Widget m_wRoot;
	bool m_bRead;
	
	void SCR_NewsEntry(NewsFeedItem item)
	{
		m_Item = item;
	}
	
	void SetTile(SCR_NewsTileComponent tile, Widget w)
	{
		m_Tile = tile;
		m_wRoot = w;
	}
}

class SCR_NewsSubMenu : SCR_SubMenuBase
{
	[Attribute("{02155A85F2DC521F}UI/layouts/Menus/PlayMenu/PlayMenuTile.layout", UIWidgets.ResourceNamePicker, "", "layout")]
	protected ResourceName m_Layout;

	protected static ref array<ref SCR_NewsEntry> m_aEntries = {};

	protected SCR_GalleryComponent m_Gallery;
	protected SCR_SimpleMessageComponent m_SimpleMessage;
	protected bool m_bEntriesDisplayed;
	
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		m_Gallery = SCR_GalleryComponent.GetGalleryComponent("Gallery", m_wRoot);
		
		Widget simpleMessageRoot = m_wRoot.FindAnyWidget("SimpleMessage");
		if (simpleMessageRoot)
			m_SimpleMessage = SCR_SimpleMessageComponent.Cast(simpleMessageRoot.FindHandler(SCR_SimpleMessageComponent));
		
		SCR_ServicesStatusHelper.RefreshPing();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Insert(OnCommStatusCheckFinished);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();
		
		if (m_SimpleMessage && SCR_ServicesStatusHelper.GetLastReceivedCommStatus() != SCR_ECommStatus.FINISHED && m_aEntries.IsEmpty())
			m_SimpleMessage.SetVisible(true);
		else
			SetNewsEntries();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();
		
		m_bEntriesDisplayed = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabRemove()
	{
		super.OnTabRemove();
		SCR_ServicesStatusHelper.GetOnCommStatusCheckFinished().Remove(OnCommStatusCheckFinished);
		
		m_bEntriesDisplayed = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetNewsEntries()
	{
		MainMenuUI.GetNewsEntries(m_aEntries);
		
		if (m_aEntries.IsEmpty())
		{
			ShowErrorDialog();
			return;
		}
		
		if (m_bEntriesDisplayed)
			return;
		
		if (m_SimpleMessage)
			m_SimpleMessage.SetVisible(false);
		
		m_Gallery.ClearAll();
		
		foreach (SCR_NewsEntry entry : m_aEntries)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(m_Layout);
			if (!w)
				continue;

			SCR_NewsTileComponent tile = SCR_NewsTileComponent.Cast(w.FindHandler(SCR_NewsTileComponent));
			if (!tile)
				continue;

			entry.SetTile(tile, tile.m_wRoot);
			tile.m_OnRead.Insert(OnRead);
			tile.ShowTile(entry);

			m_Gallery.AddItem(w);
		}
		
		m_bEntriesDisplayed = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRead(SCR_NewsTileComponent comp)
	{
		string url = comp.m_Entry.m_Item.URL();
		if (url.IsEmpty())
			return;
		
		GetGame().GetPlatformService().OpenBrowser(url);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCommStatusCheckFinished(SCR_ECommStatus status, float responseTime, float lastSuccessTime, float lastFailTime)
	{	
		if (!m_bShown)
			return;

		if (status == SCR_ECommStatus.FINISHED)
			SetNewsEntries();
		else
			ShowErrorDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowErrorDialog()
	{
		if (!m_bShown)
			return;
		
		SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateTimeoutOkDialog();
		if (dialog)
			dialog.m_OnConfirm.Insert(OnErrorDialogClose);
		
		m_bEntriesDisplayed = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnErrorDialogClose()
	{
		if (!m_bShown)
			return;
		
		RequestTabChange(SCR_EProfileSuperMenuTabId.COMMUNITY);
	}
}