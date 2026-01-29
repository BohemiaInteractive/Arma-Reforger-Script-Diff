//! Dialog which sends a request to delete user's report.
class SCR_CancelMyReportDialog : SCR_ConfigurableDialogUi
{
	protected TextWidget m_wTxtTypeMsg;
	protected TextWidget m_wTxtComment;

	protected ref SCR_WorkshopItem m_Item;
	protected bool m_bAuthorReport;
	protected ref SCR_WorkshopItemActionCancelReport m_Action;
	protected SCR_LoadingOverlayDialog m_LoadingOverlayDlg;

	//------------------------------------------------------------------------------------------------
	void SCR_CancelMyReportDialog(SCR_WorkshopItem item, bool authorReport = false)
	{
		m_Item = item;
		m_bAuthorReport = authorReport;
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, "cancel_report", this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		// Get reports
		string reportDescription;
		EWorkshopReportType reportType;

		if (!m_bAuthorReport)
			m_Item.GetReport(reportType, reportDescription);

		string reportTypeStr = SCR_WorkshopUiCommon.GetReportTypeString(reportType);

		// Find widgets
		m_wTxtTypeMsg = TextWidget.Cast(m_wRoot.FindAnyWidget("TxtTypeMsg"));
		m_wTxtComment = TextWidget.Cast(m_wRoot.FindAnyWidget("TxtComment"));

		// Display report details
		if (!m_bAuthorReport)
			SetMessage("#AR-Workshop_CancelReportDescription");

		m_wTxtTypeMsg.SetText(reportTypeStr);
		m_wTxtComment.SetText(reportDescription);
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		m_LoadingOverlayDlg = SCR_LoadingOverlayDialog.Create();

		m_Action = m_Item.CancelReport();
		m_Action.m_OnCompleted.Insert(Callback_OnSuccess);
		m_Action.m_OnFailed.Insert(Callback_OnFailed);
		m_Action.Activate();

		m_OnConfirm.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	void Callback_OnSuccess()
	{
		m_LoadingOverlayDlg.CloseAnimated();
		Close();
	}

	//------------------------------------------------------------------------------------------------
	void Callback_OnFailed()
	{
		m_LoadingOverlayDlg.CloseAnimated();
		Close();
	}

	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItemActionCancelReport GetWorkshopItemAction()
	{
		return m_Action;
	}
}