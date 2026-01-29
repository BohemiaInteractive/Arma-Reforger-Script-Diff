/*
	Function library of scenario ui related methods
*/
class SCR_ScenarioUICommon
{
	// Button names
	static const string BUTTON_PLAY =			"Play";
	static const string BUTTON_CONTINUE =		"Continue";
	static const string BUTTON_RESTART =		"Restart";
	static const string BUTTON_HOST =			"Host";
	static const string BUTTON_FIND_SERVERS =	"FindServers";
	static const string BUTTON_FAVORITE =		"Favorite";
	static const string BUTTON_SCENARIOS =		"Scenarios";

	// Actions
	static const string ACTION_RESTART =		"MenuRestart";
	static const string ACTION_FIND_SERVERS =	"MenuJoin";
	static const string ACTION_HOST =			"MenuHost";
	static const string ACTION_SERVER_DETAILS =	"MenuServerDetails";
	
	// Dialogs
	static const string DIALOG_RESTART =		"scenario_restart";

	//------------------------------------------------------------------------------------------------
	//! Starts the mission or opens a confirmation dialog if it can't be immediately started
	static void TryPlayScenario(MissionWorkshopItem scenario)
	{
		if (!scenario)
			return;
		
		int nCompleted, nTotal;

		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr)
			mgr.GetDownloadQueueState(nCompleted, nTotal);

