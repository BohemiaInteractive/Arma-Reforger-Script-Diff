//------------------------------------------------------------------------------------------------
//! Show list of reported mods and provide option to cancel reports
class SCR_ReportedAddonsDialog : SCR_AddonListDialog
{
	protected const ResourceName LINE_LAYOUT = "{6D73127FB096229D}UI/layouts/Menus/ContentBrowser/DownloadManager/ReportedAddonEntry.layout";
	protected const ResourceName DIALOGS_CONFIG = "{ADEA32EB841E8629}Configs/ContentBrowser/ContentBrowserReportDialogs.conf";
	
	protected const string AUTHOR_REPORT_TAG = "report_author";
	protected const string AUTHOR_UNBLOCK_TAG = "unblock_author_simple";
	
	protected const string MSG_AUTHOR = "#AR-Workshop_FilterCategory_Author";
	protected const string STR_FIXED_REPORT = "#AR-Workshop_Dialog_Success";
	protected const string UNBLOCK_AUTHOR_MESSAGE = "#AR-Workshop_CancelAuthorReport";
	
	protected const string WIDGET_DETAIL = "TxtDetailType";
	protected const string WIDGET_MESSAGE = "TxtDetailMessage";
	
	protected const string BUTTON_CANCEL_REPORT = "cancelReport";
	
	protected const int AUTHOR_MOD_LIMIT = 20;
	
	protected ref ScriptInvoker<SCR_ReportedAddonsDialog> Event_OnAllReportsCanceled;
	protected bool m_bBlocked;
	
	// Widgets 
	protected TextWidget m_wTxtType;
	protected TextWidget m_wTxtMessage;
	protected SCR_InputButtonComponent m_NavCancelReport;
	protected Widget m_wList;
	
	protected SCR_LoadingOverlayDialog m_LoadingOvelay;
	protected ref SCR_CancelMyReportDialog m_DialogCancelReport;
	
	//protected SCR_WorkshopItem m_ItemFocused;
	protected SCR_ModularButtonComponent m_LineFocused;
	
	//protected ref array<SCR_ModularButtonComponent> m_aButtons = new array<SCR_ModularButtonComponent>();
	
	protected ref map<SCR_ModularButtonComponent, SCR_WorkshopItem> m_mButtonsItems;
	protected ref map<SCR_ModularButtonComponent, WorkshopAuthor> m_mButtonsAuthors;
	
	protected WorkshopAuthor m_LoadingAuthor;
	protected ref BackendCallback m_ReportCallback = new BackendCallback();
	
	//------------------------------------------------------------------------------------------------
	// Override API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		m_mButtonsItems = new map<SCR_ModularButtonComponent, SCR_WorkshopItem>();
		m_mButtonsAuthors = new map<SCR_ModularButtonComponent, WorkshopAuthor>();
		
		m_wList = VerticalLayoutWidget.Cast(GetRootWidget().FindAnyWidget("AddonList"));
		
		// Find widgets 
		m_wTxtType = TextWidget.Cast(GetRootWidget().FindAnyWidget(WIDGET_DETAIL));
		m_wTxtMessage = TextWidget.Cast(GetRootWidget().FindAnyWidget(WIDGET_MESSAGE));
		
		SetupReports();
		
		FocusTopLine();
		
