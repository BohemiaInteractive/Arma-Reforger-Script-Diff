/*
Common functions and variables related to Workshop UI.
*/

class SCR_WorkshopUiCommon
{
	static const float IMAGE_SIZE_RATIO = 16.0 / 9.0;
	static const string PRIMARY_ACTION_MESSAGE_DECORATION = "[%1]";
	static const int CONTINUOUS_UPDATE_DELAY = 1000;
	static const int MAX_DEPENDENCIES_SHOWN = 999;
	static const int MAX_VOTES_SHOWN = 99999;
	
	// Hacky big number of scenarios
	static const int PAGE_SCENARIOS = 4096;

	// Map addon tags to icon names
	static const ResourceName ADDON_TYPE_FILTER_CATEGORY_CONFIG = "{A557E41062372854}Configs/ContentBrowser/Filters/category_type.conf";
	static const ResourceName ADDON_MANW_FILTER_CATEGORY_CONFIG = "{FB69283C67BD9E67}Configs/ContentBrowser/Filters/category_manw.conf";
	static ref map<string, string> s_sAddonTagImageMap = new map<string, string>();

	// Map addon tags to default pictures
	static ref map<string, ResourceName> s_sAddonTagDefaultThumbnailMap = new map<string, ResourceName>();

	// --- THUMBNAILS ---
	static const ResourceName ADDON_DEFAULT_THUMBNAIL = 		"{04EB797EBF59CDEF}UI/Textures/Workshop/AddonThumbnails/workshop_defaultFallback_UI.edds";
	static const ResourceName ADDON_DEFAULT_THUMBNAIL_LOADING = "{75455009AFED376B}UI/Textures/Workshop/AddonThumbnails/workshop_loading_UI.edds";
	static const ResourceName SCENARIO_SP_DEFAULT_THUMBNAIL = 	"{17D65C6D78C7722C}UI/Textures/Workshop/AddonThumbnails/workshop_scenarios_UI.edds";
	static const ResourceName SCENARIO_MP_DEFAULT_THUMBNAIL = 	"{62A03BAAAED612E8}UI/Textures/Workshop/AddonThumbnails/workshop_mpScenarios_UI.edds";
	
	// --- MESSAGES ---
	static const string	MESSAGE_DEPENDENCIES_MISSING = 	"#AR-Workshop_State_MissingDependencies";
	static const string	MESSAGE_DEPENDENCIES_DISABLED = "#AR-Workshop_State_MustEnableDependencies";
	static const string	MESSAGE_DEPENDENCIES_OUTDATED = "#AR-Workshop_State_DependencyUpdateAvailable";
	static const string	MESSAGE_UPDATE_AVAILABLE = 		"#AR-Workshop_State_UpdateAvailable";
	static const string MESSAGE_REPORTED = 				"#AR-Workshop_State_Reported";
	static const string MESSAGE_BANNED =				"#AR-Workshop_State_Banned";
	static const string MESSAGE_CORRUPTED =				"#AR-Workshop_State_Corrupted";
	static const string MESSAGE_RESTRICTED =			"#AR-Workshop_State_Restricted";
	static const string MESSAGE_RESTRICTED_GENERIC = 	"#AR-Workshop_Dialog_Error_ModIsBlocked";
	static const string MESSAGE_FALLBACK = 				"#AR-CoreMenus_Tooltips_Unavailable";

	// --- LABELS ---
	static const string LABEL_DOWNLOAD = 				"#AR-Workshop_ButtonDownload";
	static const string LABEL_UPDATE = 					"#AR-Workshop_ButtonUpdate";
	static const string LABEL_CANCEL = 					"#AR-Workshop_ButtonCancel";
	static const string LABEL_ENABLE = 					"#AR-Workshop_ButtonEnable";
	static const string LABEL_DISABLE = 				"#AR-Workshop_ButtonDisable";
	static const string LABEL_DELETE =					"#AR-Workshop_ButtonDelete";
	static const string LABEL_DEPENDENCIES_DOWNLOAD = 	"#AR-Workshop_ButtonDownloadDependencies";
	static const string LABEL_DEPENDENCIES_ENABLE = 	"#AR-Workshop_ButtonEnableDependencies";
	static const string LABEL_DEPENDENCIES_UPDATE =		"#AR-Workshop_ButtonUpdateDependencies";
	static const string LABEL_ADDON_DISABLED =			"#AR-Workshop_State_Disabled";
	static const string LABEL_ADDON_MISSING =			"#AR-ServerBrowser_JoinMissingModsTittle";
	static const string LABEL_REPORT =					"#AR-Workshop_ButtonReport";
	static const string LABEL_CANCEL_REPORT =			"#AR-Workshop_CancelReportTitle";
	static const string LABEL_DEPENDENCIES_NUMBER_ONE =	"#AR-Workshop_Details_ModDependencies_Short_One_LC";
	static const string LABEL_DEPENDENCIES_NUMBER =		"#AR-Workshop_Details_ModDependencies_Short_LC";
	static const string LABEL_DEPENDENT_NUMBER_ONE =	"#AR-Workshop_Details_DependentMods_Short_One_LC_Downloaded";
	static const string LABEL_DEPENDENT_NUMBER =		"#AR-Workshop_Details_DependentMods_Short_LC_Downloaded";
	static const string LABEL_IP_ADDRESS =				"#AR-ServerBrowser_IP";
	
