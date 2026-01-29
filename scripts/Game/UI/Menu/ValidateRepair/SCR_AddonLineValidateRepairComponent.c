class SCR_AddonLineValidateRepairComponent : SCR_ListMenuEntryComponent
{
	protected const int REFRESH_DELAY = 1000;

	protected ref SCR_ValidateRepairLineWidgets m_Widgets = new SCR_ValidateRepairLineWidgets();

	protected ref map<SCR_EAddonValidateState, ref SCR_AddonLineValidateState> m_mVisualStates;
	protected ref map<int, string> m_mErrorMessages;

	protected ref SCR_WorkshopItem m_Item;
	protected SCR_BackendImageComponent m_BackendImageComponent;

	protected bool m_bIsAddonEnabled;
	protected bool m_bIsInitialized;			// Is the Addon details loaded
	protected bool m_bIsReSubscribing;			// Are we currently trying to fix the mod by completely removing it and downloading it from scratch
	protected bool m_bUpdatingContinuously;
	protected bool m_bChangedState;
	protected float m_fCurrentProgress;			// 0.0 to 1.0

	protected SCR_EAddonValidateState m_eCurrentState;
	protected string m_sDownloadText;

	protected ref BackendCallback m_Callback;
	protected ref SCR_WorkshopDownloadSequence m_DownloadRequest;

	protected ref ScriptInvokerScriptedWidgetComponent m_OnValidateStart;
	protected ref ScriptInvokerScriptedWidgetComponent m_OnValidateEnd;

	protected ref ScriptInvokerScriptedWidgetComponent m_OnRepairStart;
	protected ref ScriptInvokerScriptedWidgetComponent m_OnRepairEnd;

	protected ref ScriptInvokerScriptedWidgetComponent m_OnDownloadStart;
	protected ref ScriptInvokerScriptedWidgetComponent m_OnDownloadEnd;

	//------------------------------------------------------------------------------------------------
	bool IsBusy()
	{
		return (m_eCurrentState == SCR_EAddonValidateState.VALIDATING || m_eCurrentState == SCR_EAddonValidateState.REPAIRING || m_eCurrentState == SCR_EAddonValidateState.DOWNLOADING || m_eCurrentState == SCR_EAddonValidateState.UPDATING || m_eCurrentState == SCR_EAddonValidateState.NEED_UPDATING);
	}

	//------------------------------------------------------------------------------------------------
	bool HasFailed()
	{
		return (m_eCurrentState == SCR_EAddonValidateState.VALIDATING_FAILED || m_eCurrentState == SCR_EAddonValidateState.REPAIRING_FAILED);
	}

	//------------------------------------------------------------------------------------------------
	bool HasFailedDownloading()
	{
		return m_eCurrentState == SCR_EAddonValidateState.DOWNLOADING_FAILED;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsBusyExtended()
	{
		if (IsBusy() || !m_Item || m_Item.IsDownloadRunning())
			return true;
		
		WorkshopItem workshop = m_Item.GetWorkshopItem();
		if (!workshop || workshop.IsVerificationRunning())
			return true;
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetWorkshopItem()
	{
		return m_Item;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceChange(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		if (newDevice != EInputDeviceType.MOUSE)
			m_Widgets.m_RedownloadButtonComponent0.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		bool result = super.OnFocus(w, x, y);

		if (HasFailed() && !IsBusy() && !HasFailedDownloading())
			m_Widgets.m_RedownloadButtonComponent0.SetVisible(GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE);

		UpdateAllWidgets();

		return result;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		bool result = super.OnFocusLost(w, x, y);

		m_Widgets.m_RedownloadButtonComponent0.SetVisible(false);
		UpdateAllWidgets();

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Update visual state of line
	protected void UpdateAllWidgets()
	{
		if (!m_Item)
			return;

		// Update name
		m_Widgets.m_wNameText.SetText(m_Item.GetName());
		UpdateImage();
		UpdateDownloadProgressBar();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateImage()
	{
		if (!m_BackendImageComponent)
			return;

		m_BackendImageComponent.SetImage(m_Item.GetThumbnail());
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDownloadProgressBar()
	{
		if (!m_Widgets)
			return;

		float progress = 1;
		switch(m_eCurrentState)
		{
			case SCR_EAddonValidateState.VALIDATING:
				progress = m_Item.GetWorkshopItem().GetVerificationProgress();
			break;
			case SCR_EAddonValidateState.NEED_UPDATING:
			case SCR_EAddonValidateState.IDLE:
				progress = 0;
			break;
			case SCR_EAddonValidateState.REPAIRING:
			case SCR_EAddonValidateState.DOWNLOADING:
			case SCR_EAddonValidateState.UPDATING:
				progress = m_Item.GetWorkshopItem().GetProgress();
			break;
		}

		if (m_bChangedState || m_fCurrentProgress != progress)
		{
			if (m_eCurrentState == SCR_EAddonValidateState.DOWNLOADING && progress == 1.0)
			{
				OnDownloadFinished();
				return; // Early return so we dont update the visuals twice
			}
			else if (m_eCurrentState == SCR_EAddonValidateState.UPDATING && progress == 1.0)
			{
				FinishedUpdating();
				return; // Early return so we dont update the visuals twice
			}

			if (m_bChangedState)
			{
				SCR_AddonLineValidateState currentState = m_mVisualStates.Get(m_eCurrentState);
				m_sDownloadText = currentState.GetText();

				m_Widgets.m_ProgressComponent.SetSliderColor(currentState.GetColor());
				m_Widgets.m_wDownloadStateText.SetColor(currentState.GetColor());
				m_Widgets.m_LoadingOverlayComponent.SetShown(IsBusy());

				m_bChangedState = false;
			}

			m_fCurrentProgress = progress;
			int progressInt = m_fCurrentProgress * 100;
			m_Widgets.m_wDownloadStateText.SetTextFormat(m_sDownloadText, progressInt);
			m_Widgets.m_ProgressComponent.StopProgressAnimation();
			m_Widgets.m_ProgressComponent.SetValue(m_fCurrentProgress);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void StartContinuousUpdate()
	{
		if (m_bUpdatingContinuously || !m_Item)
			return;

		m_fCurrentProgress = 0;

		m_Widgets.m_LoadingOverlayComponent.SetShown(true);

		UpdateDownloadProgressBar();

		GetGame().GetCallqueue().CallLater(ContinuousUpdate, REFRESH_DELAY, true);

		m_bUpdatingContinuously = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void StopContinuousUpdate(bool skipUIUpdate = false)
	{
		if (m_bUpdatingContinuously)
			GetGame().GetCallqueue().Remove(ContinuousUpdate);

		if (!skipUIUpdate)
		{
			UpdateDownloadProgressBar();
			m_Widgets.m_LoadingOverlayComponent.SetShown(false);
		}

		m_bUpdatingContinuously = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void ContinuousUpdate()
	{
		if (!m_Item)
		{
			m_eCurrentState = SCR_EAddonValidateState.IDLE;
			m_bChangedState = true;

			StopContinuousUpdate(true);
		}

		UpdateDownloadProgressBar();
	}

	//------------------------------------------------------------------------------------------------
	bool StartValidating()
	{
		if (IsBusyExtended())
			return false;

		m_Widgets.m_wDownloadErrorText.SetText(string.Empty);

		WorkshopItem workshop = m_Item.GetWorkshopItem();

		m_Callback = new BackendCallback();
		m_Callback.SetOnSuccess(OnValidationSuccess);
		m_Callback.SetOnError(OnValidationError);
		workshop.VerifyIntegrity(m_Callback);

		m_eCurrentState = SCR_EAddonValidateState.VALIDATING;
		m_bChangedState = true;

		if (m_OnValidateStart)
			m_OnValidateStart.Invoke(this);

		StartContinuousUpdate();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void StopValidating(bool skipUIUpdate = false)
	{
		if (m_Item)
			m_Item.GetWorkshopItem().CancelVerification();

		if (m_OnValidateEnd)
			m_OnValidateEnd.Invoke(this);

		m_eCurrentState = SCR_EAddonValidateState.IDLE;
		m_bChangedState = true;

		StopContinuousUpdate(skipUIUpdate);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnValidationSuccess()
	{
		m_eCurrentState = SCR_EAddonValidateState.FINISHED;
		m_bChangedState = true;

		StopContinuousUpdate();

		if (m_OnValidateEnd)
			m_OnValidateEnd.Invoke(this);

		PrintFormat("ValidateAndRepair: Validated Addon (%1 %2) with code:%3", m_Item.GetName(), m_Item.GetId(), m_Callback.GetBackendError(), level: LogLevel.DEBUG);

		if (m_Item.GetWorkshopItem().GetActiveRevision().IsCorrupted())
		{
			if (!StartRepairing())
			{
				OnValidationError();
				return;
			}
		}
		

		if (m_OnValidateEnd)
			m_OnValidateEnd.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnValidationError()
	{
		m_eCurrentState = SCR_EAddonValidateState.VALIDATING_FAILED;
		m_bChangedState = true;

		StopContinuousUpdate();

		if (m_OnValidateEnd)
			m_OnValidateEnd.Invoke(this);

		PrintFormat("ERROR !!! ValidateAndRepair: Unable to Validate Addon (%1 %2) with code:%3", m_Item.GetName(), m_Item.GetId(), m_Callback.GetBackendError(), level: LogLevel.WARNING);

		SetErrorMessage(m_Callback.GetBackendError());
	}

	//------------------------------------------------------------------------------------------------
	bool StartRepairing()
	{
		if (IsBusyExtended() || !m_Item.GetWorkshopItem().GetActiveRevision().IsCorrupted())
			return false;

		WorkshopItem workshop = m_Item.GetWorkshopItem();

		m_Widgets.m_wDownloadErrorText.SetText(string.Empty);

		m_Callback = new BackendCallback();
		m_Callback.SetOnSuccess(OnRepairingSuccess);
		m_Callback.SetOnError(OnRepairingError);
		workshop.RepairIntegrity(m_Callback);

		m_eCurrentState = SCR_EAddonValidateState.REPAIRING;
		m_bChangedState = true;

		if (m_OnRepairStart)
			m_OnRepairStart.Invoke(this);

		StartContinuousUpdate();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void StopRepairing(bool skipUIUpdate = false)
	{
		if (m_Item)
			m_Item.GetWorkshopItem().Cancel(null);

		if (m_OnRepairEnd)
			m_OnRepairEnd.Invoke(this);

		m_eCurrentState = SCR_EAddonValidateState.IDLE;
		m_bChangedState = true;

		StopContinuousUpdate(skipUIUpdate);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRepairingSuccess()
	{
		m_eCurrentState = SCR_EAddonValidateState.FINISHED;
		m_bChangedState = true;

		if (m_OnRepairEnd)
			m_OnRepairEnd.Invoke(this);

		PrintFormat("ValidateAndRepair: Repaired Addon (%1 %2) with code:%3", m_Item.GetName(), m_Item.GetId(), m_Callback.GetBackendError(), level: LogLevel.DEBUG);
		
		if (m_bIsAddonEnabled)
			m_Item.SetEnabled(true);

		StopContinuousUpdate();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRepairingError()
	{
		m_eCurrentState = SCR_EAddonValidateState.REPAIRING_FAILED;
		m_bChangedState = true;

		if (m_OnRepairEnd)
			m_OnRepairEnd.Invoke(this);

		PrintFormat("ERROR !!! ValidateAndRepair: Unable to Repair Addon (%1 %2) with code:%3", m_Item.GetName(), m_Item.GetId(), m_Callback.GetBackendError(), level: LogLevel.WARNING);

		SetErrorMessage(m_Callback.GetBackendError());

		StopContinuousUpdate();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClickAttemptResubscribing()
	{
		StartCleanDownload();
	}

	//------------------------------------------------------------------------------------------------
	bool StartCleanDownload()
	{
		if (IsBusyExtended())
			return false;

		m_Widgets.m_wDownloadErrorText.SetText(string.Empty);

		m_Item.DeleteLocally();

		Revision revision = m_Item.GetLatestRevision();
		m_DownloadRequest = SCR_WorkshopDownloadSequence.Create(m_Item, revision, m_DownloadRequest);

		m_DownloadRequest.GetOnReady().Insert(OnDownloadStart);
		m_DownloadRequest.GetOnCancel().Insert(OnDownloadError);
		m_DownloadRequest.GetOnError().Insert(OnDownloadError);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDownloadStart(SCR_DownloadSequence sequence)
	{
		m_eCurrentState = SCR_EAddonValidateState.DOWNLOADING;
		m_bChangedState = true;

		if (m_OnDownloadStart)
			m_OnDownloadStart.Invoke(this);

		StartContinuousUpdate();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDownloadFinished()
	{
		m_eCurrentState = SCR_EAddonValidateState.FINISHED;
		m_bChangedState = true;

		StopContinuousUpdate();

		if (m_OnDownloadEnd)
			m_OnDownloadEnd.Invoke(this);

		PrintFormat("ValidateAndRepair: Downloaded Addon (%1 %2) with code:%3", m_Item.GetName(), m_Item.GetId(), m_Callback.GetBackendError(), level: LogLevel.DEBUG);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDownloadError(SCR_DownloadSequence sequence)
	{
		m_eCurrentState = SCR_EAddonValidateState.DOWNLOADING_FAILED;
		m_bChangedState = true;

		StopContinuousUpdate();

		if (m_OnDownloadEnd)
			m_OnDownloadEnd.Invoke(this);

		PrintFormat("ERROR !!! ValidateAndRepair: Unable to Download Addon (%1 %2) with code:%3", m_Item.GetName(), m_Item.GetId(), m_Callback.GetBackendError(), level: LogLevel.WARNING);

		SetErrorMessage(SCR_EAddonValidateError.UNKNOWN);
	}

	//------------------------------------------------------------------------------------------------
	protected bool StartUpdating()
	{
		m_eCurrentState = SCR_EAddonValidateState.UPDATING;
		m_bChangedState = true;

		StartContinuousUpdate();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void FinishedUpdating()
	{
		m_eCurrentState = SCR_EAddonValidateState.IDLE;
		m_bChangedState = true;

		StopContinuousUpdate();

		PrintFormat("ValidateAndRepair: Finished Updating Addon (%1 %2)", m_Item.GetName(), m_Item.GetId(), level: LogLevel.DEBUG);
		
		StartValidating();
	}

	//------------------------------------------------------------------------------------------------
	protected bool NeedUpdating()
	{
		m_eCurrentState = SCR_EAddonValidateState.NEED_UPDATING;
		m_bChangedState = true;
		
		UpdateAllWidgets();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetErrorMessage(int code)
	{
		string errorMsg;
		if (m_mErrorMessages.Find(code, errorMsg))
			m_Widgets.m_wDownloadErrorText.SetTextFormat(errorMsg, code);
		else
			m_Widgets.m_wDownloadErrorText.SetTextFormat(m_mErrorMessages.Get(SCR_EAddonValidateError.UNKNOWN), code);
	}

	// --- Callbacks ---
	//------------------------------------------------------------------------------------------------
	protected void OnWorkshopItemChange()
	{
		// If the addon was deleted/unsubscribed, remove from the list
		if (m_Item && !m_Item.GetSubscribed())
		{
			PrintFormat("ERROR !!! ValidateAndRepair: Addon (%1 %2) with code:%3 is not subscribed", m_Item.GetName(), m_Item.GetId(), m_Callback.GetBackendError(), level: LogLevel.WARNING);
			return;
		}

		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWorkshopItemLoaded()
	{
		if (m_Item)
		{
			m_Item.m_OnGetAsset.Remove(OnWorkshopItemChange);
			m_Item.m_OnChanged.Insert(OnWorkshopItemChange);
		}

		UpdateAllWidgets();
	}

	// --- Invokers ---
	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnValidateStart()
	{
		if (!m_OnValidateStart)
			m_OnValidateStart = new ScriptInvokerScriptedWidgetComponent();

		return m_OnValidateStart;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnValidateEnd()
	{
		if (!m_OnValidateEnd)
			m_OnValidateEnd = new ScriptInvokerScriptedWidgetComponent();

		return m_OnValidateEnd;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnRepairStart()
	{
		if (!m_OnRepairStart)
			m_OnRepairStart = new ScriptInvokerScriptedWidgetComponent();

		return m_OnRepairStart;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnRepairEnd()
	{
		if (!m_OnRepairEnd)
			m_OnRepairEnd = new ScriptInvokerScriptedWidgetComponent();

		return m_OnRepairEnd;
	}

	//------------------------------------------------------------------------------------------------
	//! Setup line
	void Init(notnull SCR_WorkshopItem item, notnull map<SCR_EAddonValidateState, ref SCR_AddonLineValidateState> states, notnull map<int, string> errorMessages)
	{
		m_Item = item;

		if (!m_Item.GetDetailsLoaded())
		{
			m_Item.m_OnGetAsset.Insert(OnWorkshopItemLoaded);
			m_Item.LoadDetails();
		}
		else
			m_Item.m_OnChanged.Insert(OnWorkshopItemChange);

		m_mVisualStates = states;
		m_mErrorMessages = errorMessages;
		
		m_bIsAddonEnabled = m_Item.GetEnabled();
			
		// We should not validate/repair something currently being downloaded or outdated
		if (item.IsDownloadRunning())
			StartUpdating();
		else if (item.GetUpdateAvailable())
			NeedUpdating();

		UpdateAllWidgets();
		UpdateModularButtons();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;

		m_Widgets.Init(w);
		m_Widgets.m_wDownloadStateText.SetText(string.Empty);
		m_Widgets.m_ProgressComponent.SetValue(0, false);
		m_Widgets.m_ProgressComponent.SetAnimationTime(0.2);
		m_Widgets.m_RedownloadButtonComponent0.SetVisible(false);

		m_Widgets.m_RedownloadButtonComponent0.m_OnClicked.Insert(OnClickAttemptResubscribing);

		m_BackendImageComponent = SCR_BackendImageComponent.Cast(w.FindHandler(SCR_BackendImageComponent));
		
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceChange);

		super.HandlerAttached(w);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_eCurrentState == SCR_EAddonValidateState.VALIDATING)
			StopValidating(true);
		else if (m_eCurrentState == SCR_EAddonValidateState.REPAIRING)
			StopRepairing(true);

		if (m_OnRepairStart)
			m_OnRepairStart.Clear();

		if (m_OnRepairEnd)
			m_OnRepairEnd.Clear();

		if (m_OnValidateStart)
			m_OnValidateStart.Clear();

		if (m_OnValidateEnd)
			m_OnValidateEnd.Clear();

		super.HandlerDeattached(w);
	}
}

enum SCR_EAddonValidateError
{
	UNKNOWN = EBackendError.EBERR_UNKNOWN,
	AVAILABLE_SPACE = EBackendError.EBERR_STORAGE_IS_FULL,
	INTERNET = EBackendError.EBERR_AUTH_FAILED,
	BUSY = EBackendError.EBERR_BUSY
}

enum SCR_EAddonValidateState
{
	IDLE,

	VALIDATING,
	VALIDATING_FAILED,	// Should only be happening if the addon is already busy

	REPAIRING,
	REPAIRING_FAILED,

	DOWNLOADING,
	DOWNLOADING_FAILED,

	UPDATING,			// We cant validate a addon if its already being processed by something else, in this case an update being downloaded
	NEED_UPDATING,		// While we can validate outdated mods, we should encourage to update first

	FINISHED
}

[BaseContainerProps()]
class SCR_AddonLineValidateError
{
	[Attribute(SCR_EAddonValidateError.UNKNOWN.ToString(), UIWidgets.ComboBox, string.Empty, string.Empty, enumType: SCR_EAddonValidateError)]
	protected SCR_EAddonValidateError m_eId;

	[Attribute("", UIWidgets.EditBox, "The Error message for the error id")]
	protected string m_sText;

	//------------------------------------------------------------------------------------------------
	SCR_EAddonValidateError GetErrorCode()
	{
		return m_eId;
	}

	//------------------------------------------------------------------------------------------------
	string GetText()
	{
		return m_sText;
	}
}

[BaseContainerProps()]
class SCR_AddonLineValidateState
{
	[Attribute(SCR_EAddonValidateState.IDLE.ToString(), UIWidgets.ComboBox, string.Empty, string.Empty, enumType: SCR_EAddonValidateState)]
	protected SCR_EAddonValidateState m_eState;

	[Attribute("", UIWidgets.EditBox, "Text to display while in this state")]
	protected string m_sText;

	[Attribute(UIColors.GetColorAttribute(Color.Black), UIWidgets.ColorPicker)]
	protected ref Color m_Color;

	//------------------------------------------------------------------------------------------------
	SCR_EAddonValidateState GetState()
	{
		return m_eState;
	}

	//------------------------------------------------------------------------------------------------
	string GetText()
	{
		return m_sText;
	}

	//------------------------------------------------------------------------------------------------
	Color GetColor()
	{
		return m_Color;
	}
}

[BaseContainerProps(configRoot : true)]
class SCR_AddonLineValidateStates
{
	[Attribute("")]
	ref array<ref SCR_AddonLineValidateState> m_aStates;

	[Attribute("")]
	ref array<ref SCR_AddonLineValidateError> m_aErrors;
}
