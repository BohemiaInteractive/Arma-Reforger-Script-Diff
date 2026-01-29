/*
Main dialog of the download manager, which is typically opened from the download manager panel.

It lists all current and previous downloads.

!!! For the menu to work, it needs a SCR_DownloadManager_Entity to be placed in the world.
This is required because the menu can be opened after any of the downloads have been started,
but by that time we must store the download references somewhere.
*/
enum SCR_EDownloadManagerTabs
{
	ACTIVE = 0,
	HISTORY
}

// TODO: move tab specific stuff to a child of SCR_SuperMenuComponent
class SCR_DownloadManager_Dialog : SCR_TabDialog
{
	protected const string STATE_DOWNLOADING = "#AR-Workshop_TabName_Downloaded";
	protected const string STATE_ALL_PAUSE = "#AR-DownloadManager_State_AllDownloadsPaused";
	protected const string STATE_NO_ACTIVE_DOWNLOADS = "#AR-DownloadManager_State_NoActiveDownloads";

	protected const ResourceName DOWNLOAD_LINE_LAYOUT = "{FB196DBC0ABA6AE4}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManagerEntry.layout";
	protected const string DOWNLOAD_SUMMARY_FORMAT = "%1 / %2";
	protected const string DOWNLOAD_SUMMARY_COLORED_FORMAT = "<color rgba=%1>%2</color>";
	
	protected ref SCR_DownloadManagerDialogWidgets m_Widgets = new SCR_DownloadManagerDialogWidgets();
	
	// Tabs
	protected ref SCR_DownloadManagerListComponent m_ActiveDownloads;
	protected ref SCR_DownloadManagerListComponent m_HistoryDownloads;
	
	//-----------------------------------------------------------------------------------------------
	static SCR_DownloadManager_Dialog Create()
	{
		return SCR_CommonDialogs.CreateDownloadManagerDialog();
	}

	//------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{	
		super.OnMenuOpen(preset);
		
		// Init widgets
		m_Widgets.Init(GetRootWidget());
		GetGame().GetInputManager().AddActionListener("MenuDownloadManager", EActionTrigger.DOWN, OnBackButton);
		
		m_Widgets.m_Overlay.SetVisible(false);
		m_Widgets.m_DownloadSummaryText.SetVisible(false);
		m_Widgets.m_ProgressText.SetVisible(false);
		
		// Setup submenus 
		m_ActiveDownloads = SCR_DownloadManagerListComponent.Cast(m_SuperMenuComponent.GetSubMenu(SCR_EDownloadManagerTabs.ACTIVE));
		m_HistoryDownloads = SCR_DownloadManagerListComponent.Cast(m_SuperMenuComponent.GetSubMenu(SCR_EDownloadManagerTabs.HISTORY));
		
		m_HistoryDownloads.ShowPauseResumeAllButton(false);
		
		InitList();
		
		// Setup callback 
		SCR_DownloadManager downloadManager = SCR_DownloadManager.GetInstance();
		
