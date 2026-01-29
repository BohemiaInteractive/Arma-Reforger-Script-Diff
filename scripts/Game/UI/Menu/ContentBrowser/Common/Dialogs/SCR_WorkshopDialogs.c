/*
Common dialogs in Workshop
// TODO: Cleanup and update looks
*/

class SCR_WorkshopDialogs
{
	static const ResourceName DIALOGS_CONFIG = "{26F075E5D30629E5}Configs/ContentBrowser/ContentBrowserDialogs.conf";
	static const string WIDGET_LIST = "AddonList";

	//------------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateDialog(string presetName)
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, presetName);
	}
}

class SCR_FailedModsDownloadDialog : SCR_ConfigurableDialogUi
{
	protected static const string FAILED_ADDON_LIST_DIALOG = "failed_dialogs_list";
	protected const string STR_AFFECTED_MODS = "#AR-Workshop_FailedModsDownload";
	protected const string FAILED_ADDON_FORMAT = "- %1 \n"; 
	
	protected const string MESSAGE_ERROR_UNKNOWN = "#AR-AddonValidate_Error_Unknown_UC";
	protected const string MESSAGE_ERROR_BUSY = "#AR-AddonValidate_Error_Busy_UC";
	protected const string MESSAGE_ERROR_AUTH = "#AR-AddonValidate_Error_NoInternet_UC";
	protected const string MESSAGE_ERROR_STORAGE = "#AR-AddonValidate_Error_AvailableSpace_UC";
	
	protected static ref SCR_FailedModsDownloadDialog m_Instance;
	
	protected ref array<SCR_WorkshopItemActionDownload> m_aDisplayedActions = {};
	
	//------------------------------------------------------------------------------------------
	static void ShowFailedModsDialog(SCR_WorkshopItemActionDownload action, int reason)
	{
		// Add failed addons to dialog 
		if (m_Instance)
		{
			m_Instance.UpdateFailedModsDialogContent(action, false);
			return;
		}

		string message;
		
		// Choose message dialog
		switch (reason)
		{
			default:
			case EBackendError.EBERR_UNKNOWN: 			message = MESSAGE_ERROR_UNKNOWN; break;	// Unknown
			case EBackendError.EBERR_BUSY: 				message = MESSAGE_ERROR_BUSY; break;	// Mod is busy doing something else
			case EBackendError.EBERR_AUTH_FAILED: 		message = MESSAGE_ERROR_AUTH; break;	// Cant login ?
			case EBackendError.EBERR_STORAGE_IS_FULL: 	message = MESSAGE_ERROR_STORAGE; break;	// No Available space
		}
		
		if (!message.IsEmpty())
			message += "\n";
		
		// Create dialog
		m_Instance = new SCR_FailedModsDownloadDialog();
		
		CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, FAILED_ADDON_LIST_DIALOG, m_Instance);
		m_Instance.SetMessageFormat(message, reason);
		m_Instance.UpdateFailedModsDialogContent(action, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMessageFormat(string text, int reason)
	{
		if (m_wMessage)
		{
			m_wMessage.SetVisible(true);
			m_wMessage.SetTextFormat(text, reason);
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	override void Close()
	{
		super.Close();
		
		m_Instance = null;
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void UpdateFailedModsDialogContent(SCR_WorkshopItemActionDownload action, bool initial)
	{
		if (!action)
			return;
		
		Widget contentWidget = GetContentLayoutRoot();
		if (!contentWidget)
			return;
		
		SCR_MessageDialogContent content = SCR_MessageDialogContent.Cast(contentWidget.FindHandler(SCR_MessageDialogContent));
		if (!content)
			return;
		
		if (m_aDisplayedActions.Contains(action))
			return;
		
		m_aDisplayedActions.Insert(action);
		
		if (initial)
			content.SetMessage(STR_AFFECTED_MODS + "\n\n" + string.Format(FAILED_ADDON_FORMAT, action.GetAddonName()));
		else
			content.SetMessage(content.GetMessage() + string.Format(FAILED_ADDON_FORMAT, action.GetAddonName()));
	}
}

//! There is not enough storage on your hard drive. The space required is at least %1.
class SCR_NotEnoughStorageDialog : SCR_ConfigurableDialogUi
{
	protected float m_fSizeBytes;
	
	protected static ref SCR_NotEnoughStorageDialog m_Instance;
	
	protected ref array<SCR_WorkshopItemActionDownload> m_aDisplayedActions = {};
	
	//------------------------------------------------------------------------------------------------
	static void ShowNotEnoughStorageDialog(float sizeBytes, SCR_WorkshopItemActionDownload action)
	{
		if (m_Instance)
		{
			m_Instance.AddToSize(sizeBytes, action);
			return;
		}
		
		m_Instance = new SCR_NotEnoughStorageDialog();
		
		CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "error_not_enough_storage", m_Instance);
		m_Instance.AddToSize(sizeBytes, action);
	}
	
	//-----------------------------------------------------------------------------------------------
	override void Close()
	{
		super.Close();
		
		m_Instance = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Increase size displayed in dialog
	void AddToSize(float sizeBytes, SCR_WorkshopItemActionDownload action)
	{
		if (!action)
			return;
		
		if (m_aDisplayedActions.Contains(action))
			return;
		
		m_aDisplayedActions.Insert(action);
		
		m_fSizeBytes += sizeBytes;
		
		string sizeStr = SCR_ByteFormat.GetReadableSize(m_fSizeBytes);
		string messageStr = WidgetManager.Translate(m_DialogPreset.m_sMessage, sizeStr);
		
		SetMessage(messageStr);
	}
}
