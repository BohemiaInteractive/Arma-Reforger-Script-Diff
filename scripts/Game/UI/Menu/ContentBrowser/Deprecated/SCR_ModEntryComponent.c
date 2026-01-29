//------------------------------------------------------------------------------------------------
class SCR_ModEntryComponent : SCR_ContentEntryComponent
{		
	const string WIDGET_MOD_RATING = "hRating";
	const string WIDGET_MOD_SIZE = "hDataSize";
	
	// Widgets 
	protected TextWidget m_wTxtRating;
	protected TextWidget m_wTxtDataSize;
	
	// Mod handling 
	protected ref SCR_WorkshopItem m_Item; // Strong ref!
	ref ScriptInvoker m_OnModDonwloaded = new ScriptInvoker();
	ref Revision m_Version;
	
	// Handling button 
	protected SCR_MultipleStatesButtonComponent m_BtnDownload; 
	
	// Other
	protected bool m_bHover = false;
	protected bool m_bFocus = false;
	
	// Constants
	const int STATE_DOWNLOAD = 0;
	const int STATE_DOWNLOADING = 1;
	const int STATE_CANCEL = 2;
	const int STATE_DOWNLOADED = 3;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Mini texts widgets 
		Widget wRating = w.FindAnyWidget(WIDGET_MOD_RATING);
		if (wRating)
			m_wTxtRating = TextWidget.Cast(wRating.FindAnyWidget("Text"));
		
		Widget wSize = w.FindAnyWidget(WIDGET_MOD_SIZE);
		if (wSize)
			m_wTxtDataSize = TextWidget.Cast(wSize.FindAnyWidget("Text"));
		
		// Handling setup 
		Widget btnDownloadWidget = w.FindAnyWidget(WIDGET_BTN_DOWNLOAD);
		if (btnDownloadWidget)
		{
			m_BtnDownload = SCR_MultipleStatesButtonComponent.Cast(btnDownloadWidget.FindHandler(SCR_MultipleStatesButtonComponent));
			if (m_BtnDownload)
			{
				m_BtnDownload.m_OnClicked.Insert(OnDownloadClick);
				m_BtnDownload.m_OnHover.Insert(OnDownloadButtonHover);
				m_BtnDownload.m_OnHoverLeave.Insert(OnDownloadButtonHoverLeave);
			}
		}
		
		this.UpdateAllWidgets();
		//GetGame().GetCallqueue().CallLater(UpdateEachFrame, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		//GetGame().GetCallqueue().Remove(UpdateEachFrame);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		//super.OnFocus(w, x, y);
		//m_OnFocus.Invoke(m_wRoot, m_Dependency);
		
		//m_bFocus = true;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		
		m_bFocus = false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateAllWidgets()
	{
		if (!m_Item)
			return;
		
		// Set data from the workshop item, if it's fetched
		
		if (m_Item.GetRequestFailed())
		{
			SetLabelText("[Failed to load data]");
			return;
		}
		
		// Texts and image
		/*
		SetDescriptionText(m_Item.GetSummary());
		bool imageLoaded, backendHasImage;
		ImageScale image;
		m_Item.GetPreviewImage(imageLoaded, backendHasImage, image);
		if (imageLoaded)
		{
			SetThumbnail(image.Path());
		}
		*/
		
		// Rating 
		int rating = m_Item.GetAverageRating() * 100;
		SetModRating(rating.ToString() + "%");
		
		// Set name
		SetLabelText(m_Item.GetName());
		
		// Size
		string sizeStr = SCR_ByteFormat.GetReadableSize(m_Item.GetSizeBytes());
		this.SetModDataSize(sizeStr);
		
		
		this.UpdateDownloadButtonState();
	}
	
