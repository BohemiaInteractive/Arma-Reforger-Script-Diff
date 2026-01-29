//! Shows a list of addons and some text.
// TODO: Cleanup and update looks
class SCR_AddonListDialog : SCR_ConfigurableDialogUi
{
	protected ref array<ref SCR_WorkshopItem> m_aItems = {};
	protected ref array<SCR_DownloadManager_AddonDownloadLine> m_aDownloadLines = {};

	protected const ResourceName ADDON_LINE_LAYOUT = "{BB5AEDDA3C4134FD}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManager_AddonDownloadLineConfirmation.layout";

	//------------------------------------------------------------------------------------------------
	void SCR_AddonListDialog(array<ref SCR_WorkshopItem> items, string preset)
	{
		m_aItems = items;

		if (!preset.IsEmpty())
			SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, preset, this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		VerticalLayoutWidget layout = VerticalLayoutWidget.Cast(GetContentLayoutRoot().FindAnyWidget(SCR_WorkshopDialogs.WIDGET_LIST));

		// Create widgets
		foreach (SCR_WorkshopItem item : m_aItems)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(ADDON_LINE_LAYOUT, layout);

			SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
			comp.InitForWorkshopItem(item, null, false);

			m_aDownloadLines.Insert(comp);
		}

		super.OnMenuOpen(preset);
	}

	//------------------------------------------------------------------------------------------------
	static SCR_AddonListDialog CreateItemsList(array<ref SCR_WorkshopItem> items, string preset, ResourceName dialogsConfig = string.Empty)
	{
		if (dialogsConfig == string.Empty)
			dialogsConfig = SCR_WorkshopDialogs.DIALOGS_CONFIG;

		SCR_AddonListDialog addonListDialog = new SCR_AddonListDialog(items, "");
		SCR_ConfigurableDialogUi.CreateFromPreset(dialogsConfig, preset, addonListDialog);
		return addonListDialog;
	}

	//------------------------------------------------------------------------------------------------
	//! Dialog when downloading restricted addons
	static SCR_AddonListDialog CreateRestrictedAddonsDownload(array<ref SCR_WorkshopItem> items)
	{
		return DisplayRestrictedAddonsList(items, "error_restricted_dependencies_download", "error_reported_dependencies_download");
	}

	//------------------------------------------------------------------------------------------------
	//! Dialog when trying to connect to a server with restricted addons
	static SCR_AddonListDialog CreateRestrictedAddonsJoinServer(array<ref SCR_WorkshopItem> items)
	{
		return DisplayRestrictedAddonsList(items, "error_restricted_addons_join_server", "error_reported_addons_join_server");
	}

	//------------------------------------------------------------------------------------------------
	//! Display dialogs for reported and blocked (banned) mods
	static SCR_AddonListDialog DisplayRestrictedAddonsList(array<ref SCR_WorkshopItem> items, string tagBlocked, string tagReported)
	{
		// Banned specific
		array<ref SCR_WorkshopItem> banned = SCR_AddonManager.SelectItemsOr(items, EWorkshopItemQuery.BLOCKED);

		if (!banned.IsEmpty())
			return new SCR_AddonListDialog(banned, tagBlocked);

		// Reported
		array<ref SCR_WorkshopItem> reported = SCR_AddonManager.SelectItemsOr(items, EWorkshopItemQuery.REPORTED_BY_ME | EWorkshopItemQuery.AUTHOR_BLOCKED);
		return new SCR_ReportedAddonsDialog(reported, tagReported);
	}

	//------------------------------------------------------------------------------------------------
	//! Dialog when failed to load game with selected mods
	static SCR_AddonListDialog CreateFailedToStartWithMods(array<ref SCR_WorkshopItem> items)
	{
		return new SCR_AddonListDialog(items, "error_failed_to_start_with_mods");
	}

	//------------------------------------------------------------------------------------------------
	array<SCR_DownloadManager_AddonDownloadLine> GetDonwloadLines()
	{
		return m_aDownloadLines;
	}
}

//! Dialog which is used when an addon is disabled but some dependent addons depend on it.
class SCR_DisableDependentAddonsDialog : SCR_AddonListDialog
{
	protected ref SCR_WorkshopItem m_Dependency;

	//------------------------------------------------------------------------------------------------
	void SCR_DisableDependentAddonsDialog(array<ref SCR_WorkshopItem> items, string preset, SCR_WorkshopItem dependency)
	{
		m_Dependency = dependency;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_DisableDependentAddonsDialog CreateDisableDependentAddons(array<ref SCR_WorkshopItem> items, SCR_WorkshopItem dependency)
	{
		return new SCR_DisableDependentAddonsDialog(items, "disable_dependent_addons", dependency);
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		// Disable the dependency too
		m_Dependency.SetEnabled(false);

		foreach (SCR_WorkshopItem item : m_aItems)
		{
			if (item.GetOffline())
				item.SetEnabled(false);
		}
		
		super.OnConfirm();
	}

	//------------------------------------------------------------------------------------------------
	override void OnCancel()
	{
		// Enable back the dependency
		m_Dependency.SetEnabled(true);

		super.OnCancel();
	}
}

//! This dialog is shown when we want to delete an addon but some other downloaded addons depend on it.
class SCR_DeleteAddonDependentAddonsDetected : SCR_AddonListDialog
{
	protected ref SCR_WorkshopItem m_Item;

	//------------------------------------------------------------------------------------------------
	static SCR_DeleteAddonDependentAddonsDetected Create(array<ref SCR_WorkshopItem> items, SCR_WorkshopItem item)
	{
		return new SCR_DeleteAddonDependentAddonsDetected(items, "delete_dependent_addons_detected", item);
	}

	//------------------------------------------------------------------------------------------------
	protected void SCR_DeleteAddonDependentAddonsDetected(array<ref SCR_WorkshopItem> items, string preset, SCR_WorkshopItem item)
	{
		m_Item = item;
	}

	//------------------------------------------------------------------------------------------------
	//! Delete the provided addon on confirmation. Disable dependent addons if they are enabled.
	override void OnConfirm()
	{
		if (m_Item.GetSubscribed())
			m_Item.SetSubscribed(false);

		m_Item.DeleteLocally();

		foreach (SCR_WorkshopItem item : m_aItems)
		{
			if (item.GetEnabled())
				item.SetEnabled(false);
		}

		super.OnConfirm();
	}
}