		if (nTotal > 0)
			SCR_StartScenarioWhileDownloadingDialog.CreateDialog(scenario);
		else
			scenario.Play();
	}

	//------------------------------------------------------------------------------------------------
	//! Hosts the scenario or opens a confirmation dialog if it can't be immediately started
	static void TryHostScenario(notnull MissionWorkshopItem scenario, notnull SCR_DSConfig config)
	{
		int nCompleted, nTotal;

		SCR_DownloadManager mgr = SCR_DownloadManager.GetInstance();
		if (mgr)
			mgr.GetDownloadQueueState(nCompleted, nTotal);

		if (nTotal > 0)
			SCR_HostScenarioWhileDownloadingDialog.CreateDialog(scenario, config);
		else
			scenario.Host(config);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool HasSave(MissionWorkshopItem mission)
	{
		if (!mission)
			return false;

		array<SaveGame> outSaveGames();
		return GetGame().GetSaveGameManager().GetSaves(outSaveGames, mission.Id()) > 0;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool LoadSave(MissionWorkshopItem scenario, SCR_MissionHeader header, ChimeraMenuPreset startMenu)
	{
		if (!SCR_ScenarioUICommon.CanPlay(scenario))
			return false;

		array<SaveGame> saves();		
		if (GetGame().GetSaveGameManager().GetSaves(saves, scenario.Id()) == 0)
			return false; // No saves available
		
		GetGame().GetSaveGameManager().Load(saves[saves.Count() - 1], false);
		SCR_MenuLoadingComponent.SaveLastMenu(startMenu);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool RestartScenario(MissionWorkshopItem scenario)
	{
		if (!SCR_ScenarioUICommon.CanPlay(scenario))
			return false;

		array<SaveGame> saves();		
		GetGame().GetSaveGameManager().GetSaves(saves, scenario.Id());

		Tuple2<int, MissionWorkshopItem> sharedCtx(saves.Count(), scenario);
		SaveGameOperationCb callback(OnRestartScenarioCleanupComplete, sharedCtx);
		foreach (SaveGame save : saves)
		{
			GetGame().GetSaveGameManager().Delete(save, callback);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static void OnRestartScenarioCleanupComplete(bool success, Managed context)
	{
		auto sharedCtx = Tuple2<int, MissionWorkshopItem>.Cast(context);
		if (!success || !sharedCtx.param2)
		{
			sharedCtx.param1 = -1; // Error, future invokes will never make it reach count 0
			return;
		}

		// Last delete of old data complete
		if (--sharedCtx.param1 == 0)
			TryPlayScenario(sharedCtx.param2);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsMultiplayer(MissionWorkshopItem mission)
	{
		return mission && mission.GetPlayerCount() > 1;
	}

	//------------------------------------------------------------------------------------------------
	// Default scenarios don't have a owner workshop item, so they're always ready. For the others we need to check
	static bool IsReady(MissionWorkshopItem mission)
	{
		if (!mission)
			return false;

		if (mission.GetOwner())
			return mission.GetOwner().IsReadyToRun();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsEnabled(MissionWorkshopItem mission)
	{
		if (!mission)
			return false;

		if (mission.GetOwner())
			return mission.GetOwner().IsEnabled();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsAddonDownloaded(MissionWorkshopItem mission)
	{
		if (!mission)
			return false;

		if (mission.GetOwner())
			return mission.GetOwner().GetActiveRevision();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsMissingDependencies(MissionWorkshopItem mission)
	{
		if (!mission)
			return true;

		// There are no dependencies if scenario is not from an addon
		if (!mission.GetOwner())
			return false;

		Revision revision = mission.GetOwner().GetActiveRevision();
		if (!revision)
			revision = mission.GetOwner().GetLocalRevision();
		
		if (!revision)
			return true;
		
		array<WorkshopItem> dependencies = {};
		revision.GetDependencies(dependencies);
		
		foreach (WorkshopItem dependency : dependencies)
		{	
			if (!dependency.GetActiveRevision())
				return true;
		}
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	static bool AreDependenciesAvailable(MissionWorkshopItem mission)
	{
		if (!mission)
			return false;

		// There are no dependencies if scenario is not from an addon
		if (!mission.GetOwner())
			return true;

		Revision revision = mission.GetOwner().GetActiveRevision();
		if (!revision)
			return false;
		
		array<WorkshopItem> dependencies = {};
		revision.GetDependencies(dependencies);
		
		foreach (WorkshopItem dependency : dependencies)
		{
			revision = dependency.GetLatestRevision();
			if (!revision || revision.GetAvailability() != ERevisionAvailability.ERA_AVAILABLE)
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ERevisionAvailability GetOwnerRevisionAvailability(MissionWorkshopItem mission)
	{
		if (!mission)
			return SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY;

		WorkshopItem owner = mission.GetOwner();
		SCR_ERevisionAvailability availability = SCR_ERevisionAvailability.ERA_AVAILABLE;

		if (owner)
			availability = SCR_AddonManager.ItemAvailability(owner);

		return availability;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsOwnerRestricted(MissionWorkshopItem mission)
	{
		if (!mission)
			return false;
		
		WorkshopItem owner = mission.GetOwner();
		if (!owner)
			return false;

		bool restricted;
		int accessState = owner.GetAccessState();
		
		WorkshopAuthor author = owner.Author();
		if (author)
			restricted = author.IsBlocked();
		
		return restricted || accessState & EWorkshopItemAccessState.EWASTATE_BLOCKED || accessState & EWorkshopItemAccessState.EWASTATE_REPORTED;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool CanPlay(MissionWorkshopItem mission)
	{
		return mission && GetPlayHighestPriorityIssue(mission) == SCR_EScenarioIssues.NONE;
	}

	//------------------------------------------------------------------------------------------------
	static bool CanJoin(MissionWorkshopItem mission)
	{
		return mission && GetJoinHighestPriorityIssue(mission) == SCR_EScenarioIssues.NONE;
	}

	//------------------------------------------------------------------------------------------------
	static bool CanHost(MissionWorkshopItem mission)
	{
		return mission && GetHostHighestPriorityIssue(mission) == SCR_EScenarioIssues.NONE;
	}

	//------------------------------------------------------------------------------------------------
	static MissionWorkshopItem GetInGameScenario(ResourceName resource)
	{
		return GetGame().GetBackendApi().GetWorkshop().GetInGameScenario(resource);
	}
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// The way these methods are used currently, these checks are repeated a myriad times and constantly. This is far from ideal, but was the quickest way to get it done
	
	// --- Issues ---
	// TODO: change the enum to flags to reduce the number of times these checks need to be performed
	/*
	Can Play, Continue, Restart when:
	- Addon is downloaded and enabled (this should also enable dependencies)
	- Dependencies are downloaded and match the main addon version
	*/
	//------------------------------------------------------------------------------------------------
	protected static SCR_EScenarioIssues GetPlayHighestPriorityIssue(MissionWorkshopItem mission)
	{
		if (!mission || GetGame().GetBackendApi().GetWorkshop().NeedAddonsScan())
			return SCR_EScenarioIssues.MISSION_MISSING;

		if (!IsAddonDownloaded(mission))
			return SCR_EScenarioIssues.ADDON_NOT_DOWNLOADED;

		if (IsMissingDependencies(mission))
			return SCR_EScenarioIssues.DEPENDENCIES_MISSING;

		if (!IsReady(mission))
			return SCR_EScenarioIssues.DEPENDENCIES_VERSION_MISMATCH;

		if (!IsEnabled(mission))
			return SCR_EScenarioIssues.ADDON_DISABLED;

		return SCR_EScenarioIssues.NONE;
	}

	/*
	Can Join when:
	- Is connected and multiplayer services are available
	*/
	//------------------------------------------------------------------------------------------------
	protected static SCR_EScenarioIssues GetJoinHighestPriorityIssue(MissionWorkshopItem mission)
	{
		if (!mission || GetGame().GetBackendApi().GetWorkshop().NeedAddonsScan())
			return SCR_EScenarioIssues.MISSION_MISSING;

		if (!SCR_ServicesStatusHelper.AreMultiplayerServicesAvailable())
			return SCR_EScenarioIssues.CONNECTION_ISSUE;

		return SCR_EScenarioIssues.NONE;
	}

	/*
	Can Host when:
	- Is connected and multiplayer services are available
	- Addon is available on Workshop, downloaded and enabled (this should also enable dependencies)
	- Dependencies are available on Workshop, downloaded and match the main addon version
	*/
	//------------------------------------------------------------------------------------------------
	protected static SCR_EScenarioIssues GetHostHighestPriorityIssue(MissionWorkshopItem mission)
	{
		if (!mission || GetGame().GetBackendApi().GetWorkshop().NeedAddonsScan())
			return SCR_EScenarioIssues.MISSION_MISSING;

		if (!SCR_ServicesStatusHelper.AreMultiplayerServicesAvailable())
			return SCR_EScenarioIssues.CONNECTION_ISSUE;

		if (!IsAddonDownloaded(mission))
			return SCR_EScenarioIssues.ADDON_NOT_DOWNLOADED;

		if (GetOwnerRevisionAvailability(mission) != SCR_ERevisionAvailability.ERA_AVAILABLE)
			return SCR_EScenarioIssues.ADDON_AVAILABILITY_ISSUE;

		if (IsMissingDependencies(mission))
			return SCR_EScenarioIssues.DEPENDENCIES_MISSING;

		if (!IsReady(mission))
			return SCR_EScenarioIssues.DEPENDENCIES_VERSION_MISMATCH;

		if (!AreDependenciesAvailable(mission))
			return SCR_EScenarioIssues.DEPENDENCIES_AVAILABILITY_ISSUE;

		if (!IsEnabled(mission))
			return SCR_EScenarioIssues.ADDON_DISABLED;

		return SCR_EScenarioIssues.NONE;
	}

//---- REFACTOR NOTE END ----
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Overreliance on scripts and hardcoded strings to drive UI looks. We should start considering the possibility of defining configurable stylesheets designers can work with outside of scripts
	
	// --- UI ---
	//------------------------------------------------------------------------------------------------
	protected static string GetErrorTexture(SCR_EScenarioIssues issue, SCR_ERevisionAvailability availability)
	{
		switch (issue)
		{
			case SCR_EScenarioIssues.NONE: 									return "";
			case SCR_EScenarioIssues.CONNECTION_ISSUE: 						return SCR_ConnectionUICommon.GetConnectionIssuesIcon();
			case SCR_EScenarioIssues.MISSION_MISSING: 						return SCR_WorkshopUiCommon.ICON_MOD_NOT_AVAILABLE;
			case SCR_EScenarioIssues.ADDON_DISABLED: 						return SCR_WorkshopUiCommon.ICON_DISABLED;
			case SCR_EScenarioIssues.ADDON_NOT_DOWNLOADED: 					return SCR_WorkshopUiCommon.ICON_MOD_NOT_AVAILABLE;
			case SCR_EScenarioIssues.ADDON_AVAILABILITY_ISSUE: 				return SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorTexture(availability);
			case SCR_EScenarioIssues.DEPENDENCIES_MISSING: 					return SCR_WorkshopUiCommon.ICON_DEPENDENCIES;
			case SCR_EScenarioIssues.DEPENDENCIES_VERSION_MISMATCH:			return SCR_WorkshopUiCommon.ICON_DEPENDENCIES;
			case SCR_EScenarioIssues.DEPENDENCIES_AVAILABILITY_ISSUE:		return SCR_WorkshopUiCommon.ICON_DEPENDENCIES;
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected static string GetErrorMessage(SCR_EScenarioIssues issue, SCR_ERevisionAvailability availability)
	{
		switch (issue)
		{
			case SCR_EScenarioIssues.NONE: 									return "";
			case SCR_EScenarioIssues.CONNECTION_ISSUE: 						return SCR_ConnectionUICommon.GetConnectionIssuesMessage();
			case SCR_EScenarioIssues.MISSION_MISSING: 						return SCR_WorkshopUiCommon.MESSAGE_MOD_NOT_AVAILABLE;
			case SCR_EScenarioIssues.ADDON_DISABLED: 						return SCR_WorkshopUiCommon.LABEL_ADDON_DISABLED;
			case SCR_EScenarioIssues.ADDON_NOT_DOWNLOADED: 					return SCR_WorkshopUiCommon.LABEL_ADDON_MISSING;
			case SCR_EScenarioIssues.ADDON_AVAILABILITY_ISSUE: 				return SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorMessage(availability);
			case SCR_EScenarioIssues.DEPENDENCIES_MISSING: 					return SCR_WorkshopUiCommon.MESSAGE_DEPENDENCIES_MISSING;
			case SCR_EScenarioIssues.DEPENDENCIES_VERSION_MISMATCH:			return SCR_WorkshopUiCommon.MESSAGE_DEPENDENCIES_MISSING;
			case SCR_EScenarioIssues.DEPENDENCIES_AVAILABILITY_ISSUE:		return SCR_WorkshopUiCommon.MESSAGE_DEPENDENCIES_MISSING;
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected static void SetInputButtonEnabled(SCR_InputButtonComponent button, SCR_EScenarioIssues issue, SCR_ERevisionAvailability availability)
	{
		if (!button)
			return;

		bool enabled = issue == SCR_EScenarioIssues.NONE;

		button.SetEnabled(enabled, false);

		if (enabled)
		{
			button.ResetTexture();
			return;
		}

		button.SetTexture(UIConstants.ICONS_IMAGE_SET, GetErrorTexture(issue, availability), Color.FromInt(UIColors.WARNING_DISABLED.PackToInt()));
	}

	//------------------------------------------------------------------------------------------------
	protected static void UpdateMouseButtonTooltip(SCR_ScriptedWidgetTooltip tooltip, SCR_EScenarioIssues issue, SCR_ERevisionAvailability availability)
	{
		if (!tooltip)
			return;

		SCR_ActionHintScriptedWidgetTooltip content = SCR_ActionHintScriptedWidgetTooltip.Cast(tooltip.GetContent());
		if (!content)
			return;

		if (issue == SCR_EScenarioIssues.NONE)
		{
			content.ResetMessage();
			content.SetMessageColor(Color.FromInt(UIColors.NEUTRAL_INFORMATION.PackToInt()));
			content.ResetActionColor();
			return;
		}

		content.SetMessage(GetErrorMessage(issue, availability));
		content.SetMessageColor(Color.FromInt(UIColors.WARNING.PackToInt()));
		content.SetActionColor(Color.FromInt(UIColors.IDLE_DISABLED.PackToInt()));
	}

	// --- Play, Continue, Restart ---
	//------------------------------------------------------------------------------------------------
	static void UpdatePlayMouseButton(SCR_ModularButtonComponent button, MissionWorkshopItem mission, bool entryFocused, bool visible = true)
	{
		if (!button)
			return;

		visible = visible && entryFocused && !HasSave(mission);

		button.SetVisible(visible);
		if (visible)
			SCR_ListEntryHelper.UpdateMouseButtonColor(button, GetPlayHighestPriorityIssue(mission) != SCR_EScenarioIssues.NONE, entryFocused);
	}

	//------------------------------------------------------------------------------------------------
	static void UpdateContinueMouseButton(SCR_ModularButtonComponent button, MissionWorkshopItem mission, bool entryFocused, bool visible = true)
	{
		if (!button)
			return;

		visible = visible && entryFocused && HasSave(mission);

		button.SetVisible(visible);
		if (visible)
			SCR_ListEntryHelper.UpdateMouseButtonColor(button, GetPlayHighestPriorityIssue(mission) != SCR_EScenarioIssues.NONE, entryFocused);
	}

	//------------------------------------------------------------------------------------------------
	static void UpdateRestartMouseButton(SCR_ModularButtonComponent button, MissionWorkshopItem mission, bool entryFocused, bool visible = true)
	{
		if (!button)
			return;

		visible = visible && HasSave(mission);

		button.SetVisible(visible);
		if (visible)
			SCR_ListEntryHelper.UpdateMouseButtonColor(button, GetPlayHighestPriorityIssue(mission) != SCR_EScenarioIssues.NONE, entryFocused);
	}

	//------------------------------------------------------------------------------------------------
	static void UpdatePlayMouseButtonTooltip(SCR_ScriptedWidgetTooltip tooltip, MissionWorkshopItem mission)
	{
		UpdateMouseButtonTooltip(tooltip, GetPlayHighestPriorityIssue(mission), GetOwnerRevisionAvailability(mission));
	}

	//------------------------------------------------------------------------------------------------
	static void UpdatePlayInputButton(SCR_InputButtonComponent button, MissionWorkshopItem mission, bool visible = true)
	{
		if (!button)
			return;

		visible = visible && !HasSave(mission);
		button.SetVisible(visible, false);

		if (visible)
			SetInputButtonEnabled(button, GetPlayHighestPriorityIssue(mission), GetOwnerRevisionAvailability(mission));
	}

	//------------------------------------------------------------------------------------------------
	static void UpdateContinueInputButton(SCR_InputButtonComponent button, MissionWorkshopItem mission, bool visible = true)
	{
		if (!button)
			return;

		visible = visible && HasSave(mission);
		button.SetVisible(visible, false);

		if (visible)
			SetInputButtonEnabled(button, GetPlayHighestPriorityIssue(mission), GetOwnerRevisionAvailability(mission));
	}

	//------------------------------------------------------------------------------------------------
	static void UpdateRestartInputButton(SCR_InputButtonComponent button, MissionWorkshopItem mission, bool visible = true)
	{
		if (!button)
			return;

		visible = visible && HasSave(mission);
		button.SetVisible(visible, false);

		if (visible)
			SetInputButtonEnabled(button, GetPlayHighestPriorityIssue(mission), GetOwnerRevisionAvailability(mission));
	}

	// --- Join (Find Servers) ---
	//------------------------------------------------------------------------------------------------
	static void UpdateJoinMouseButton(SCR_ModularButtonComponent button, MissionWorkshopItem mission, bool entryFocused, bool visible = true)
	{
		if (!button)
			return;

		visible = visible && entryFocused && IsMultiplayer(mission);

		button.SetVisible(visible);
		if (visible)
			SCR_ListEntryHelper.UpdateMouseButtonColor(button, GetJoinHighestPriorityIssue(mission) != SCR_EScenarioIssues.NONE, entryFocused);
	}

	//------------------------------------------------------------------------------------------------
	static void UpdateJoinMouseButtonTooltip(SCR_ScriptedWidgetTooltip tooltip, MissionWorkshopItem mission)
	{
		UpdateMouseButtonTooltip(tooltip, GetJoinHighestPriorityIssue(mission), GetOwnerRevisionAvailability(mission));
	}

	//------------------------------------------------------------------------------------------------
	static void UpdateJoinInputButton(SCR_InputButtonComponent button, MissionWorkshopItem mission, bool visible = true)
	{
		if (!button)
			return;

		visible = visible && IsMultiplayer(mission);
		button.SetVisible(visible, false);

		if (visible)
			SetInputButtonEnabled(button, GetJoinHighestPriorityIssue(mission), GetOwnerRevisionAvailability(mission));
	}

	// --- Host ---
	//------------------------------------------------------------------------------------------------
	static void UpdateHostMouseButton(SCR_ModularButtonComponent button, MissionWorkshopItem mission, bool entryFocused, bool visible = true)
	{
		if (!button)
			return;

		visible = visible && entryFocused && IsMultiplayer(mission) && !GetGame().IsPlatformGameConsole();

		button.SetVisible(visible);
		if (visible)
			SCR_ListEntryHelper.UpdateMouseButtonColor(button, GetHostHighestPriorityIssue(mission) != SCR_EScenarioIssues.NONE, entryFocused);
	}

	//------------------------------------------------------------------------------------------------
	static void UpdateHostMouseButtonTooltip(SCR_ScriptedWidgetTooltip tooltip, MissionWorkshopItem mission)
	{
		UpdateMouseButtonTooltip(tooltip, GetHostHighestPriorityIssue(mission), GetOwnerRevisionAvailability(mission));
	}

	//------------------------------------------------------------------------------------------------
	static void UpdateHostInputButton(SCR_InputButtonComponent button, MissionWorkshopItem mission, bool visible = true)
	{
		if (!button)
			return;

		visible = visible && IsMultiplayer(mission) && !GetGame().IsPlatformGameConsole();
		button.SetVisible(visible, false);

		if (visible)
			SetInputButtonEnabled(button, GetHostHighestPriorityIssue(mission), GetOwnerRevisionAvailability(mission));
	}

	// --- Input Buttons ---
	//------------------------------------------------------------------------------------------------
	static void UpdateInputButtons(MissionWorkshopItem mission, array<SCR_InputButtonComponent> buttons, bool visible = true)
	{
		Widget root;
		
		foreach (SCR_InputButtonComponent button : buttons)
		{
			root = button.GetRootWidget();
			if (!root)
				continue;

			switch (root.GetName())
			{
				case SCR_ConfigurableDialogUi.BUTTON_CONFIRM: 	UpdatePlayInputButton(button, mission, visible); 		break;
				case BUTTON_PLAY: 								UpdatePlayInputButton(button, mission, visible); 		break;
				case BUTTON_CONTINUE: 							UpdateContinueInputButton(button, mission, visible); 	break;
				case BUTTON_RESTART: 							UpdateRestartInputButton(button, mission, visible); 	break;
				case BUTTON_FIND_SERVERS: 						UpdateJoinInputButton(button, mission, visible); 		break;
				case BUTTON_HOST: 								UpdateHostInputButton(button, mission, visible); 		break;
			}
		}
	}

	// --- Tooltips ---
	//------------------------------------------------------------------------------------------------
	static void UpdateMouseButtonTooltips(SCR_ScriptedWidgetTooltip tooltip, MissionWorkshopItem mission)
	{
		if (!tooltip)
			return;

		if (tooltip.IsValid(BUTTON_PLAY) || tooltip.IsValid(BUTTON_CONTINUE) || tooltip.IsValid(BUTTON_RESTART))
			UpdatePlayMouseButtonTooltip(tooltip, mission);

		else if (tooltip.IsValid(BUTTON_FIND_SERVERS))
			UpdateJoinMouseButtonTooltip(tooltip, mission);

		else if (tooltip.IsValid(BUTTON_HOST))
			UpdateHostMouseButtonTooltip(tooltip, mission);
	}
	
	//---- REFACTOR NOTE END ----
}

enum SCR_EScenarioIssues
{
	NONE,
	CONNECTION_ISSUE,
	MISSION_MISSING,
	ADDON_DISABLED,
	ADDON_NOT_DOWNLOADED,
	ADDON_AVAILABILITY_ISSUE,
	DEPENDENCIES_MISSING,
	DEPENDENCIES_VERSION_MISMATCH,
	DEPENDENCIES_AVAILABILITY_ISSUE
}
