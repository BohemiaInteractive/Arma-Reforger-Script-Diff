//! Dialog to cancel downloads during to server joining
class SCR_ServerJoinDownloadsConfirmationDialog : SCR_ConfigurableDialogUi
{
	protected static const string TAG_ALL = 		"server_download_all_cancel";
	protected static const string TAG_REQUIRED = 	"server_download_required";
	protected static const string TAG_UNRELATED = 	"server_download_unrelated_cancel";

	protected const ResourceName DOWNLOAD_LINE_LAYOUT = "{1C5D2CC10D7A1BC3}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManager_AddonDownloadLineNonInteractive.layout";
	
	protected const string BUTTON_RETRY = "Retry";

	protected Widget m_ListWrapper;
	
	protected ref array<ref SCR_WorkshopItemActionDownload> m_aActions = {};
	protected SCR_InputButtonComponent m_Retry;
	
	//------------------------------------------------------------------------------------------------
	static SCR_ServerJoinDownloadsConfirmationDialog Create(array<ref SCR_WorkshopItemActionDownload> actions, SCR_EJoinDownloadsConfirmationDialogType type)
	{
		string tag;

		switch (type)
		{
			case SCR_EJoinDownloadsConfirmationDialogType.ALL:
				tag = TAG_ALL;
				break;

			case SCR_EJoinDownloadsConfirmationDialogType.REQUIRED:
				tag = TAG_REQUIRED;
				break;

			case SCR_EJoinDownloadsConfirmationDialogType.UNRELATED:
				tag = TAG_UNRELATED;
				break;
		}

		SCR_ServerJoinDownloadsConfirmationDialog dialog = new SCR_ServerJoinDownloadsConfirmationDialog();
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, tag, dialog);
		dialog.Setup(actions);
		
		return dialog;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		m_ListWrapper = GetContentLayoutRoot().FindAnyWidget(SCR_WorkshopDialogs.WIDGET_LIST);

		super.OnMenuOpen(preset);
		
		// Retry button
		m_Retry = FindButton(BUTTON_RETRY);
		if (m_Retry)
			m_Retry.m_OnActivated.Insert(RetryActions);
	}
	
	//------------------------------------------------------------------------------------------------
	void Setup(array<ref SCR_WorkshopItemActionDownload> actions)
	{
		if (!m_ListWrapper)
			return;
		
		// --- Cleanup ---
		// Remove old widgets
		SCR_WidgetHelper.RemoveAllChildren(m_ListWrapper);
		
		// Release old actions
		foreach (SCR_WorkshopItemActionDownload action : actions)
		{
			action.m_OnActivated.Remove(UpdateRetryButton);
			action.m_OnFailed.Remove(UpdateRetryButton);
			action.m_OnCompleted.Remove(UpdateRetryButton);
		}
		
		// --- Initialize new ---
		m_aActions = actions;
	
		foreach (SCR_WorkshopItemActionDownload action : m_aActions)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(DOWNLOAD_LINE_LAYOUT, m_ListWrapper);
			SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
			comp.InitForServerDownloadAction(action);

			action.m_OnActivated.Insert(UpdateRetryButton);
			action.m_OnFailed.Insert(UpdateRetryButton);
			action.m_OnCompleted.Insert(UpdateRetryButton);
		}
		
		UpdateRetryButton();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateRetryButton()
	{
		if (!m_Retry)
			return;
		
		bool visible;
		
		foreach (SCR_WorkshopItemActionDownload action : m_aActions)
		{
			visible = action.IsFailed();
			if (visible)
				break;
		}
		
		m_Retry.SetVisible(visible, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RetryActions()
	{
		array<ref SCR_WorkshopItemActionDownload> newActions = {};
		
		foreach (SCR_WorkshopItemActionDownload action : m_aActions)
		{
			if (action.IsFailed())
				newActions.Insert(action.RetryDownload());
			else
				newActions.Insert(action);
		}
		
		Setup(newActions);
	}
}