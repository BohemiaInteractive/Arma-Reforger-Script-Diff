class SCR_ModReportDialogComponent : SCR_ScriptedWidgetComponent
{
	// Const localized strings 
	protected const string MSG_SELECT = "#AR-Workshop_SelectMsg";
	protected const string LINE_DOWNLOADED = "#AR-Workshop_Details_Downloaded";
	protected const string MSG_REPORT_SUCCESS = "#AR-Workshop_Dialog_Report_SuccessDescription";
	protected const string MSG_UNBLOCK_AUTHOR_CONFIRM = "#AR-Workshop_CancelAuthorReport";
	protected const string TAG_AUTHOR_UNBLOCK = "unblock_author_simple";
	
	protected const int AUTHOR_MOD_LIMIT = 50;
	
	// Attributes
	[Attribute("{ADEA32EB841E8629}Configs/ContentBrowser/ContentBrowserReportDialogs.conf", UIWidgets.ResourceNamePicker, "Layout of the navigation button", params: "layout")]
	protected ResourceName m_sDialogsConfig;
	
	[Attribute("select_report")]
	protected string m_sTagSelectReport;
	
	[Attribute("report_author")]
	protected string m_sTagReportAuthor;
	
	// Variables 
	protected ref SCR_ConfigurableDialogUi m_CurrentDialog;
	protected ReportDialogUI m_ReportDialog;
	
	protected ref SCR_WorkshopItem m_Item;
	protected WorkshopAuthor m_Author;
	
	protected ref SCR_ContentBrowser_GetAssetListParams m_Params;
	
	protected ref array<ref SCR_ConfigurableDialogUi> m_aDialogs = {};
	protected SCR_LoadingOverlayDialog m_LoadingOverlayDlg;

	protected ref array <ref SCR_WorkshopItem> m_aAuthorModsList = {};
	
	protected ref SCR_WorkshopItemActionAddAuthorBlock m_ActionAddAuthorBlock;
	protected ref SCR_WorkshopItemActionRemoveAuthorBlock m_ActionRemoveAuthorBlock;
	
	protected ref ScriptInvokerVoid m_OnItemReportSuccessDialogClose;
	
	//------------------------------------------------------------------------------------------------
	void OpenSelectReport(notnull SCR_WorkshopItem item)
	{
		m_CurrentDialog = SCR_ConfigurableDialogUi.CreateFromPreset(m_sDialogsConfig, m_sTagSelectReport);
		if (!m_CurrentDialog)
			return;
		
		m_aDialogs.Insert(m_CurrentDialog);
		
		m_Item = item;
		m_Author = item.GetWorkshopItem().Author();
		
		// Message
		string author = m_Item.GetAuthorName();
		m_CurrentDialog.GetMessageWidget().SetTextFormat(MSG_SELECT, author);
		m_CurrentDialog.GetMessageWidget().SetVisible(true);
		
		// Actions 
		m_CurrentDialog.m_OnConfirm.Insert(OnSelectReportConfirm);
		
		// Author report action
		SCR_InputButtonComponent butAuthor = m_CurrentDialog.FindButton("report_author");
		if (butAuthor)
			butAuthor.m_OnActivated.Insert(OnSelectReportAuthor);
	}
	
	//------------------------------------------------------------------------------------------------
	void OpenSelectReportAuthor(notnull SCR_WorkshopItem item)
	{
		m_Item = item;
		m_Author = item.GetWorkshopItem().Author();
		
		if (!m_Author.IsBlocked())
		{
			// Block author
			m_LoadingOverlayDlg = SCR_LoadingOverlayDialog.Create();
			OpenReportAuthorModList();
			
			return;
		}

		// Show cancel report block
		OpenRemoveAuthorBlockModList();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSelectReportConfirm(SCR_ConfigurableDialogUi dialog)
	{
		OpenReportThis();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSelectReportAuthor()
	{	
		OpenSelectReportAuthor(m_Item);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Dialog for selecting type, typing message and confirming this mod report
	protected void OpenReportThis()
	{
		m_ReportDialog = ReportDialogUI.Cast(GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.ReportItemDialog));
		if (!m_ReportDialog)
			return;
		
		m_ReportDialog.SetWorkshopItem(m_Item);
		
		// Invoker actions 
		m_ReportDialog.GetOnReportSuccess().Insert(OnItemReportedSuccessfully);
		m_ReportDialog.m_OnCancel.Insert(OnCancelThisReport);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemReportedSuccessfully(bool isAuthorBlocked)
	{
		SCR_ConfigurableDialogUi dlg = SCR_WorkshopDialogs.CreateDialog("report_success");
		dlg.m_OnClose.Insert(OnItemReportSuccessDialogClose);

		// Message
		string msg;
		
		if (!isAuthorBlocked)
			msg = MSG_REPORT_SUCCESS + "\n\n" ;
		
		msg += "#AR-Workshop_ReportModReverse";
		dlg.GetMessageWidget().SetTextFormat(msg, "#AR-Workshop_Filter_Reported", "#AR-Workshop_CancelReport");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnItemReportSuccessDialogClose()
	{
		if (m_OnItemReportSuccessDialogClose)
			m_OnItemReportSuccessDialogClose.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelThisReport()
	{
		if (m_ReportDialog)
			m_ReportDialog.m_OnCancel.Remove(OnCancelThisReport);
		
		OpenSelectReport(m_Item);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Dialog displaying all author mmods and explaing what will happen after report confirm
	protected void OpenReportAuthorModList()
	{
		string author = m_Item.GetAuthorName();
		
		OpenAuthorModsDialog();
		
		// Setup message
		string msg = "#AR-Workshop_ReportAuthorMsg";
		SCR_AddonListDialog addonDialog = SCR_AddonListDialog.Cast(m_CurrentDialog);
		
		// Show affected mods only if there are any mods to be displayed 
		if (addonDialog && !addonDialog.GetDonwloadLines().IsEmpty())
			msg += "\n\n#AR-Workshop_AffectedMods";
		
		m_CurrentDialog.GetMessageWidget().SetTextFormat(msg, author);
		
		// Actions 
		m_CurrentDialog.m_OnConfirm.Insert(OnConfirmReportAuthorModList);
		m_CurrentDialog.m_OnCancel.Insert(OnCancelReportAuthorModList);
		m_LoadingOverlayDlg.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Dialog displaying all author mmods and explaing what will happen after report confirm
	protected void OpenRemoveAuthorBlockModList()
	{				
		m_CurrentDialog = SCR_ConfigurableDialogUi.CreateFromPreset(m_sDialogsConfig, TAG_AUTHOR_UNBLOCK);
		
		// Add author to message
		string msg = MSG_UNBLOCK_AUTHOR_CONFIRM;
		
		string author = m_Item.GetAuthorName();
		m_CurrentDialog.GetMessageWidget().SetTextFormat(msg, author);
		
		// Actions 
		m_CurrentDialog.m_OnConfirm.Insert(OnConfirmRemoveAuthorBlock);
		m_CurrentDialog.m_OnCancel.Insert(OnCancelRemoveAuthorReport);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OpenAuthorModsDialog(bool showReported = true)
	{
		// Create dialog list
		array<WorkshopItem> toReport = {};
		if (m_Author.IsBlocked())
			m_Author.GetPageItems(toReport);
		else
			m_Author.GetOfflineItems(toReport);

		m_aAuthorModsList.Clear();
	
		SCR_WorkshopItem scrItem;
		foreach (WorkshopItem item : toReport)
		{
			scrItem = SCR_AddonManager.GetInstance().Register(item);
			m_aAuthorModsList.Insert(scrItem);
		}
		
		SCR_AddonListDialog dialog = SCR_AddonListDialog.CreateItemsList(m_aAuthorModsList, m_sTagReportAuthor, m_sDialogsConfig);
		m_CurrentDialog = dialog;
		if (!dialog)
			return;
		
		m_aDialogs.Insert(m_CurrentDialog);
		
		// Show message
		m_CurrentDialog.GetMessageWidget().SetVisible(true);
		
		// Mark mod entries 
		array<SCR_DownloadManager_AddonDownloadLine> lines = dialog.GetDonwloadLines();
	
		SCR_WorkshopItem item;
		foreach (SCR_DownloadManager_AddonDownloadLine line : lines)
		{
			item = line.GetItem();
			
			// Owned 
			if (item.GetOffline())
				line.DisplayError(SCR_WorkshopUiCommon.DOWNLOAD_STATE_COMPLETED);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnConfirmReportAuthorModList()
	{
		// Block 
		m_ActionAddAuthorBlock = m_Item.AddAuthorBlock();
		
		m_ActionAddAuthorBlock.m_OnCompleted.Insert(OnAuthorReportSuccess);
		m_ActionAddAuthorBlock.m_OnFailed.Insert(OnAuthorReportFail);
		
		if (!m_ActionAddAuthorBlock.Activate() && !m_ActionAddAuthorBlock.Reactivate())
			return;
		
		OnCancelReportAuthorModList();
		m_LoadingOverlayDlg = SCR_LoadingOverlayDialog.Create();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnConfirmRemoveAuthorBlock()
	{
		m_ActionRemoveAuthorBlock = m_Item.RemoveAuthorBlock();
		
		m_ActionRemoveAuthorBlock.m_OnCompleted.Insert(OnRemoveAuthorBlockSuccess);
		m_ActionRemoveAuthorBlock.m_OnFailed.Insert(OnAuthorReportFail);
		
		if (!m_ActionRemoveAuthorBlock.Activate() && !m_ActionRemoveAuthorBlock.Reactivate())
			return;
		
		OnCancelReportAuthorModList();
		m_LoadingOverlayDlg = SCR_LoadingOverlayDialog.Create();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAuthorReportFail()
	{
		CloseDialogs();
		m_LoadingOverlayDlg.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAuthorReportSuccess()
	{
		OnItemReportedSuccessfully(true);
		
		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		
		// Clear up all author items loading and remove from local storage
		foreach (SCR_WorkshopItem item : m_aAuthorModsList)
		{
			item.DeleteLocally();
			item.SetSubscribed(false);
			
			// Cancel download 
			if (!mgr)
				continue;
			
			SCR_WorkshopItemActionDownload action = mgr.GetActionOfItem(item);
			if (action)
				action.Cancel();
		}
		
		CloseDialogs();
		m_LoadingOverlayDlg.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CloseDialogs()
	{
		foreach (SCR_ConfigurableDialogUi dialog : m_aDialogs)
		{
			dialog.Close();
		}
		
		m_aDialogs.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelReportAuthorModList()
	{
		m_CurrentDialog.m_OnConfirm.Remove(OnConfirmReportAuthorModList);
		m_CurrentDialog.m_OnCancel.Remove(OnCancelReportAuthorModList);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRemoveAuthorBlockSuccess()
	{
		//m_CurrentDialog.Close();
		m_LoadingOverlayDlg.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelRemoveAuthorReport()
	{
		m_CurrentDialog.m_OnConfirm.Remove(OnRemoveAuthorBlockSuccess);
		m_CurrentDialog.m_OnCancel.Remove(OnCancelRemoveAuthorReport);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelAuthorReport()
	{
		m_ReportDialog.m_OnCancel.Remove(OnCancelAuthorReport);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnItemReportSuccessDialogClose()
	{
		if (!m_OnItemReportSuccessDialogClose)
			m_OnItemReportSuccessDialogClose = new ScriptInvokerVoid();
		
		return m_OnItemReportSuccessDialogClose;
	}
}