	// --- ICONS ---
	static const string ICON_DOWNLOAD = 		"download";
	static const string ICON_UPDATE = 			"update";
	static const string ICON_CANCEL_DOWNLOAD =	"cancel";
	static const string ICON_REPAIR = 			"repairCircle";
	static const string ICON_DEPENDENCIES = 	"dependencies";
	static const string ICON_REPORTED =			"reportedByMe";
	static const string ICON_ENABLED = 			"okCircle";
	static const string ICON_DISABLED = 		"cancelCircle";
	static const string ICON_LINKED =			"linked";
	static const string ICON_VERSION_MATCH =	"version_match";
	static const string ICON_VERSION_MISMATCH =	"version_mismatch";
	
	// --- REVISION AVAILABILITY ---
	// Short messages
	static const string	MESSAGE_MOD_NOT_AVAILABLE =					"#AR-Workshop_State_NotAvailable"; // Fallback
	static const string MESSAGE_MOD_NOT_AVAILABLE_REMOVED =			"#AR-Workshop_State_Removed";
	static const string MESSAGE_MOD_NOT_AVAILABLE_INCOMPATIBLE =	"#AR-Workshop_State_Incompatible";
	static const string MESSAGE_MOD_AVAILABLE_WHEN_UPDATED = 		"#AR-Workshop_State_UpdateRequired";
	static const string MESSAGE_MOD_DOWNLOAD_NOT_FINISHED = 		"#AR-Workshop_State_DownloadNotFinished";

	// Verbose messages
	static const string	MESSAGE_VERBOSE_MOD_NOT_AVAILABLE =					"#AR-Workshop_State_NotAvailableDesc"; // Fallback
	static const string MESSAGE_VERBOSE_MOD_NOT_AVAILABLE_REMOVED =			"#AR-Workshop_State_RemovedDesc";
	static const string MESSAGE_VERBOSE_MOD_NOT_AVAILABLE_INCOMPATIBLE =	"#AR-Workshop_State_IncompatibleDesc";
	static const string MESSAGE_VERBOSE_MOD_AVAILABLE_WHEN_UPDATED = 		"#AR-Workshop_State_UpdateRequiredDesc";
	static const string MESSAGE_VERBOSE_MOD_DOWNLOAD_NOT_FINISHED = 		"#AR-Workshop_State_DownloadNotFinishedDesc";

	// Icons
	static const string	ICON_MOD_NOT_AVAILABLE =				"not-available"; // Fallback
	static const string ICON_MOD_NOT_AVAILABLE_REMOVED =		"cancelCircle";	// TODO: specific icon?
	static const string ICON_MOD_NOT_AVAILABLE_INCOMPATIBLE =	"incompatible";
	static const string ICON_MOD_AVAILABLE_WHEN_UPDATED = 		"available-when-updated";
	static const string ICON_MOD_DOWNLOAD_NOT_FINISHED = 		"downloading"; // TODO: specific icon?

	// --- DOWNLOAD STATES ---
	static const string DOWNLOAD_STATE_DOWNLOADING = 	"#AR-DownloadManager_State_Downloading";
	static const string DOWNLOAD_STATE_PROCESSING = 	"#AR-DownloadManager_State_Processing";
	static const string DOWNLOAD_STATE_PAUSED = 		"#AR-Workshop_ButtonPause";
	static const string DOWNLOAD_STATE_CANCELED = 		"#AR-Workshop_Canceled";
	static const string DOWNLOAD_STATE_DOWNLOAD_FAIL = 	"#AR-Workshop_DownloadFail";
	static const string DOWNLOAD_STATE_COMPLETED = 		"#AR-Workshop_Details_Downloaded";
	
	// State icons
	static const string DOWNLOAD_STATE_ICON_DOWNLOADING = 	"downloading";
	static const string DOWNLOAD_STATE_ICON_PROCESSING = 	"update";
	static const string DOWNLOAD_STATE_ICON_PAUSED = 		"download-pause";
	
	// --- LINKS ---
	static const string LINK_NAME_LICENSES = "Link_Bohemia";
	
