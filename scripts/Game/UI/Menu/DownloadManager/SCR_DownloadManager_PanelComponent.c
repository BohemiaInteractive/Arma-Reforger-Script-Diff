/*
Widget component of the download manager panel, which can be added into any menu where you want to have the download manager panel.

It provides basic data of the current state of downloads.
*/

class SCR_DownloadManager_PanelComponent : ScriptedWidgetComponent
{
	// The panel will be visible for this time, then it will fade out
	protected const float FADE_OUT_WAIT_TIME = 5;
	
	protected ref SCR_DownloadManager_PanelWidgets m_Widgets = new SCR_DownloadManager_PanelWidgets();
	protected Widget m_wRoot;
	protected ref SCR_FadeInOutAnimator m_Animator;
		
	protected int m_iLastUpdateTimeMs;
	
	//------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_Widgets.Init(w);
		
		m_iLastUpdateTimeMs = System.GetTickCount();
		
		if (SCR_DownloadManager.GetInstance())
			SCR_DownloadManager.GetInstance().m_OnNewDownload.Insert(Callback_OnNewDownload);
		
		m_Widgets.m_OpenButtonComponent.m_OnActivated.Insert(OnOpenButton);
		
		m_Animator = new SCR_FadeInOutAnimator(m_wRoot, UIConstants.FADE_RATE_FAST, UIConstants.FADE_RATE_SLOW, FADE_OUT_WAIT_TIME, fadeOutSetVisibleFalse: true);
		m_Animator.FadeOutInstantly();
		
		Update();
	}
	
	
	//------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{	
		if (GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(Update);
	
		if (SCR_DownloadManager.GetInstance())
			SCR_DownloadManager.GetInstance().m_OnNewDownload.Remove(Callback_OnNewDownload);
	}
	
	
	//------------------------------------------------------------------------------------------
	void Update()
	{
		int tDeltaMs = System.GetTickCount() - m_iLastUpdateTimeMs;
		float tDelta = 0.001 * tDeltaMs;
		
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr && !SCR_Global.IsEditMode())
		{
			UpdateAllWidgets(tDelta);
			
			GetGame().GetCallqueue().CallLater(Update, 10);
		}
		
		m_iLastUpdateTimeMs = System.GetTickCount();
	}
	
	
	//------------------------------------------------------------------------------------------
	protected void UpdateAllWidgets(float tDelta)
	{
		// Update whole overlay visibility
		// It's visible when there are some downloads
		
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		int nTotal, nCompleted;
		mgr.GetDownloadQueueState(nCompleted, nTotal);
		
		m_Animator.ForceVisible(nTotal > 0);
		
		m_Animator.Update(tDelta);
	}
	
	
	//------------------------------------------------------------------------------------------
	void Callback_OnNewDownload(SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	{
	}
	
	
	//------------------------------------------------------------------------------------------
	void OnOpenButton()
	{
		SCR_DownloadManager_Dialog.Create();
	}
	
};