		SCR_InputButtonComponent button = FindButton(BUTTON_CANCEL_REPORT);
		if (button)
		{
			button.m_OnActivated.Clear();
			button.m_OnActivated.Insert(OnCancelReport);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check report state and setup entries data
	//! Search for reported author
	protected void SetupReports()
	{
		ref array<WorkshopAuthor> blockedAuthors = {};
		
		foreach (SCR_WorkshopItem item : m_aItems)
		{
			// Are there blocked/banned addons by serice?
			if (item.GetBlocked())
			{
				CreateAddonEntry(item);
				m_bBlocked = true;
				continue;
			}
			
			// Author blocks 
			if (item.GetModAuthorReportedByMe())
			{
				// Save reported author
				WorkshopAuthor author = item.GetWorkshopItem().Author();
				
				if (!blockedAuthors.Contains(author))
				{
					// Add author entry
					blockedAuthors.Insert(author);
					CreateAuthorEntry(author);
				}
			}
			
			// Report by player 
			if (item.GetReportedByMe())
				CreateAddonEntry(item);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnConfirm()
	{
		OnCancelReportActived();
		m_OnConfirm.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	// Protected API
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Create entry with specific reported author
	protected SCR_ModularButtonComponent CreateEntry(string name, string message)
	{
		Widget entry = GetGame().GetWorkspace().CreateWidgets(LINE_LAYOUT, m_wList);
		
		SCR_SimpleEntryComponent comp = SCR_SimpleEntryComponent.Cast(entry.FindHandler(SCR_SimpleEntryComponent));
		comp.SetMessages(name, message);
		
		// Find button comp
		SCR_ModularButtonComponent button = SCR_ModularButtonComponent.Cast(entry.FindHandler(SCR_ModularButtonComponent));
		if (!button)
			return null;
		
		// Callback
		button.m_OnFocus.Insert(OnAddonLineFocus);

		return button;
	}
	
	//------------------------------------------------------------------------------------------------
	// Addon report
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Create addon entry with message based on type
	protected void CreateAddonEntry(notnull SCR_WorkshopItem item)
	{
		// Msg
		string msg = SCR_WorkshopUiCommon.GetRestrictedAddonStateText(item);

		// Line
		SCR_ModularButtonComponent button = CreateEntry(item.GetName(), msg);
		m_mButtonsItems.Insert(button, item);
		
		// Callbacks
		button.m_OnDoubleClicked.Insert(OnAddonEntrySelected);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAddonEntrySelected(SCR_ModularButtonComponent button)
	{
		SCR_WorkshopItem item;
		m_mButtonsItems.Find(button, item);
		
		if (!item)
			Print("Item wasn't found or set!");
		
		// Load
		item.m_OnMyReportLoaded.Insert(OnAddonReportLoadSuccess);
		item.m_OnError.Insert(OnAddonReportLoadError);
		item.m_OnTimeout.Insert(OnAddonReportLoadError);
		item.LoadReport();
		
		// Show load UI
		m_LoadingOvelay = SCR_LoadingOverlayDialog.Create();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAddonReportLoadSuccess(SCR_WorkshopItem item)
	{
		// Cleanup
		item.m_OnMyReportLoaded.Remove(OnAddonReportLoadSuccess);
		item.m_OnError.Remove(OnAddonReportLoadError);
		item.m_OnTimeout.Remove(OnAddonReportLoadError);
		
		m_LoadingOvelay.Close();
		
		// Setup dialog and callbacks
		m_DialogCancelReport = new SCR_CancelMyReportDialog(item);
			
		if (m_DialogCancelReport)
		{
			m_DialogCancelReport.m_OnConfirm.Insert(OnCancelReportConfirm);
			m_DialogCancelReport.m_OnClose.Insert(OnCancelReportDialog);
			m_DialogCancelReport.m_OnClose.Insert(OnReportDialogClose);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAddonReportLoadError(SCR_WorkshopItem item)
	{
		// Cleanup
		item.m_OnMyReportLoaded.Remove(OnAddonReportLoadSuccess);
		item.m_OnError.Remove(OnAddonReportLoadError);
		item.m_OnTimeout.Remove(OnAddonReportLoadError);
		
		m_LoadingOvelay.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	// Author report
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Create entry with reported author
	protected void CreateAuthorEntry(notnull WorkshopAuthor author)
	{
		SCR_ModularButtonComponent button = CreateEntry(author.Name(), MSG_AUTHOR);
		m_mButtonsAuthors.Insert(button, author);
		
		// Callbacks
		button.m_OnDoubleClicked.Insert(OnAuthorEntrySelected);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAuthorEntrySelected(SCR_ModularButtonComponent button)
	{		
		m_mButtonsAuthors.Find(button, m_LoadingAuthor);
			
		if (!m_LoadingAuthor)
			Print("Author wasn't found or set!");
		
		// Request author addons - TODO: restore when fix is ready
		/*
		PageParams params = new PageParams();
		params.limit = AUTHOR_MOD_LIMIT;
		params.offset = 0;
		
		m_ReportCallback.SetOnSuccess(OnAuthorReportLoadResponse);
		m_ReportCallback.SetOnError(OnAuthorReportLoadError);
		
		m_LoadingAuthor.RequestPage(m_ReportCallback, params, false);
		
		// Show load UI
		m_LoadingOvelay = SCR_LoadingOverlayDialog.Create();
		*/
		
		// Show confirm list 
		AuthorBlockCancelDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelReport()
	{
	 	if (m_mButtonsItems.Contains(m_LineFocused))
		{
			OnAddonEntrySelected(m_LineFocused);
		}
		else
		{
			OnAuthorEntrySelected(m_LineFocused);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AuthorBlockCancelDialog()
	{
		SCR_ConfigurableDialogUi dialog = SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, AUTHOR_UNBLOCK_TAG);
		
		dialog.GetMessageWidget().SetTextFormat(UNBLOCK_AUTHOR_MESSAGE, m_LoadingAuthor.Name());
		
		dialog.m_OnConfirm.Insert(OnConfirmRemoveAuthorBlock);
		dialog.m_OnCancel.Insert(OnCancelRemoveAuthorReport);
		dialog.m_OnClose.Insert(OnReportDialogClose);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAuthorReportLoadResponse()
	{
		m_LoadingOvelay.Close();
		OpenAuthorModsDialog(m_LoadingAuthor);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAuthorReportLoadError()
	{
		m_LoadingOvelay.Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OpenAuthorModsDialog(WorkshopAuthor author)
	{
		// Get items
		array<WorkshopItem> items = {};
		author.GetPageItems(items);
		
		//Set items list 
		array<ref SCR_WorkshopItem> scrItems;
		SCR_AddonManager addonManager = SCR_AddonManager.GetInstance();
		
		foreach (WorkshopItem item : items)
		{
			SCR_WorkshopItem scrItem = addonManager.Register(item);
			scrItems.Insert(scrItem);
		}
		
		SCR_AddonListDialog dialog = SCR_AddonListDialog.CreateItemsList(scrItems, AUTHOR_REPORT_TAG, DIALOGS_CONFIG);
		
		// Set message 
		dialog.GetMessageWidget().SetTextFormat(
			"#AR-Workshop_CancelAuthorReport" + "\n\n" + "#AR-Workshop_AffectedMods", m_LoadingAuthor.Name());
		
		// Callbacks 
		dialog.m_OnConfirm.Insert(OnConfirmRemoveAuthorBlock);
		dialog.m_OnCancel.Insert(OnCancelRemoveAuthorReport);
		dialog.m_OnClose.Insert(OnReportDialogClose);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnConfirmRemoveAuthorBlock(SCR_ConfigurableDialogUi dialog)
	{
		m_ReportCallback.SetOnSuccess(OnRemoveAuthorBlockResponse);
		m_ReportCallback.SetOnSuccess(OnRemoveAuthorBlockError);
		
		m_LoadingAuthor.RemoveBlock(m_ReportCallback);
		
		OnCancelRemoveAuthorReport(dialog);
		m_LoadingOvelay = SCR_LoadingOverlayDialog.Create();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRemoveAuthorBlockResponse()
	{
		m_LoadingOvelay.Close();
		
		SCR_ModularButtonComponent button = SCR_MapHelper<SCR_ModularButtonComponent, WorkshopAuthor>.GetKeyByValue(m_mButtonsAuthors, m_LoadingAuthor);
		DisplayReportCancelSuccess(button);
		
		// Remove item 
		m_mButtonsAuthors.Remove(button);
		
		AllButtonsCanceled();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRemoveAuthorBlockError()
	{
		m_LoadingOvelay.Close();
		SCR_CommonDialogs.CreateRequestErrorDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this on closing any report dialog to set repot list focus 
	protected void OnReportDialogClose(SCR_ConfigurableDialogUi dialog)
	{
		dialog.m_OnClose.Remove(OnReportDialogClose);
		GetGame().GetCallqueue().CallLater(FocusTopLine);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisplayReportCancelSuccess(notnull SCR_ModularButtonComponent button)
	{
		// Show success in ui
		SCR_SimpleEntryComponent entry = SCR_SimpleEntryComponent.Cast(button.GetRootWidget().FindHandler(SCR_SimpleEntryComponent));
		
		if (entry)
		{
			entry.GetMessage().SetColor(Color.FromInt(UIColors.CONFIRM.PackToInt()));
			entry.SetMessages(entry.GetLeftText(), STR_FIXED_REPORT);
		}
		
		// Clearup button
		button.m_OnFocus.Remove(OnAddonLineFocus);
		button.m_OnDoubleClicked.Remove(OnAddonEntrySelected);
		button.GetRootWidget().SetFlags(WidgetFlags.NOFOCUS);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AllButtonsCanceled()
	{
		if (m_mButtonsItems.IsEmpty() && m_mButtonsAuthors.IsEmpty())
		{
			Close();
			InvokeOnAllReportsCanceled();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelRemoveAuthorReport(SCR_ConfigurableDialogUi dialog)
	{
		dialog.m_OnConfirm.Remove(OnConfirmRemoveAuthorBlock);
		dialog.m_OnCancel.Remove(OnCancelRemoveAuthorReport);
	}
	
	//------------------------------------------------------------------------------------------------
	// Callbacks
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Show details of addon line report when focused
	protected void OnAddonLineFocus(SCR_ModularButtonComponent button)
	{
		Widget w = button.GetRootWidget();
		SCR_ModularButtonComponent line = SCR_ModularButtonComponent.Cast(w.FindHandler(SCR_ModularButtonComponent));
		
		if (line)
			m_LineFocused = line;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Start to canceling report of currently focused mod
	protected void OnCancelReportActived()
	{
		if (!m_LineFocused)
			return;
		
		if (m_mButtonsAuthors.Contains(m_LineFocused))
		{
			OnAuthorEntrySelected(m_LineFocused);
			return;
		}
		
		OnAddonEntrySelected(m_LineFocused);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelReportConfirm(SCR_ConfigurableDialogUi dialog)
	{
		m_DialogCancelReport.GetWorkshopItemAction().m_OnCompleted.Insert(OnCancelReportSuccess);
		m_DialogCancelReport.GetWorkshopItemAction().m_OnFailed.Insert(OnCancelReportFail);
		
		// Clear confirm 
		m_DialogCancelReport.m_OnConfirm.Remove(OnCancelReportConfirm);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelReportSuccess(SCR_WorkshopItemActionCancelReport action)
	{	
		SCR_ModularButtonComponent button = SCR_MapHelper<SCR_ModularButtonComponent, SCR_WorkshopItem>.GetKeyByValue(m_mButtonsItems, action.GetWorkshopItem());
		DisplayReportCancelSuccess(button);
		
		// Remove item 
		m_mButtonsItems.Remove(button);
		
		AllButtonsCanceled();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelReportFail(SCR_WorkshopItemActionCancelReport action)
	{
		// Remove invoker actions 
		action.m_OnCompleted.Remove(OnCancelReportSuccess);
		action.m_OnFailed.Remove(OnCancelReportFail);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCancelReportDialog(SCR_ConfigurableDialogUi dialog)
	{
		// Clear invokers 
		m_DialogCancelReport.m_OnConfirm.Remove(OnCancelReportConfirm);
		m_DialogCancelReport.m_OnCancel.Remove(OnCancelReportDialog);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void FocusTopLine()
	{
		// Addons
		foreach (SCR_ModularButtonComponent button, SCR_WorkshopItem item : m_mButtonsItems)
		{
			if (button.GetRootWidget().IsFocusable())
			{
				GetGame().GetWorkspace().SetFocusedWidget(button.GetRootWidget());
				return;
			}	
		}
		
		// Authors
		foreach (SCR_ModularButtonComponent button, WorkshopAuthor author : m_mButtonsAuthors)
		{
			if (button.GetRootWidget().IsFocusable())
			{
				GetGame().GetWorkspace().SetFocusedWidget(button.GetRootWidget());
				return;
			}	
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Invokers 
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeOnAllReportsCanceled()
	{
		if (!Event_OnAllReportsCanceled)
			Event_OnAllReportsCanceled = new ScriptInvoker();
		
		Event_OnAllReportsCanceled.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAllReportsCanceled()
	{
		if (!Event_OnAllReportsCanceled)
			Event_OnAllReportsCanceled = new ScriptInvoker();
		
		return Event_OnAllReportsCanceled;
	}
	
};