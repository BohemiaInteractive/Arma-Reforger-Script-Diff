/*!
Download manager entry for handling addon download state
*/

//------------------------------------------------------------------------------------------------
void ScriptInvoker_DownloadManagerEntry(SCR_DownloadManagerEntry entry);
typedef func ScriptInvoker_DownloadManagerEntry;

//------------------------------------------------------------------------------------------------
class SCR_DownloadManagerEntry : SCR_ScriptedWidgetComponent
{
	const int PAUSE_ENABLE_DELAY_MS = 1000; // delay used to prevent spamming pause/resume request
	protected const int PROCESSING_MESSAGE_UPDATE_DELAY = 2000; // delay to prevent message flickering when downloading switched to copying fragments
	
	// State messages
	protected const string PERCENTAGE = "#AR-SupportStation_ActionFormat_Percentage";
	
	[Attribute(UIConstants.ICONS_IMAGE_SET, UIWidgets.ResourceNamePicker, desc: "Imageset for the icon", params: "imageset")]
	protected ResourceName m_sIconImageset;
	
	[Attribute(UIConstants.PROCESSING_SPINNER_ANIMATION_SPEED.ToString())]
	protected float m_fIconAnimationSpeed;
	
	[Attribute(defvalue: SCR_Enum.GetDefault(EAnimationCurve.EASE_IN_OUT_SINE), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EAnimationCurve))]
	protected EAnimationCurve m_eIconAnimationCurve;
	
	[Attribute("0")]
	protected bool m_bHideProgressBarOnComplete;
	
	protected ref SCR_DownloadManagerEntryWidgets m_Widgets = new SCR_DownloadManagerEntryWidgets();
	protected ref SCR_BackendImageComponent m_BackendImage;
	
	protected ref SCR_WorkshopItemActionDownload m_Action;
	protected ref SCR_WorkshopItem m_Item;
	
	protected bool m_bPauseEnabled = true;
	protected bool m_bShowButtons;
	protected bool m_bDelayedProcessingDisplay;
	protected EDownloadManagerActionState m_iState = EDownloadManagerActionState.INACTIVE;
	
	protected WidgetAnimationImageRotation m_SpinnerAnimation;
	
	protected ref ScriptInvokerBase<ScriptInvoker_DownloadManagerEntry> m_OnUpdate;
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<ScriptInvoker_DownloadManagerEntry> GetOnUpdate()
	{
		if (!m_OnUpdate)
			m_OnUpdate = new ScriptInvokerBase<ScriptInvoker_DownloadManagerEntry>();
		
		return m_OnUpdate;
	}
	
	//------------------------------------------------------------------------------------------------
	// Override 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_Widgets.Init(w);
		
		m_Widgets.m_PauseBtnComponent.m_OnClicked.Insert(OnClickPause);
		m_Widgets.m_ResumeBtnComponent.m_OnClicked.Insert(OnClickResume);
		m_Widgets.m_CancelBtnComponent.m_OnClicked.Insert(OnClickCancel);
		m_Widgets.m_RetryBtnComponent.m_OnClicked.Insert(OnClickRetry);
		
		m_Widgets.m_HorizontalButtons.SetVisible(false);
		m_Widgets.m_PauseBtn.SetVisible(false);
		m_Widgets.m_ResumeBtn.SetVisible(false);
		m_Widgets.m_CancelBtn.SetVisible(false);
		m_Widgets.m_RetryBtn.SetVisible(false);
		
		// Processing spinner animation
		m_Widgets.m_ProcessingSpinner.SetRotation(360);
		m_SpinnerAnimation = AnimateWidget.Rotation(m_Widgets.m_ProcessingSpinner, 0, m_fIconAnimationSpeed);
		if (m_SpinnerAnimation)
		{
			m_SpinnerAnimation.GetOnCycleCompleted().Insert(AnimateIcon);
			AnimateIcon(m_SpinnerAnimation);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_bShowButtons = true;
		UpdateButtons();

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW,int x, int y)
	{
		m_bShowButtons = false;
		UpdateButtons();
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// Custom
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void SetupWidgets()
	{
		if (!m_Action)
		{
			FallbackVisuals();
			return;
		}
		
		// Image 
		if (m_Item && m_Item.GetWorkshopItem() && m_Item.GetWorkshopItem().Thumbnail())
			m_BackendImage.SetImage(m_Item.GetWorkshopItem().Thumbnail());
		else
			m_BackendImage.SetImage(null);
		
		// Name
		m_Widgets.m_NameText.SetText(m_Action.GetAddonName());
		
		// Versions
		m_Widgets.m_VersionText.SetText(m_Action.GetTargetRevision().GetVersion());
	}
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Unlocalized fallback visuals

	//------------------------------------------------------------------------------------------------
	// Display placeholder values if action failed to be loaded
	protected void FallbackVisuals()
	{
		if (!m_Item)
		{
			m_Widgets.m_NameText.SetText("...");
			m_Widgets.m_VersionText.SetText("...");
			m_BackendImage.SetImage(null);
			
			return;
		}
		
		m_Widgets.m_NameText.SetText(m_Item.GetName());
		
		// Versions
		if (m_Item.GetItemTargetRevision())
			m_Widgets.m_VersionText.SetText(m_Item.GetItemTargetRevision().GetVersion());
		else
			m_Widgets.m_VersionText.SetText("...");
		
		// Image
		if (m_Item.GetWorkshopItem() && m_Item.GetWorkshopItem().Thumbnail())
			m_BackendImage.SetImage(m_Item.GetWorkshopItem().Thumbnail());
		else
			m_BackendImage.SetImage(null);
	}
	
//---- REFACTOR NOTE END ----	

	//------------------------------------------------------------------------------------------------
	//! Update all text displaying progress
	protected void UpdateProgressWidgets(bool force = false)
	{
		m_iState = DownloadActionState(m_Action);
		
		// Progress bar
		bool running = (m_iState == EDownloadManagerActionState.RUNNING); 
		bool runningOrFailed = running || m_iState == EDownloadManagerActionState.FAILED;
		
		if (m_bHideProgressBarOnComplete)
			m_Widgets.m_Progress.SetVisible(runningOrFailed);
		
		// Download size
		string addonSize = SCR_ByteFormat.ContentDownloadFormat(m_Action.GetSizeBytes());
		
		if (runningOrFailed)
		{
			// Show progress
			string downloadSize = SCR_ByteFormat.ContentDownloadFormat(m_Action.GetSizeBytes() * m_Action.GetProgress());
			
			m_Widgets.m_DownloadedText.SetText(string.Format("%1/%2", downloadSize, addonSize));
			m_Widgets.m_ProgressComponent.SetValue(m_Action.GetProgress());
		}
		else
		{
			m_Widgets.m_DownloadedText.SetText(addonSize);
			
			if (m_Action.IsCompleted())
				m_Widgets.m_ProgressComponent.SetValue(m_Action.GetProgress());
		}
		
		// Update buttons 
		UpdateButtons();
		
		bool processing = m_Action.IsProcessing() && !(m_Action.IsPaused() || m_Action.IsFailed() || m_Action.IsCanceled());
		if (!processing || force)
		{
			UpdateMessage(processing);
		}
		// Delay to avoid flickering
		else
		{
			m_bDelayedProcessingDisplay = true;
			GetGame().GetCallqueue().CallLater(UpdateMessage, PROCESSING_MESSAGE_UPDATE_DELAY, false, processing);
		}
		
		// Invoke
		if (m_OnUpdate)
			m_OnUpdate.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateMessage(bool processing)
	{
		if (m_bDelayedProcessingDisplay)
		{
			GetGame().GetCallqueue().Remove(UpdateMessage);
			m_bDelayedProcessingDisplay = false;
		}
		
		// Show message 
		string msg, imgName;
		Color col;
		StateMessage(msg, imgName, col);
		
		m_Widgets.m_DownloadStateText.SetText(msg);
		m_Widgets.m_DownloadIcon.LoadImageFromSet(0, m_sIconImageset, imgName);
		m_Widgets.m_DownloadIconShadow.LoadImageFromSet(0, m_sIconImageset, imgName);
		
		if (col)
		{
			m_Widgets.m_DownloadIcon.SetColor(col);
			m_Widgets.m_DownloadStateText.SetColor(col);
			m_Widgets.m_ProgressComponent.SetSliderColor(col);
			m_Widgets.m_ProcessingSpinner.SetColor(col);	
		}

		m_Widgets.m_ProcessingSpinner.SetVisible(processing);
		m_Widgets.m_DownloadIcon.SetVisible(!processing);
		m_Widgets.m_DownloadIconShadow.SetVisible(!processing);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateButtons()
	{
		m_Widgets.m_HorizontalButtons.SetVisible(m_bShowButtons);

		if (!m_bShowButtons)
			return;
		
		bool pause, resume, cancel, retry;
		CanDoActions(pause, resume, cancel, retry);
		
		// Buttons
		m_Widgets.m_PauseBtn.SetVisible(pause);
		m_Widgets.m_ResumeBtn.SetVisible(resume);
		m_Widgets.m_CancelBtn.SetVisible(cancel);
		m_Widgets.m_RetryBtn.SetVisible(retry);
		
		m_Widgets.m_PauseBtn.SetEnabled(m_Action.IsRunningAsyncSolved());
		m_Widgets.m_ResumeBtn.SetEnabled(m_Action.IsPauseAsyncSolved());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup state message variables
	protected void StateMessage(out string message, out string imageName, out Color color)
	{
		float progress;

		// Downloading
		if (m_Action.IsActive() && !m_Action.IsCompleted() && !m_Action.IsPaused())
		{	
			// Copying fragments
			if (m_Action.IsProcessing())
			{
				progress = m_Action.GetProcessingProgress();

				message = WidgetManager.Translate(PERCENTAGE, SCR_WorkshopUiCommon.DOWNLOAD_STATE_PROCESSING, SCR_WorkshopUiCommon.GetDownloadProgressPercentage(progress));
				imageName = SCR_WorkshopUiCommon.DOWNLOAD_STATE_ICON_PROCESSING;
				color = Color.FromInt(UIColors.ONLINE.PackToInt());
				return;
			}
			
			// Download running  
			progress = m_Action.GetProgress();

			message = WidgetManager.Translate(PERCENTAGE, SCR_WorkshopUiCommon.DOWNLOAD_STATE_DOWNLOADING, SCR_WorkshopUiCommon.GetDownloadProgressPercentage(progress));
			imageName = SCR_WorkshopUiCommon.DOWNLOAD_STATE_ICON_DOWNLOADING;
			color = Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt());
			return;
		}
		
		// Paused 
		if (m_Action.IsPaused())
		{
			progress = m_Action.GetProgress();

			message = WidgetManager.Translate(PERCENTAGE, SCR_WorkshopUiCommon.DOWNLOAD_STATE_PAUSED, SCR_WorkshopUiCommon.GetDownloadProgressPercentage(progress));
			imageName = SCR_WorkshopUiCommon.DOWNLOAD_STATE_ICON_PAUSED;
			color = Color.FromInt(UIColors.CONTRAST_DEFAULT.PackToInt());
			return;
		}
		
		// Canceled 
		if (m_Action.IsCanceled())
		{
			message = SCR_WorkshopUiCommon.DOWNLOAD_STATE_CANCELED;
			imageName = UIConstants.ICON_CANCEL;
			color = Color.FromInt(UIColors.WARNING.PackToInt());
			return;
		}
		
		// Failed 
		if (m_Action.IsFailed())
		{
			FailReason(message, imageName);
			color = Color.FromInt(UIColors.WARNING.PackToInt());
			return;
		}
		
		// Success
		if (m_Action.IsCompleted())
		{
			message = SCR_WorkshopUiCommon.DOWNLOAD_STATE_COMPLETED;
			imageName = UIConstants.ICON_OK;
			color = Color.FromInt(UIColors.CONFIRM.PackToInt());
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return message with details why entry wasn't downloaded
	protected void FailReason(out string message, out string icon)
	{
		// Addon specific fail
		message = SCR_WorkshopUiCommon.DOWNLOAD_STATE_DOWNLOAD_FAIL;
		icon = UIConstants.ICON_WARNING;
		
		// Communication with server failed
		if (!SCR_ServicesStatusHelper.IsBackendConnectionAvailable())
		{
			message = SCR_ConnectionUICommon.MESSAGE_VERBOSE_DISCONNECTION;
			icon = SCR_ConnectionUICommon.ICON_SERVICES_ISSUES;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Return message with details why entry wasn't downloaded
	protected void AnimateIcon(WidgetAnimationBase animation)
	{
		if (m_SpinnerAnimation)
		{
			m_SpinnerAnimation.GetOnCycleCompleted().Remove(AnimateIcon);
			m_SpinnerAnimation.Stop();
		}
		
		m_Widgets.m_ProcessingSpinner.SetRotation(360);
		m_SpinnerAnimation = AnimateWidget.Rotation(m_Widgets.m_ProcessingSpinner, 0, m_fIconAnimationSpeed);
		if (m_SpinnerAnimation)
		{
			m_SpinnerAnimation.SetRepeat(true);
			m_SpinnerAnimation.SetCurve(m_eIconAnimationCurve);
			m_SpinnerAnimation.GetOnCycleCompleted().Insert(AnimateIcon);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DisablePauserResume(SCR_ModularButtonComponent button)
	{
		m_bPauseEnabled = false;
		button.SetEnabled(false);
		// Enable pause button later to prevent request spamming
		GetGame().GetCallqueue().Remove(EnablePauserResume);
		GetGame().GetCallqueue().CallLater(EnablePauserResume, PAUSE_ENABLE_DELAY_MS, false, button);
		
		if (m_OnUpdate)
			m_OnUpdate.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EnablePauserResume(SCR_ModularButtonComponent button)
	{
		m_bPauseEnabled = true;
		button.SetEnabled(true);
		
		if (m_OnUpdate)
			m_OnUpdate.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnActionChanged(SCR_WorkshopItemAction action)
	{
		UpdateProgressWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	// Public
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Based on current download action state setup state simplified for download manager UI grouping
	//! Function is static to compare download action state before entry is created
	static EDownloadManagerActionState DownloadActionState(SCR_WorkshopItemActionDownload action)
	{
		if (!action)
		{
			Print("Download manager entry state can't be update due to missing download action");
			return EDownloadManagerActionState.INACTIVE;
		}
		
		// Running  
		if (!action.IsCompleted() && !action.IsFailed() && !action.IsCanceled())
			return EDownloadManagerActionState.RUNNING;
		
		// Failed
		if (action.IsFailed() || action.IsCanceled())
			return EDownloadManagerActionState.FAILED;
		
		// Downloaded
		if (action.IsCompleted() && !action.IsFailed() && !action.IsCanceled())
			return EDownloadManagerActionState.DOWNLOADED;
		
		// No state
		return EDownloadManagerActionState.INACTIVE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes the line in interactive mode.
	//! It will be able to interact with the download action.
	void InitForDownloadAction(SCR_WorkshopItem item, SCR_WorkshopItemActionDownload action)
	{
		m_Item = item;
		m_Action = action;
		
		m_Action.m_OnChanged.Insert(OnActionChanged);
		
		m_BackendImage = SCR_BackendImageComponent.Cast(GetRootWidget().FindHandler(SCR_BackendImageComponent));
		
		// Setup static 
		SetupWidgets();
		
		if (m_Action)
			UpdateProgressWidgets(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Store which actions can be used for entry in current state
	void CanDoActions(out bool canPause, out bool canResume, out bool canCancel, out bool canRetry)
	{	
		canPause = m_iState == EDownloadManagerActionState.RUNNING && !m_Action.IsPaused();
		canResume = m_iState == EDownloadManagerActionState.RUNNING && m_Action.IsPaused();
		canCancel = m_iState == EDownloadManagerActionState.RUNNING;
		canRetry = m_iState == EDownloadManagerActionState.FAILED;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnClickPause()
	{
		if (!m_Action)
			return;
		
		if (m_Action.IsActive() && !m_Action.IsPaused())
			m_Action.Pause();
		
		UpdateProgressWidgets();
		
		// Set disabled
		//DisablePauserResume(m_Widgets.m_ResumeBtnComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnClickResume()
	{
		if (!m_Action)
			return;
		
		if (m_Action.IsPaused())
			m_Action.Resume();
		else if (m_Action.IsInactive())
			m_Action.Activate();
		
		UpdateProgressWidgets(true);
		
		// Set disabled
		//DisablePauserResume(m_Widgets.m_PauseBtnComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnClickCancel()
	{
		if (m_Action)
			m_Action.Cancel();
		
		UpdateProgressWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnClickRetry()
	{
		if (m_Action)
			m_Action.RetryDownload();

		if (m_wRoot)
			UpdateProgressWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	// Get set
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetItem()
	{
		return m_Item;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItemActionDownload GetDownloadAction()
	{
		return m_Action;
	}
	
	//------------------------------------------------------------------------------------------------
	EDownloadManagerActionState GetState()
	{
		return m_iState;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetPauseEnabled()
	{
		return m_bPauseEnabled;
	}
}