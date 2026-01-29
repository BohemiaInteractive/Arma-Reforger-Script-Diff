/*!
Component which visualizes state of download manager with the small circular indicator.
*/

class SCR_DownloadManager_CircularIndicatorComponent : SCR_ScriptedWidgetComponent
{
	protected const float FADE_OUT_WAIT_TIME = 2.5;

	protected ref SCR_DownloadManager_CircleProgressIndicatorWidgets m_Widgets = new SCR_DownloadManager_CircleProgressIndicatorWidgets();

	protected ref SCR_FadeInOutAnimator m_Animator;
	
	protected bool m_bIsDownloading;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_Widgets.Init(w);

		m_Animator = new SCR_FadeInOutAnimator(m_Widgets.m_DownloadButton, UIConstants.FADE_RATE_FAST, UIConstants.FADE_RATE_SLOW, FADE_OUT_WAIT_TIME, fadeOutSetVisibleFalse: true);
		
		if (!SCR_Global.IsEditMode())
		{
			UpdateAllWidgets();
			
			if (!m_bIsDownloading)
				m_Animator.FadeOutInstantly();
		}

		m_Widgets.m_DownloadButtonComponent.m_OnClicked.Insert(OnButtonClick);

		// Owner menu events
		SCR_MenuHelper.GetOnMenuFocusGained().Insert(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuOpen().Insert(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuFocusLost().Insert(OnMenuDisabled);
		SCR_MenuHelper.GetOnMenuClose().Insert(OnMenuDisabled);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		GetGame().GetCallqueue().Remove(OnFrame);
		
		// Owner menu events
		SCR_MenuHelper.GetOnMenuFocusGained().Remove(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuOpen().Remove(OnMenuEnabled);
		SCR_MenuHelper.GetOnMenuFocusLost().Remove(OnMenuDisabled);
		SCR_MenuHelper.GetOnMenuClose().Remove(OnMenuDisabled);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFrame(float tDelta)
	{
		if (!m_wRoot)
			return;
		
		//float tDelta = ftime / 1000.0; // delta time for callqueue
		
		if (!SCR_Global.IsEditMode())
			UpdateAllWidgets();

		m_Animator.Update(tDelta);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets()
	{
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (!mgr)
			return;
		
		int nCompleted, nTotal;
		mgr.GetDownloadQueueState(nCompleted, nTotal);

		m_bIsDownloading = nTotal > 0;
		
		// Progress bar, progress text
		m_Widgets.m_DownloadDoneImage.SetVisible(nTotal == 0);
		m_Widgets.m_QueueSizeText.SetVisible(m_bIsDownloading);
		if (m_bIsDownloading)
		{
			// Get total progress of all downloads
			array<ref SCR_WorkshopItemActionDownload> downloadQueue = mgr.GetDownloadQueue();
			float progress = SCR_DownloadManager.GetDownloadActionsProgress(downloadQueue);

			// Progress bar
			m_Widgets.m_ProgressCircle.SetMaskProgress(progress);

			// Queue size text
			string queueSizeText = string.Format("%1", nTotal - nCompleted);
			m_Widgets.m_QueueSizeText.SetText(queueSizeText);
		}
		else
		{
			m_Widgets.m_QueueSizeText.SetText(string.Empty);
			m_Widgets.m_ProgressCircle.SetMaskProgress(1.0);
		}

		m_Animator.ForceVisible(m_bIsDownloading);

		// Disable button when invisible
		m_Widgets.m_DownloadButtonComponent.SetEnabled(m_bIsDownloading);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonClick()
	{
		SCR_DownloadManager_Dialog.Create();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuEnabled(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(GetRootWidget()))
		{
			UpdateAllWidgets();
			if (!m_bIsDownloading)
				m_Animator.FadeOutInstantly();
			
			menu.m_OnUpdate.Remove(OnFrame);
			menu.m_OnUpdate.Insert(OnFrame);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuDisabled(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(GetRootWidget()))
		{
			menu.m_OnUpdate.Remove(OnFrame);
		}
	}
}
