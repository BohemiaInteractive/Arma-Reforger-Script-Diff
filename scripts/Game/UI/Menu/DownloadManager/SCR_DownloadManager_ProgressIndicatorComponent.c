/*!
Component which visualizes state of download manager: progress bar, progress text.
*/
class SCR_DownloadManager_ProgressIndicatorComponent : ScriptedWidgetComponent
{
	ref SCR_DownloadManager_ProgressIndicatorWidgets m_Widgets = new SCR_DownloadManager_ProgressIndicatorWidgets();
	
	//------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_Widgets.Init(w);
		
		Update();
		
		GetGame().GetCallqueue().CallLater(Update, 50, true);
	}
	
	
	
	//------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{	
		if (GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(Update);
	}
	
	
	
	//------------------------------------------------------------------------------------------
	void Update()
	{
		// Download state text
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr && !SCR_Global.IsEditMode())
		{
			UpdateAllWidgets();
		}
	}
	
	
	
	//------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		string downloadStateText;
		int nCompleted, nTotal;
		mgr.GetDownloadQueueState(nCompleted, nTotal);
		
		if (nTotal > 0 && !mgr.GetDownloadsPaused())
			downloadStateText = string.Format("%1 %2 / %3", WidgetManager.Translate(SCR_WorkshopUiCommon.DOWNLOAD_STATE_DOWNLOADING), nCompleted, nTotal);
		else if (nTotal > 0 && mgr.GetDownloadsPaused())
			downloadStateText = string.Format("%1 %2 / %3", WidgetManager.Translate("#AR-DownloadManager_State_AllDownloadsPaused"), nCompleted, nTotal);
		else if (mgr.GetDownloadsPaused())
			downloadStateText = "#AR-DownloadManager_State_AllDownloadsPaused";
		else
			downloadStateText = "#AR-DownloadManager_State_NoActiveDownloads";
		
		m_Widgets.m_StateText.SetText(downloadStateText);
		
		// Progress bar, progress text
		if (nTotal > 0)
		{
			// Get total progress of all downloads
			array<ref SCR_WorkshopItemActionDownload> downloadQueue = mgr.GetDownloadQueue();
			float progress = SCR_DownloadManager.GetDownloadActionsProgress(downloadQueue);
			
			// Progress bar
			m_Widgets.m_ProgressBar.SetCurrent(progress);
			
			// Progress percent text
			string progressText = string.Format("%1 %%", Math.Floor(progress * 100.0));
			m_Widgets.m_ProgressText.SetText(progressText);
		}
		else
		{
			m_Widgets.m_ProgressBar.SetCurrent(0);
		}
		
		//m_Widgets.m_ProgressBar.SetVisible(nTotal > 0);
		m_Widgets.m_ProgressOverlay.SetVisible(nTotal > 0);
	}
	
	
	
};