		downloadManager.m_OnNewDownload.Insert(OnNewDownload);
		downloadManager.m_OnDownloadComplete.Insert(OnDownloadComplete);
		downloadManager.m_OnDownloadFailed.Insert(OnDownloadFailed);
		downloadManager.m_OnDownloadCanceled.Insert(OnDownloadCanceled);
	}
	
	//-----------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{	
		super.OnMenuClose();
		
		GetGame().GetInputManager().RemoveActionListener("MenuDownloadManager", EActionTrigger.DOWN, OnBackButton);
		
		// Clear all recent failed downloads
		SCR_DownloadManager.GetInstance().ClearFailedDownloads();
	}
	
	//-----------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (!mgr)
			return;
		int nCompleted, nTotal;
			mgr.GetDownloadQueueState(nCompleted, nTotal);
		
		SCR_WorkshopItemActionDownload dlAction = GetSelectedAction();
		bool pauseCancelEnabled;	// True when pause or cancel buttons should be enabled
		if (dlAction)
		{
			if (!dlAction.IsFailed() && !dlAction.IsCompleted() && !dlAction.IsCanceled())
				pauseCancelEnabled = true;
		}
		
		// Pause All / Resume All button label
		bool paused = mgr.GetDownloadsPaused();
		string pauseAllButtonLabel = "#AR-DownloadManager_ButtonPauseAll";
		if (paused)
			pauseAllButtonLabel = "#AR-DownloadManager_ButtonResumeAll";
		
		// Pause button
		string pauseButtonLabel = "#AR-DownloadManager_ButtonPause";
		if (dlAction)
		{
			if (dlAction.IsPaused() || dlAction.IsInactive())
				pauseButtonLabel = "#AR-DownloadManager_ButtonResume";
		}
		
		// Show downlaoding speed 
		UpdateProgressWidgets();
		UpdateDownloadingSpeedWidget(m_ActiveDownloads.HasContent(), nCompleted == nTotal);

		if (m_ActiveDownloads.GetShown())
			m_ActiveDownloads.ShowPauseResumeAllButton(nCompleted != nTotal);
		else
		{
			m_ActiveDownloads.ShowPauseResumeAllButton(false);
			m_HistoryDownloads.ShowPauseResumeAllButton(false);
		}
	}
	
	//------------------------------------------------------------------------------------------
	protected void UpdateProgressWidgets()
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		string downloadStateText;
		int nCompleted, nTotal;
		mgr.GetDownloadQueueState(nCompleted, nTotal);
		
		if (nTotal > 0 && !mgr.GetDownloadsPaused())
			downloadStateText = string.Format("%1 %2 / %3", WidgetManager.Translate(STATE_DOWNLOADING), nCompleted, nTotal);
		else if (nTotal > 0 && mgr.GetDownloadsPaused())
			downloadStateText = string.Format("%1 %2 / %3", WidgetManager.Translate(STATE_ALL_PAUSE), nCompleted, nTotal);
		else if (mgr.GetDownloadsPaused())
			downloadStateText = STATE_ALL_PAUSE;
		else
			downloadStateText = STATE_NO_ACTIVE_DOWNLOADS;
		
		m_Widgets.m_StateText.SetText(downloadStateText);
		
		m_Widgets.m_Overlay.SetVisible(m_ActiveDownloads.HasContent());
		m_Widgets.m_DownloadSummaryText.SetVisible(mgr.HasRunningDownloads() || nCompleted > 0);
		m_Widgets.m_ProgressText.SetVisible(nTotal > 0);
		
		// Progress bar, progress text
		if (nTotal > 0)
		{
			// Get total progress of all downloads
			array<ref SCR_WorkshopItemActionDownload> downloadQueue = mgr.GetDownloadQueue();
			float progress = SCR_DownloadManager.GetDownloadActionsProgress(downloadQueue);
			
			// Progress bar
			m_Widgets.m_ProgressBarComponent.SetValue(progress);
			
			// Progress percent text
			m_Widgets.m_ProgressText.SetText(UIConstants.FormatUnitPercentage(SCR_WorkshopUiCommon.GetDownloadProgressPercentage(progress)));
			
			// Display total size 
			float downloadTotalSize = SCR_DownloadManager.GetInstance().GetDownloadQueueSize();
			m_Widgets.m_DownloadSummaryText.SetVisible(downloadTotalSize > 0);
			
			if (downloadTotalSize > 0)
			{
				float downloadDoneSize =  SCR_DownloadManager.GetInstance().GetDownloadedSize();

				string downloadDoneSizeStr = SCR_ByteFormat.ContentDownloadFormat(downloadDoneSize);
				string downloadTotalSizeStr = SCR_ByteFormat.ContentDownloadFormat(downloadTotalSize);
				
				string coloredDone = WidgetManager.Translate(DOWNLOAD_SUMMARY_COLORED_FORMAT, UIColors.FormatColor(UIColors.CONTRAST_COLOR), downloadDoneSizeStr);
				string coloredTotal = WidgetManager.Translate(DOWNLOAD_SUMMARY_COLORED_FORMAT, UIColors.FormatColor(UIColors.CONTRAST_COLOR), downloadTotalSizeStr);
				
				m_Widgets.m_DownloadSummaryText.SetText(string.Format(DOWNLOAD_SUMMARY_FORMAT, coloredDone, coloredTotal));
			}
		}
		else
		{
			m_Widgets.m_ProgressBarComponent.SetValue(0);
		}
	}
	
	//------------------------------------------------------------------------------------------
	protected void UpdateDownloadingSpeedWidget(bool display, bool showFallback)
	{
		m_Widgets.m_DownloadSpeed.SetVisible(display);
		if (!display)
			return;
		
		if (showFallback)
		{
			m_Widgets.m_DownloadSpeed.SetText("0 KB/s");
			return;
		}
		
		string unit = " KB/s";
		float speed = 0;
		
		if (SCR_DownloadManager.GetInstance().HasRunningDownloads())
			speed = GetGame().GetRestApi().GetDownloadTrafficKBPerS();
		
		// Faster speed
		if (speed > SCR_ByteFormat.BYTE_UNIT_SIZE)
		{
			speed /= SCR_ByteFormat.BYTE_UNIT_SIZE;
			unit = " MB/s";
		}
			
		// Show text
		m_Widgets.m_DownloadSpeed.SetText(Math.Round(speed).ToString() + unit);
	}
	
	//-----------------------------------------------------------------------------------------------
	// Custom
	//-----------------------------------------------------------------------------------------------
	//-----------------------------------------------------------------------------------------------
	// Init list of downloads
	protected void InitList()
	{
		array<ref SCR_DownloadManager_Entry> downloads = {};
		SCR_DownloadManager.GetInstance().GetAllDownloads(downloads);
		
		array<ref SCR_WorkshopItemActionDownload> failedDownloads = SCR_DownloadManager.GetInstance().GetFailedDownloads();
		
		for (int i = 0, count = downloads.Count(); i < count; i++)
		{
			SCR_WorkshopItemActionDownload action = downloads[i].m_Action;
			
			if (SCR_DownloadManagerEntry.DownloadActionState(action) == EDownloadManagerActionState.RUNNING)
			{
				m_ActiveDownloads.AddEntry(DOWNLOAD_LINE_LAYOUT, action);
			}
			else
			{
				// Show recent fails in active
				if (failedDownloads.Find(action) != -1)
					m_ActiveDownloads.AddEntry(DOWNLOAD_LINE_LAYOUT, action);
				else
					m_HistoryDownloads.AddEntry(DOWNLOAD_LINE_LAYOUT, action);
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	protected SCR_WorkshopItemActionDownload GetSelectedAction()
	{
		Widget w = GetGame().GetWorkspace().GetFocusedWidget();
		
		if (!w)
			return null;
		
		SCR_DownloadManagerEntry entry  = SCR_DownloadManagerEntry.Cast(w.FindHandler(SCR_DownloadManagerEntry)); 
		if (!entry)
			return null;
		
		return entry.GetDownloadAction();
	}

	//------------------------------------------------------------------------------------------
	// Callbacks
	//------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------
	protected void OnNewDownload(SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	{
		// Remove same item in history 
		SCR_DownloadManagerEntry sameEntry = m_HistoryDownloads.EntryWithItem(item);
		if (sameEntry)
			m_HistoryDownloads.RemoveEntry(sameEntry);
		
		// Check same item in active
		sameEntry = m_ActiveDownloads.EntryWithItem(item);
		
		if (sameEntry)
		{
			m_ActiveDownloads.ChangeEntryCategory(sameEntry.GetRootWidget(), EDownloadManagerActionState.RUNNING);
			sameEntry.InitForDownloadAction(item, item.GetDownloadAction());
		}
		else
		{
			// Add action on activate in order to have target version
			action.m_OnActivated.Insert(OnNewActionActivate);
		}
	}
	
	//------------------------------------------------------------------------------------------
	protected void OnNewActionActivate(SCR_WorkshopItemActionDownload action)
	{
		m_ActiveDownloads.AddEntry(DOWNLOAD_LINE_LAYOUT, action);
		action.m_OnActivated.Remove(OnNewActionActivate);
	}
	
	//------------------------------------------------------------------------------------------
	protected void OnDownloadComplete(SCR_WorkshopItemActionDownload action)
	{
		// Move to downloaded 
		SCR_DownloadManagerEntry entry  = m_ActiveDownloads.DownloadActionLine(action);
		
		if (entry)
			m_ActiveDownloads.ChangeEntryCategory(entry.GetRootWidget(), EDownloadManagerActionState.DOWNLOADED);
	}
	
	//------------------------------------------------------------------------------------------
	//! Call when download are not running
	//! Move view to failed downlaods and show dialog with listed downloads
	protected void OnDownloadFailed(SCR_WorkshopItemActionDownload action, int reason)
	{
		array<ref SCR_DownloadManager_Entry> downloads = {};
		SCR_DownloadManager.GetInstance().GetAllDownloads(downloads);
		
		SCR_DownloadManagerEntry entry = m_ActiveDownloads.DownloadActionLine(action);
		
		if (entry)
			m_ActiveDownloads.ChangeEntryCategory(entry.GetRootWidget(), EDownloadManagerActionState.FAILED);
		
		// Open active and scroll up 
		m_SuperMenuComponent.GetTabView().ShowTab(SCR_EDownloadManagerTabs.ACTIVE, false);
	}
	
	//------------------------------------------------------------------------------------------
	protected void OnDownloadCanceled(SCR_WorkshopItemActionDownload action)
	{
		SCR_DownloadManagerEntry entry = m_ActiveDownloads.DownloadActionLine(action);
		
		if (entry)
			m_ActiveDownloads.ChangeEntryCategory(entry.GetRootWidget(), EDownloadManagerActionState.FAILED);
	}
	
	//-----------------------------------------------------------
	protected void OnBackButton()
	{
		m_OnCancel.Invoke();
		Close();
	}
	
	//-----------------------------------------------------------
	protected void OnPauseAllButton()
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		
		if (!mgr)
			return;
		
		bool paused = mgr.GetDownloadsPaused();
		mgr.SetDownloadsPaused(!paused);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnPauseButton()
	{
		SCR_WorkshopItemActionDownload action = GetSelectedAction();
		
		if (!action)
			return;
		
		if (action.IsPaused())
			action.Resume();
		else if (action.IsInactive())
			action.Activate();
		else
			action.Pause();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnCancelButton()
	{
		SCR_WorkshopItemActionDownload action = GetSelectedAction();
		
		if (!action)
			return;
		
		action.Cancel();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnAutoenableButton()
	{
		SCR_WorkshopSettings settings = SCR_WorkshopSettings.Get();
		SCR_WorkshopSettings.Save(settings);
	}
}