	//------------------------------------------------------------------------------------------------
	/*
	void UpdateEachFrame()
	{
		this.Update();
		GetGame().GetCallqueue().CallLater(UpdateEachFrame, 1);
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	void UpdateDownloadButtonState()
	{
		if (!m_Item)
		{
			return;
		}
			
		// Update the state of the main button
		
		SCR_WorkshopItem item = m_Item;
		bool downloading = false;
		bool paused = false;
		bool downloaded = false;
		Revision targetRevision;
		float progress = -1.0;
		int newState = -1;
		
		item.GetDownloadState(downloading, paused, progress, targetRevision);
		downloaded = item.GetOffline();
		
		// Set state of the button
		/*
		if (m_bHover)
		{
			// Hovering the cursor
			if (downloading)
				newState = STATE_CANCEL;
			else
			{
				if (!downloaded)
					newState = STATE_DOWNLOAD;
				else
					newState = STATE_DOWNLOADED;
			}
		}
		else
		{
			
			if (downloaded)
			{
				newState = STATE_DOWNLOADED;
			}
			else
			{
				// NOT hovering the cursor
				if (downloading)
					newState = STATE_DOWNLOADING;
				else
					newState = STATE_DOWNLOAD;
			}
		}
		*/
		
		
		
		// On consoles we can't hover, so hovering is irrelevant
		if (downloaded)
		{
			newState = STATE_DOWNLOADED;
		}
		else
		{
			// NOT hovering the cursor
			if (downloading)
				newState = STATE_CANCEL;
			else
				newState = STATE_DOWNLOAD;
		}
		
		if (m_BtnDownload.GetSelectedItem() != newState)
		{
			m_BtnDownload.ChangeState(newState);
		}
		
		// Set progress
		if (progress != -1.0 && downloading)
		{
			m_BtnDownload.StartProgress();
			m_BtnDownload.SetProgress(progress * 100.0);
		}
		else
		{
			m_BtnDownload.FinishProgress();
		}
		
		// Show or hide the hint
		m_BtnDownload.SetHintVisible(m_BtnDownload.GetSelectedItem() != STATE_DOWNLOADED && m_bFocus);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Do this actions on using button on  
	void OnDownloadClick(SCR_ButtonBaseComponent button)
	{
		int state = m_BtnDownload.GetSelectedItem();
		
		switch (state)
		{
			// Ready to download 
			case STATE_DOWNLOAD:
			DownloadContent();
			break;
			
			// Is downloading 
			case STATE_DOWNLOADING:
			m_Item.CancelDownload();
			break;
			
			// Is downloading and hovered 
			case STATE_CANCEL:
			m_Item.CancelDownload();
			break;
			
			// Is up to date 
			case STATE_DOWNLOADED:
			break;
		}
		
		this.UpdateDownloadButtonState();
	}
	
	
	//------------------------------------------------------------------------------------------------
	void SetModContent(SCR_WorkshopItem item, string version)
	{		
		m_Item = item;
		
		if (item)
			m_Item.m_OnChanged.Insert(Callback_OnChanged);
		
		this.UpdateAllWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetModRating(string str) { SetTextSafe(m_wTxtRating, str); }
	
	//------------------------------------------------------------------------------------------------
	void SetModDataSize(string str) { SetTextSafe(m_wTxtDataSize, str); }
	
	//------------------------------------------------------------------------------------------------
	void DownloadContent()
	{
		if (!m_Item)
			return;
		 
		// Dowload
		SCR_WorkshopItemActionDownload action;
		if (m_Version == null)
		{
			// No speficic version, just get the latest one
			action = m_Item.DownloadLatestVersion();
		}
		else
		{
			// Download a specific version
			action = m_Item.Download(m_Version);
		}
		action.Activate();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDownloadButtonHover()
	{
		m_bHover = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDownloadButtonHoverLeave()
	{
		m_bHover = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this on download  addon dat 
	protected void OnItemDownload()
	{
		Print("on item download");
		this.UpdateDownloadButtonState();
	}
	
	//------------------------------------------------------------------------------------------------
	void MarkAsUpdated()
	{
		m_BtnDownload.SetProgress(0);
		UpdateDownloadButtonState();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetWorkshopItem() {return m_Item; }
	
	//------------------------------------------------------------------------------------------------
	bool IsUpdated(WorkshopItem item = null)
	{	
		return m_Item.GetOffline() && !m_Item.GetUpdateAvailable();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsDownloading(WorkshopItem item = null)
	{
		/*
		if (!item)
			item = m_Dependency.GetCachedItem();
		int flags = item.GetStateFlags();
		
		if (flags & EWorkshopItemState.EWSTATE_DOWNLOADING)
		{
			return true;
		}
		*/
		
		return false;
	}
	
	// TODO remove this, server browser must be switched use SCR_WorkshopItem
	Dependency GetDependency() { return null; }
	
	protected void Callback_OnChanged()
	{
		UpdateAllWidgets();
	}
};