	// --- INIT ---
	//------------------------------------------------------------------------------------------------
	//! Call this somewhere once, preferably at start of the game.
	static void OnGameStart()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		mgr.m_OnAddonsChecked.Remove(SCR_WorkshopUiCommon.Callback_OnAddonsChecked);
		mgr.m_OnAddonsChecked.Insert(SCR_WorkshopUiCommon.Callback_OnAddonsChecked);

		InitAddonTagMaps(ADDON_TYPE_FILTER_CATEGORY_CONFIG);
		InitAddonTagMaps(ADDON_MANW_FILTER_CATEGORY_CONFIG);
	}

	//------------------------------------------------------------------------------------------------
	//! Tries to open Workshop UI, if not possible due to privileges, negotiates them.
	static void TryOpenWorkshop()
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		if (mgr.GetUgcPrivilege())
			ContentBrowserUI.Create();
		else
		{
			mgr.m_OnUgcPrivilegeResult.Insert(OnTryOpenWorkshopUgcPrivilegeResult);
			mgr.NegotiateUgcPrivilegeAsync();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnTryOpenWorkshopUgcPrivilegeResult(bool result)
	{
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		mgr.m_OnUgcPrivilegeResult.Remove(OnTryOpenWorkshopUgcPrivilegeResult);
		if (result)
			ContentBrowserUI.Create();
		else 
			SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "mp_you_dont_have_the_right");
	}

	// --- ADDON INTERACTIONS ---
	//------------------------------------------------------------------------------------------------
	//! Toggles addon enabled state. Must be called from non-toggleable buttons.
	//! Might show confirmation dialog.
	static void OnEnableAddonButton(SCR_WorkshopItem item)
	{
		if (item && (!item.GetOffline() || item.GetRestricted()))
			return;

		SetAddonEnabled(item, !item.GetEnabled());
	}

	//------------------------------------------------------------------------------------------------
	//! Sets addon enabled state from a toggleable button.
	//! Might show confirmation dialog.
	static void OnEnableAddonToggleButton(SCR_WorkshopItem item, SCR_ModularButtonComponent buttonComp)
	{
		if (item && (!item.GetOffline() || item.GetRestricted()))
			return;

		// Button was clicked and it is now toggled or not, and this directly sets addon state
		SetAddonEnabled(item, buttonComp.GetToggled());
	}

	//------------------------------------------------------------------------------------------------
	//! Shows a confirmation dialog when unsubscribing an addon.
	static void OnDeleteAddonButton(SCR_WorkshopItem item)
	{
		// Check if some addons depend on this
		array<ref SCR_WorkshopItem> offlineAndDependent = GetDownloadedDependentAddons(item);

		if (!offlineAndDependent.IsEmpty())
		{
			// Some addons depend on this, show a special confirmation
			SCR_DeleteAddonDependentAddonsDetected.Create(offlineAndDependent, item);
			return;
		}

		// Show a standard confirmation for addon deletion
		SCR_DeleteAddonDialog.CreateDeleteAddon(item);
	}

	//------------------------------------------------------------------------------------------------
	//! Performs the primary suggested action
	//! You must store the dlRequest somewhere, in case that the download will be started.
	static void ExecutePrimaryAction(notnull SCR_WorkshopItem item, out SCR_WorkshopDownloadSequence dlRequest, Revision revision = null)
	{
		// --- Cancel current downloads ---
		if (IsDownloadingAddonOrDependencies(item))
		{
			// Aggregate all actions of this item and create a dialog to confirm disabling all current downloads
			SCR_WorkshopItemActionDownload download = item.GetDownloadAction();
			SCR_WorkshopItemActionComposite dependenciesDownload = item.GetDependencyCompositeAction();

			array<ref SCR_WorkshopItemActionDownload> downloadsToCancel = {};

			if (download)
				downloadsToCancel.Insert(download);

			if (dependenciesDownload)
			{
				array<ref SCR_WorkshopItemAction> dependencyItemActions = dependenciesDownload.GetActions();
				foreach (SCR_WorkshopItemAction action : dependencyItemActions)
				{
					SCR_WorkshopItemActionDownload downloadAction = SCR_WorkshopItemActionDownload.Cast(action);
					if (downloadAction)
						downloadsToCancel.Insert(downloadAction);
				}
			}

			// Create a confirmation dialog
			new SCR_CancelDownloadConfirmationDialog(downloadsToCancel);
			
			return;
		}
			
		// Setup Revision
		if (!revision)
			revision = item.GetLatestRevision();
		
		// --- Download ---
		if (!item.GetOffline())
		{
			dlRequest = SCR_WorkshopDownloadSequence.Create(item, revision, dlRequest);
			return;
		}
		
		// --- Fix issues ---
		switch (item.GetHighestPriorityProblem())
		{
			// Start download/update of whatever is possible to download or update
			case EWorkshopItemProblem.DEPENDENCY_MISSING:
			case EWorkshopItemProblem.UPDATE_AVAILABLE:
			case EWorkshopItemProblem.DEPENDENCY_OUTDATED:
			{
				dlRequest = SCR_WorkshopDownloadSequence.Create(item, revision, dlRequest);
				return;
			}
			
			case EWorkshopItemProblem.DEPENDENCY_DISABLED:
			{
				item.SetDependenciesEnabled(true);
				return;
			}
			
			//TODO: this does not take dependencies versions into account: what if a player wants a specific addon version to play a scenario he likes?
			// He has no way of downgrading the dependencies so they match the main addon, if there is a version difference we force him to update everything to latest.
			// This might cause the scenario he likes to become unavailable, if the author removed it from latest version
		}
	}

	//------------------------------------------------------------------------------------------------
	// NB: Download actions persist on canceled, failed or completed, as we might need to display their history
	static bool IsDownloadingAddonOrDependencies(SCR_WorkshopItem item)
	{
		if (!item)
			return false;
		
		return item.GetDownloadAction() || item.GetDependencyCompositeAction();
		
		//TODO: this might not be correct, as GetDownloadAction() will return true even when the action is completed or failed (for download manager history tab)
		//Download actions are an entirely scripted solution, so it might be safer to use DownloadableItem.GetDownloadingRevision() instead.
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns if the download is in progress (active or paused) but not finished, canceled or failed
	static bool IsDownloadingAddon(SCR_WorkshopItem item)
	{
		if (!item)
			return false;

		bool downloading;
		SCR_WorkshopItemActionDownload action = item.GetDownloadAction();
		if (action)
			downloading = action.IsActive() || action.IsPaused();
		
		return downloading;
		
		//TODO: 
		//This method is currently used to disable Delete buttons in workshop menus, as you can't delete data that's not there yet. 
		//This is however hacky, since we have DownloadableItem.GetDownloadingRevision() already
		//Check if that can be used instead
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_EAddonPrimaryActionState GetPrimaryActionState(notnull SCR_WorkshopItem item)
	{
		// TODO: unify with other Primary Action stuff in SCR_WorkshopUiCommon, as we're performing pretty much the exact same checks
		// Downloading is true if we are downloading anything for this addon or if we have started a download for any of its dependencies through this item
		if (IsDownloadingAddonOrDependencies(item))
			return SCR_EAddonPrimaryActionState.DOWNLOADING;
		
		EWorkshopItemProblem itemProblem = item.GetHighestPriorityProblem();
		if (itemProblem != EWorkshopItemProblem.NO_PROBLEM)
		{
			switch (itemProblem)
			{
				case EWorkshopItemProblem.UPDATE_AVAILABLE:		return SCR_EAddonPrimaryActionState.UPDATE;
				case EWorkshopItemProblem.DEPENDENCY_MISSING:	return SCR_EAddonPrimaryActionState.DEPENDENCIES_DOWNLOAD;
				case EWorkshopItemProblem.DEPENDENCY_OUTDATED:	return SCR_EAddonPrimaryActionState.DEPENDENCIES_UPDATE;
				case EWorkshopItemProblem.DEPENDENCY_DISABLED:	return SCR_EAddonPrimaryActionState.DEPENDENCIES_ENABLE;
			}
		}
		
		if (!item.GetOffline())
			return SCR_EAddonPrimaryActionState.DOWNLOAD;
		
		return SCR_EAddonPrimaryActionState.DOWNLOADED;
	}
	
	// --- ADDON VISUALS ---
	//------------------------------------------------------------------------------------------------
	//! Returns image associated with given Workshop tag
	static string GetTagImage(string tag)
	{
		tag.ToLower();
		return s_sAddonTagImageMap.Get(tag);
	}

	//------------------------------------------------------------------------------------------------
	static ResourceName GetDefaultAddonThumbnail(notnull SCR_WorkshopItem item)
	{
		array<WorkshopTag> tagObjs = {};

		WorkshopItem internalItem = item.GetWorkshopItem();

		if (!internalItem || !s_sAddonTagDefaultThumbnailMap)
			return ADDON_DEFAULT_THUMBNAIL;

		// Iterate all tags, return first found default iamge
		internalItem.GetTags(tagObjs);
		foreach (WorkshopTag tagObj : tagObjs)
		{
			string tagStr = tagObj.Name();
			tagStr.ToLower();

			if (s_sAddonTagDefaultThumbnailMap.Contains(tagStr))
				return s_sAddonTagDefaultThumbnailMap.Get(tagStr);
		}

		return ADDON_DEFAULT_THUMBNAIL;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns name of suggested primary action
	static string GetPrimaryActionName(SCR_WorkshopItem item)
	{
		// Downloading
		if (IsDownloadingAddonOrDependencies(item))
			return LABEL_CANCEL;
		
		// Not downloaded
		if (!item.GetOffline())
			return LABEL_DOWNLOAD;

		// Issues to solve
		switch (item.GetHighestPriorityProblem())
		{
			case EWorkshopItemProblem.DEPENDENCY_MISSING:	return LABEL_DEPENDENCIES_DOWNLOAD;
			case EWorkshopItemProblem.DEPENDENCY_DISABLED:	return LABEL_DEPENDENCIES_ENABLE;
			case EWorkshopItemProblem.UPDATE_AVAILABLE:		return LABEL_UPDATE;
			case EWorkshopItemProblem.DEPENDENCY_OUTDATED:	return LABEL_DEPENDENCIES_UPDATE;
		}
		
		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	static void GetPrimaryActionLook(SCR_EAddonPrimaryActionState state, SCR_ERevisionAvailability availability, SCR_WorkshopItem item, out string icon, out Color iconColor, out string message, out Color messageColor)
	{
		if (!item)
			return;
		
		switch (state)
		{
			case SCR_EAddonPrimaryActionState.DOWNLOAD:
			{
				icon = ICON_DOWNLOAD;
				iconColor = Color.FromInt(UIColors.IDLE_ACTIVE.PackToInt());
				
				message = string.Format(PRIMARY_ACTION_MESSAGE_DECORATION, SCR_ByteFormat.GetReadableSize(item.GetSizeBytes())); //TODO: storage size
				messageColor = Color.FromInt(UIColors.IDLE_ACTIVE.PackToInt());
				break;
			}

			case SCR_EAddonPrimaryActionState.DOWNLOADING:
			{
				icon = DOWNLOAD_STATE_ICON_DOWNLOADING;
				iconColor = Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt());

				string percentage = UIConstants.FormatUnitPercentage(GetDownloadProgressPercentage(SCR_DownloadManager.GetItemDownloadActionsProgress(item)));
				message = WidgetManager.Translate("%1 %2", DOWNLOAD_STATE_DOWNLOADING, percentage);
				messageColor = Color.FromInt(UIColors.NEUTRAL_INFORMATION.PackToInt());
				break;
			}

			case SCR_EAddonPrimaryActionState.UPDATE:
			{
				if (availability != SCR_ERevisionAvailability.ERA_COMPATIBLE_UPDATE_AVAILABLE)
				{
					iconColor = Color.FromInt(UIColors.SLIGHT_WARNING.PackToInt());
					messageColor = Color.FromInt(UIColors.IDLE_ACTIVE.PackToInt());
				}
				else
				{
					iconColor = Color.FromInt(UIColors.WARNING.PackToInt());
					messageColor = Color.FromInt(UIColors.WARNING.PackToInt());
				}

				icon = ICON_UPDATE;
				//TODO: update size
				message = string.Format(PRIMARY_ACTION_MESSAGE_DECORATION, GetPrimaryActionName(item));
				break;
			}

			case SCR_EAddonPrimaryActionState.DEPENDENCIES_UPDATE:
			{
				icon = ICON_UPDATE;
				iconColor = Color.FromInt(UIColors.SLIGHT_WARNING.PackToInt());
				
				message = string.Format(PRIMARY_ACTION_MESSAGE_DECORATION, GetPrimaryActionName(item));
				messageColor = Color.FromInt(UIColors.IDLE_ACTIVE.PackToInt());
				break;
			}

			case SCR_EAddonPrimaryActionState.DEPENDENCIES_DOWNLOAD:
			{
				icon = ICON_DOWNLOAD;
				iconColor = Color.FromInt(UIColors.SLIGHT_WARNING.PackToInt());

				message = string.Format(PRIMARY_ACTION_MESSAGE_DECORATION, GetPrimaryActionName(item));
				messageColor = Color.FromInt(UIColors.IDLE_ACTIVE.PackToInt());
				break;
			}

			case SCR_EAddonPrimaryActionState.DEPENDENCIES_ENABLE:
			{
				icon = ICON_REPAIR;
				iconColor = Color.FromInt(UIColors.SLIGHT_WARNING.PackToInt());

				message = string.Format(PRIMARY_ACTION_MESSAGE_DECORATION, GetPrimaryActionName(item));
				messageColor = Color.FromInt(UIColors.IDLE_ACTIVE.PackToInt());
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	static string GetPrimaryActionTooltipMessage(SCR_EAddonPrimaryActionState state, SCR_WorkshopItem item)
	{
		string message = MESSAGE_FALLBACK;
		
		switch (state)
		{
			case SCR_EAddonPrimaryActionState.DOWNLOAD:
			{
				message = LABEL_DOWNLOAD;
				break;
			}
			
			case SCR_EAddonPrimaryActionState.DOWNLOADING:
			{
				message = LABEL_CANCEL;
				break;
			}
		
			case SCR_EAddonPrimaryActionState.UPDATE:
			case SCR_EAddonPrimaryActionState.DEPENDENCIES_UPDATE:
			case SCR_EAddonPrimaryActionState.DEPENDENCIES_DOWNLOAD:
			case SCR_EAddonPrimaryActionState.DEPENDENCIES_ENABLE:
			{
				if (item)
					message = GetPrimaryActionName(item);
				
				break;
			}
		}
		
		return message;
	}
	
	//------------------------------------------------------------------------------------------------
	static void GetVersionDisplayLook(SCR_WorkshopItem item, out bool showUpdateText, out string currentVersionIcon, out Color currentVersionIconColor, out string currentVersionText, out Color currentVersionTextColor, out string updateVersionText)
	{
		if (!item)
			return;
		
		Revision revisionCurrent = item.GetCurrentLocalRevision();
		Revision revisionLatest = item.GetLatestRevision();
		bool needsUpdate = revisionCurrent && revisionLatest && !Revision.AreEqual(revisionCurrent, revisionLatest);
		
		// Show update text
		showUpdateText = needsUpdate;
		
		// Version text
		if (needsUpdate)
			currentVersionTextColor = UIColors.NEUTRAL_ACTIVE_STANDBY;
		else
			currentVersionTextColor = UIColors.NEUTRAL_INFORMATION;

		string revision;
		if (revisionCurrent && item.GetOffline())
			revision = revisionCurrent.GetVersion();
		else if (revisionLatest)
			revision = revisionLatest.GetVersion();
		
		currentVersionText = UIConstants.FormatVersion(revision);
		
		if (revisionLatest)
			updateVersionText = UIConstants.FormatVersion(revisionLatest.GetVersion());
		
		// Version Icon
		if (!item.GetOffline())
		{
			currentVersionIconColor = Color.FromInt(UIColors.NEUTRAL_INFORMATION.PackToInt());
			currentVersionIcon = ICON_DOWNLOAD;
		}
		else if (needsUpdate)
		{
			currentVersionIconColor = Color.FromInt(UIColors.SLIGHT_WARNING.PackToInt());
			currentVersionIcon = ICON_UPDATE;
		}
		else
		{
			currentVersionIconColor = Color.FromInt(UIColors.NEUTRAL_INFORMATION.PackToInt());
			currentVersionIcon = ICON_VERSION_MATCH;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns string such as "Banned" or "Reported" for a restricted item. Otherwise returns empty string.
	static string GetRestrictedAddonStateText(SCR_WorkshopItem item)
	{
		if (!item.GetRestricted())
			return string.Empty;

		if (item.GetReportedByMe())
			return MESSAGE_REPORTED;
		else if (item.GetBlocked())
			return MESSAGE_BANNED;
		else
			return MESSAGE_RESTRICTED;
	}

	//------------------------------------------------------------------------------------------------
	//! Converts report type enum to stringtable entry
	static string GetReportTypeString(EWorkshopReportType eReportType)
	{
		switch (eReportType)
		{
			case EWorkshopReportType.EWREPORT_INAPPROPRIATE_CONTENT: 	return "#AR-Workshop_Report_InappropiateContent";
			case EWorkshopReportType.EWREPORT_OFFENSIVE_LANGUAGE: 		return "#AR-Workshop_Report_OffensiveLanguage";
			case EWorkshopReportType.EWREPORT_MISLEADING: 				return "#AR-Workshop_Report_Missleading_NonFunctional";
			case EWorkshopReportType.EWREPORT_SPAM: 					return "#AR-Workshop_Report_Spam";
			case EWorkshopReportType.EWREPORT_SCAM: 					return "#AR-Workshop_Report_Scam";
			case EWorkshopReportType.EWREPORT_MALICIOUS: 				return "#AR-Workshop_Report_Malicious";
			case EWorkshopReportType.EWREPORT_INTELLECTUAL_PROPERTY:	return "#AR-Workshop_Report_IPInfringement";
			case EWorkshopReportType.EWREPORT_OTHER: 					return "#AR-Workshop_Report_Other";
		}
		
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	static void UpdateEnableAddonToggleButton(SCR_ModularButtonComponent button, SCR_WorkshopItem item, SCR_EAddonPrimaryActionState state, bool alwaysShowWhenDownloaded = true)
	{
		if (!item)
		{
			button.SetVisible(false);
			return;
		}
		
		bool show = state == SCR_EAddonPrimaryActionState.DOWNLOADED;		
		if (alwaysShowWhenDownloaded)
			show = item.GetOffline();
		
		/*WorldSaveItem save = WorldSaveItem.Cast(item.GetWorkshopItem());
		if (save && show)
			show = !SCR_SaveWorkshopManager.IsSaveLocalOnly(save);*/
		
		button.SetVisible(show);
		button.SetToggled(item.GetEnabled(), false);

		if (!item.GetOffline())
			return;

		button.SetEnabled(state != SCR_EAddonPrimaryActionState.DOWNLOADING && !(!item.GetEnabled() && state == SCR_EAddonPrimaryActionState.DEPENDENCIES_DOWNLOAD));

		string enableButtonMode = "no_problems";
		if (state == SCR_EAddonPrimaryActionState.DEPENDENCIES_DOWNLOAD || state == SCR_EAddonPrimaryActionState.DEPENDENCIES_ENABLE)
			enableButtonMode = "problems";

		button.SetEffectsWithAnyTagEnabled({"all", enableButtonMode});
	}
	
	//------------------------------------------------------------------------------------------------
	static array<ref SCR_WorkshopItem> GetDownloadedDependentAddons(notnull SCR_WorkshopItem item)
	{
		return SCR_AddonManager.SelectItemsAnd(item.GetDependentAddons(), EWorkshopItemQuery.OFFLINE);
	}
	
	// --- UNIFIED ROUNDING ---
	//------------------------------------------------------------------------------------------------
	static int GetDownloadProgressPercentage(float progress)
	{
		return Math.Floor(100.0 * progress);
	}

	//------------------------------------------------------------------------------------------------
	static int GetRatingPercentage(float rating)
	{
		return Math.Ceil(100.0 * rating);
	}
	
	//------------------------------------------------------------------------------------------------
	static int GetUpvotes(notnull SCR_WorkshopItem item)
	{
		return Math.Clamp(Math.Ceil(item.GetAverageRating() * item.GetRatingCount()), 0, MAX_VOTES_SHOWN);
	}
	
	//------------------------------------------------------------------------------------------------
	static int GetDownvotes(notnull SCR_WorkshopItem item)
	{
		return Math.Clamp(Math.Ceil(item.GetRatingCount() - (item.GetAverageRating() * item.GetRatingCount())), 0, MAX_VOTES_SHOWN);
	}

	// --- REVISION ERROR MESSAGES ---
	//------------------------------------------------------------------------------------------------
	static string GetRevisionAvailabilityErrorMessage(WorkshopItem item)
	{
		if (!item)
			return MESSAGE_MOD_NOT_AVAILABLE;

		return GetRevisionAvailabilityErrorMessage(SCR_AddonManager.ItemAvailability(item));
	}

	//------------------------------------------------------------------------------------------------
	static string GetRevisionAvailabilityErrorMessage(SCR_ERevisionAvailability state)
	{
		switch (state)
		{
			case SCR_ERevisionAvailability.ERA_AVAILABLE:					return string.Empty;
			case SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY:		return MESSAGE_MOD_NOT_AVAILABLE;
			case SCR_ERevisionAvailability.ERA_DELETED:						return MESSAGE_MOD_NOT_AVAILABLE_REMOVED;
			case SCR_ERevisionAvailability.ERA_OBSOLETE:					return MESSAGE_MOD_NOT_AVAILABLE_INCOMPATIBLE;
			case SCR_ERevisionAvailability.ERA_COMPATIBLE_UPDATE_AVAILABLE:	return MESSAGE_MOD_AVAILABLE_WHEN_UPDATED;
			case SCR_ERevisionAvailability.ERA_DOWNLOAD_NOT_FINISHED:		return MESSAGE_MOD_DOWNLOAD_NOT_FINISHED;
		}

		return MESSAGE_MOD_NOT_AVAILABLE;
	}

	//------------------------------------------------------------------------------------------------
	static string GetRevisionAvailabilityErrorMessageVerbose(WorkshopItem item)
	{
		if (!item)
			return MESSAGE_VERBOSE_MOD_NOT_AVAILABLE;

		return GetRevisionAvailabilityErrorMessageVerbose(SCR_AddonManager.ItemAvailability(item));
	}

	//------------------------------------------------------------------------------------------------
	static string GetRevisionAvailabilityErrorMessageVerbose(SCR_ERevisionAvailability state)
	{
		switch (state)
		{
			case SCR_ERevisionAvailability.ERA_AVAILABLE:					return string.Empty;
			case SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY:		return MESSAGE_VERBOSE_MOD_NOT_AVAILABLE;
			case SCR_ERevisionAvailability.ERA_DELETED:						return MESSAGE_VERBOSE_MOD_NOT_AVAILABLE_REMOVED;
			case SCR_ERevisionAvailability.ERA_OBSOLETE:					return MESSAGE_VERBOSE_MOD_NOT_AVAILABLE_INCOMPATIBLE;
			case SCR_ERevisionAvailability.ERA_COMPATIBLE_UPDATE_AVAILABLE:	return MESSAGE_VERBOSE_MOD_AVAILABLE_WHEN_UPDATED;
			case SCR_ERevisionAvailability.ERA_DOWNLOAD_NOT_FINISHED:		return MESSAGE_VERBOSE_MOD_DOWNLOAD_NOT_FINISHED;
		}

		return MESSAGE_VERBOSE_MOD_NOT_AVAILABLE;
	}

	//------------------------------------------------------------------------------------------------
	static string GetRevisionAvailabilityErrorTexture(WorkshopItem item)
	{
		if (!item)
			return ICON_MOD_NOT_AVAILABLE;

		return GetRevisionAvailabilityErrorTexture(SCR_AddonManager.ItemAvailability(item));
	}

	//------------------------------------------------------------------------------------------------
	static string GetRevisionAvailabilityErrorTexture(SCR_ERevisionAvailability state)
	{
		switch (state)
		{
			case SCR_ERevisionAvailability.ERA_AVAILABLE:					return string.Empty;
			case SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY:		return ICON_MOD_NOT_AVAILABLE;
			case SCR_ERevisionAvailability.ERA_DELETED:						return ICON_MOD_NOT_AVAILABLE_REMOVED;
			case SCR_ERevisionAvailability.ERA_OBSOLETE:					return ICON_MOD_NOT_AVAILABLE_INCOMPATIBLE;
			case SCR_ERevisionAvailability.ERA_COMPATIBLE_UPDATE_AVAILABLE:	return ICON_MOD_AVAILABLE_WHEN_UPDATED;
			case SCR_ERevisionAvailability.ERA_DOWNLOAD_NOT_FINISHED:		return ICON_MOD_DOWNLOAD_NOT_FINISHED;
		}

		return ICON_MOD_NOT_AVAILABLE;
	}

	// --- PROTECTED ---
	//------------------------------------------------------------------------------------------------
	//! Callback from addon manager when addons are checked
	protected static void Callback_OnAddonsChecked()
	{
		array<string> bannedAddons = {};
		GetGame().GetBackendApi().GetWorkshop().GetBannedItems(bannedAddons);

		if (!bannedAddons.IsEmpty())
			new SCR_BannedAddonsDetectedDialog(bannedAddons);
	}

	//------------------------------------------------------------------------------------------------
	//! Internal method to handle addon enabling. Might show a dialog or enable some other addons(dependnecies) if needed.
	protected static void SetAddonEnabled(SCR_WorkshopItem item, bool newEnabled)
	{
		if (!item)
			return;
		
		item.SetEnabled(newEnabled);

		if (newEnabled)
		{
			// Auto enable dependencies too
			item.SetDependenciesEnabled(true);
			return;
		}

		// Check if some addons depend on this
		array<ref SCR_WorkshopItem> offlineAndEnabled = SCR_AddonManager.SelectItemsAnd(
			item.GetDependentAddons(),
			EWorkshopItemQuery.OFFLINE | EWorkshopItemQuery.ENABLED);

		if (!offlineAndEnabled.IsEmpty())
			SCR_DisableDependentAddonsDialog.CreateDisableDependentAddons(offlineAndEnabled, item);
	}

	//------------------------------------------------------------------------------------------------
	protected static void InitAddonTagMaps(ResourceName configPath)
	{
		// Load addon type config
		SCR_FilterCategory cat = SCR_ConfigHelperT<SCR_FilterCategory>.GetConfigObject(configPath);
		if (!cat)
			return;

		foreach (SCR_FilterEntry filter : cat.GetFilters())
		{
			string internalName = filter.m_sInternalName;
			internalName.ToLower();
			s_sAddonTagImageMap.Insert(internalName, filter.m_sImageName);

			SCR_ContentBrowserFilterTag filterTag = SCR_ContentBrowserFilterTag.Cast(filter);
			if (filterTag)
			{
				if (!filterTag.m_sDefaultThumbnail.IsEmpty())
					s_sAddonTagDefaultThumbnailMap.Insert(internalName, filterTag.m_sDefaultThumbnail);
			}
		}
	}
}

enum SCR_EAddonPrimaryActionState
{
	DOWNLOAD,
	UPDATE,
	DEPENDENCIES_UPDATE,
	DEPENDENCIES_DOWNLOAD,	// Missing
	DEPENDENCIES_ENABLE,	// Disabled
	DOWNLOADING,
	DOWNLOADED
}