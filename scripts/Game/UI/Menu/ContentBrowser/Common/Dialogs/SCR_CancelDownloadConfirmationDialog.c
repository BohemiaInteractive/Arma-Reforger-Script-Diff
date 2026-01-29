//! Dialog to cancel downloads
class SCR_CancelDownloadConfirmationDialog : SCR_ConfigurableDialogUi
{
	ref array<ref SCR_WorkshopItemActionDownload> m_aActions;

	protected const ResourceName DOWNLOAD_LINE_LAYOUT = "{1C5D2CC10D7A1BC3}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManager_AddonDownloadLineNonInteractive.layout";

	//------------------------------------------------------------------------------------------------
	void SCR_CancelDownloadConfirmationDialog(array<ref SCR_WorkshopItemActionDownload> actionsToCancel)
	{
		m_aActions = actionsToCancel;
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, "cancel_download_confirmation", this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		VerticalLayoutWidget layout = VerticalLayoutWidget.Cast(GetContentLayoutRoot().FindAnyWidget(SCR_WorkshopDialogs.WIDGET_LIST));

		// Create widgets for downloads
		foreach (SCR_WorkshopItemActionDownload action : m_aActions)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(DOWNLOAD_LINE_LAYOUT, layout);
			SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
			comp.InitForCancelDownloadAction(action);
		}

		super.OnMenuOpen(preset);
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		foreach (SCR_WorkshopItemActionDownload action : m_aActions)
		{
			action.Cancel();
		}

		super.OnConfirm();
	}
}