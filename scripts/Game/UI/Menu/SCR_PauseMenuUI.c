//------------------------------------------------------------------------------------------------
class PauseMenuUI : ChimeraMenuBase
{
	InputManager m_InputManager;

	protected TextWidget m_wVersion;
	protected Widget m_wRoot;
	protected Widget m_wFade;
	protected Widget m_wSystemTime;	
	protected BlurWidget m_wBlurEffect;
	protected bool m_bFocused = true;
	
	protected const float DURATION_IN_BLUR = 0.75;
	protected const float DURATION_OUT_BLUR = 0.5;
	
	protected const float VALUE_BLUR = 0.8;
	protected const float VALUE_DISABLED = 0.0;

	//Editor and Photo mode Specific
	protected SCR_ButtonTextComponent m_EditorUnlimitedOpenButton;
	protected SCR_ButtonTextComponent m_EditorUnlimitedCloseButton;
	protected SCR_ButtonTextComponent m_EditorPhotoOpenButton;
	protected SCR_ButtonTextComponent m_EditorPhotoCloseButton;
	
	protected SCR_ButtonTextComponent m_SettingsButton;

	protected ref SCR_ConfigurableDialogUi m_ExitDialog;

	const string EXIT_SAVE = "#AR-PauseMenu_ReturnSaveTitle";
	const string EXIT_NO_SAVE = "#AR-PauseMenu_ReturnTitle";

	const string EXIT_MESSAGE = "#AR-PauseMenu_ReturnText";
	const string EXIT_TITLE = "#AR-PauseMenu_ReturnTitle";
	const string EXIT_IMAGE = "exit";

	const string RESTART_MESSAGE = "#AR-PauseMenu_RestartText";
	const string RESTART_TITLE = "#AR-PauseMenu_Restart";
	const string RESTART_IMAGE = "restart";

	const string LOAD_MESSAGE = "#AR-PauseMenu_LoadText";
	const string LOAD_TITLE = "#AR-PauseMenu_Load";
	const string LOAD_IMAGE = "up";

	const string DIALOG_SCENARIO_EXIT = "scenario_exit";
	const string DIALOG_SAVE_FAILED = "pause_menu_save_failed";

	static ref ScriptInvoker m_OnPauseMenuOpened = new ScriptInvoker();
	static ref ScriptInvoker m_OnPauseMenuClosed = new ScriptInvoker();

	protected static PauseMenuUI s_Instance;

	//------------------------------------------------------------------------------------------------
	// If the pause menu was open, reinitialize it so it sits on top
	static void MoveToTop()
	{
		if (!s_Instance)
			return;

		s_Instance.Close();

		// Must be called the next frame for the menu to be reopened immediately after closing
		GetGame().GetCallqueue().CallLater(OpenMenuOnTop);

		// TODO: tried setting the ZOrder but it does not work: ask for Enfusion API
	}

	//------------------------------------------------------------------------------------------------
	// Opens the pause menu with settings for it to be on top of other menus
	static void OpenMenuOnTop()
	{
		GetGame().OpenPauseMenu(false, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		bool isReplicationRunning = Replication.IsRunning();
		
		s_Instance = this;

		m_wRoot = GetRootWidget();
		m_wFade = m_wRoot.FindAnyWidget("BackgroundFade");
		m_wSystemTime = m_wRoot.FindAnyWidget("SystemTime");
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		SCR_ButtonTextComponent comp;

		// Pause game
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.PauseGame(true, SCR_EPauseReason.MENU | SCR_EPauseReason.MUSIC);

		// Continue
		comp = SCR_ButtonTextComponent.GetButtonText("Continue", m_wRoot);
		if (comp)
		{
			GetGame().GetWorkspace().SetFocusedWidget(comp.GetRootWidget());
			comp.m_OnClicked.Insert(Close);
		}

		// Restart
		comp = SCR_ButtonTextComponent.GetButtonText("Restart", m_wRoot);
		if (comp)
		{
			bool enabledRestart = !isReplicationRunning;
			comp.GetRootWidget().SetVisible(enabledRestart);
			comp.m_OnClicked.Insert(OnRestart);
		}

		// Respawn
		SCR_RespawnSystemComponent respawnComponent = SCR_RespawnSystemComponent.GetInstance();

		comp = SCR_ButtonTextComponent.GetButtonText("Respawn", m_wRoot);
		if (comp)
		{
			if (respawnComponent && !respawnComponent.IsPauseMenuRespawnEnabled())
			{
				comp.SetVisible(false);
			}
			else
			{
				bool canRespawn;
				if (gameMode)
				{
					RespawnSystemComponent respawn = RespawnSystemComponent.Cast(gameMode.FindComponent(RespawnSystemComponent));
					canRespawn = (respawn != null && CanRespawn());
				}

				comp.GetRootWidget().SetVisible(canRespawn);
				comp.m_OnClicked.Insert(OnRespawn);
			}
		}

		// Leave faction
		comp = SCR_ButtonTextComponent.GetButtonText("LeaveFaction", m_wRoot);
		if (comp)
		{
			bool factionLeaveAllowed;
			SCR_BaseGameMode gm = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gm)
				factionLeaveAllowed = gm.IsFactionChangeAllowed();

			SCR_FactionManager fm = SCR_FactionManager.Cast(GetGame().GetFactionManager());
			if (fm)
			{
				array<Faction> factions = {};
				fm.GetFactionsList(factions);
				int playableFactionCount = 0;
				foreach (Faction f : factions)
				{
					SCR_Faction scriptedFaction = SCR_Faction.Cast(f);
					if (scriptedFaction && scriptedFaction.IsPlayable())
						playableFactionCount++;
				}

				factionLeaveAllowed = factionLeaveAllowed && (playableFactionCount > 1);
			}

			comp.GetRootWidget().SetVisible(factionLeaveAllowed);
			if (factionLeaveAllowed)
			{
				PlayerController pc = GetGame().GetPlayerController();
				SCR_PlayerFactionAffiliationComponent factionComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));

				if (factionComp)
					comp.SetEnabled(factionComp.GetAffiliatedFaction() != null);

				comp.m_OnClicked.Insert(OnLeaveFaction)
			}
		}

		// Exit
		comp = SCR_ButtonTextComponent.GetButtonText("Exit", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnExit);
			if (IsSavingOnExit())
				comp.SetText(EXIT_SAVE);
			else
				comp.SetText(EXIT_NO_SAVE);
		}

		// Rewind
		comp = SCR_ButtonTextComponent.GetButtonText("Rewind", m_wRoot);
		if (comp)
		{			
			SCR_RewindComponent rewindManager = SCR_RewindComponent.GetInstance();
			comp.GetRootWidget().SetVisible(rewindManager != null); //--- Hide the button when rewinding is not configured for the mission
			comp.SetEnabled(rewindManager && rewindManager.HasRewindPoint()); //--- Disable the button when the rewind point does not exist
			comp.m_OnClicked.Insert(OnRewind);
		}

		// Tutorial HUB
		//TODO> Rename
		comp = SCR_ButtonTextComponent.GetButtonText("ReturnHUB", m_wRoot);
		if (comp)
		{
			SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();

			if (tutorial && tutorial.CanBreakCourse())
			{
				comp.SetVisible(true);
				comp.m_OnClicked.Insert(OnCourseBreak);
			}
			else
			{
				comp.SetVisible(false);
			}
		}

		// Camera
		comp = SCR_ButtonTextComponent.GetButtonText("Camera", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnCamera);
			comp.SetEnabled(editorManager && !editorManager.IsOpened());
			comp.GetRootWidget().SetVisible(Game.IsDev());
		}

		// Settings
		m_SettingsButton = SCR_ButtonTextComponent.GetButtonText("Settings", m_wRoot);
		if (m_SettingsButton)
			m_SettingsButton.m_OnClicked.Insert(OnSettings);

		// Field Manual
		comp = SCR_ButtonTextComponent.GetButtonText("FieldManual", m_wRoot);
		if (comp)
			comp.m_OnClicked.Insert(OnFieldManual);

		// Group menu
		comp = SCR_ButtonTextComponent.GetButtonText("GroupMenu", m_wRoot);
		if (comp)
		{
			SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
			if (!groupManager || !groupManager.IsGroupMenuAllowed())
				comp.GetRootWidget().SetVisible(false);

			comp.m_OnClicked.Insert(OnGroupMenu);

		}

		// Block list
		comp = SCR_ButtonTextComponent.GetButtonText("BlockList", m_wRoot);
		if (comp)
		{
			comp.GetRootWidget().SetVisible(isReplicationRunning);
			comp.m_OnClicked.Insert(OnBlockList);
		}

		// Players
		comp = SCR_ButtonTextComponent.GetButtonText("Players", m_wRoot);
		if (comp)
		{
			comp.GetRootWidget().SetVisible(isReplicationRunning);
			comp.m_OnClicked.Insert(OnPlayers);
		}

		// Invite friends
		comp = SCR_ButtonTextComponent.GetButtonText("InviteFriend", m_wRoot);
		if (comp)
		{
			bool canInvite = isReplicationRunning && GetGame().GetPlayerManager().IsMultiplayerActivityInviteAvailable();
			comp.GetRootWidget().SetVisible(canInvite);

			if (canInvite)
				comp.m_OnClicked.Insert(OnInviteFriends);
		}

		// Version
		m_wVersion = TextWidget.Cast(m_wRoot.FindAnyWidget("Version"));
		if (m_wVersion)
			m_wVersion.SetText(GetGame().GetBuildVersion());

		// Unlimited editor (Game Master)
		m_EditorUnlimitedOpenButton = SCR_ButtonTextComponent.GetButtonText("EditorUnlimitedOpen", m_wRoot);
		if (m_EditorUnlimitedOpenButton)
			m_EditorUnlimitedOpenButton.m_OnClicked.Insert(OnEditorUnlimited);

		m_EditorUnlimitedCloseButton = SCR_ButtonTextComponent.GetButtonText("EditorUnlimitedClose", m_wRoot);
		if (m_EditorUnlimitedCloseButton)
			m_EditorUnlimitedCloseButton.m_OnClicked.Insert(OnEditorUnlimited);

		//--- Photo mode
		m_EditorPhotoOpenButton = SCR_ButtonTextComponent.GetButtonText("EditorPhotoOpen", m_wRoot);
		if (m_EditorPhotoOpenButton)
			m_EditorPhotoOpenButton.m_OnClicked.Insert(OnEditorPhoto);
		m_EditorPhotoCloseButton = SCR_ButtonTextComponent.GetButtonText("EditorPhotoClose", m_wRoot);
		if (m_EditorPhotoCloseButton)
			m_EditorPhotoCloseButton.m_OnClicked.Insert(OnEditorPhoto);

		SetEditorUnlimitedButton(editorManager);
		SetEditorPhotoButton(editorManager);

		if (editorManager)
		{
			editorManager.GetOnModeAdd().Insert(OnEditorModeChanged);
			editorManager.GetOnModeRemove().Insert(OnEditorModeChanged);
		}

		comp = SCR_ButtonTextComponent.GetButtonText("Feedback", m_wRoot);
		if (comp)
		{
			comp.m_OnClicked.Insert(OnFeedback);
		}
		
		//Hide second separator when second menu group is empty
		if (!m_EditorPhotoCloseButton.IsVisible() && !m_EditorPhotoOpenButton.IsVisible() && !m_EditorUnlimitedOpenButton.IsVisible() && !m_EditorUnlimitedCloseButton.IsVisible())
		{
			ImageWidget separatorSecondGroup = ImageWidget.Cast(m_wRoot.FindAnyWidget("Separator0"));
			if (separatorSecondGroup)
				separatorSecondGroup.SetVisible(false);
		}
		
		m_InputManager = GetGame().GetInputManager();

		m_OnPauseMenuOpened.Invoke();

		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_FE_HUD_PAUSE_MENU_OPEN);
		
		SCR_CampaignBuildingEditorComponent CampaignBuildingEditorComponent = SCR_CampaignBuildingEditorComponent.Cast(SCR_CampaignBuildingEditorComponent.GetInstance(SCR_CampaignBuildingEditorComponent));
		if (!CampaignBuildingEditorComponent)
			return;
		
		m_wBlurEffect = BlurWidget.Cast(m_wRoot.FindAnyWidget("ScreenEffectBlur"));
		CampaignBuildingEditorComponent.GetOnObstructionEventTriggered().Insert(AreaTriggerChange);
		
		if (CampaignBuildingEditorComponent.IsViewObstructed())
			StartObstructionAnimation(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] activated
	protected void AreaTriggerChange(bool activated)
	{
		if (activated)
			StartObstructionAnimation();
		else
			FinishObstructionAnimation();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Start screen obstruction effect.
	protected void StartObstructionAnimation(bool force = false)
	{
		AnimateWidget.StopAnimation(m_wBlurEffect, WidgetAnimationBlurIntensity);
		if (force)
			m_wBlurEffect.SetIntensity(VALUE_BLUR);
		else
			AnimateWidget.BlurIntensity(m_wBlurEffect, VALUE_BLUR, DURATION_IN_BLUR);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Ends screen obstruction effect.
	protected void FinishObstructionAnimation(bool force = false)
	{
		AnimateWidget.StopAnimation(m_wBlurEffect, WidgetAnimationBlurIntensity);
		if (force)
			m_wBlurEffect.SetIntensity(VALUE_DISABLED);
		else
			AnimateWidget.BlurIntensity(m_wBlurEffect, VALUE_DISABLED, DURATION_IN_BLUR);
	}

	//------------------------------------------------------------------------------------------------
	private void OnCourseBreak()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;

		tutorial.RequestBreakCourse(SCR_ETutorialBreakType.FORCED);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		//--- Close pause menu when editor is opened or closed
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			editorManager.GetOnOpened().Insert(Close);
			editorManager.GetOnClosed().Insert(Close);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			editorManager.GetOnOpened().Insert(Close);
			editorManager.GetOnClosed().Insert(Close);
		}

		if (m_wFade)
			m_wFade.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		s_Instance = null;

		// Unpause
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.PauseGame(false,  SCR_EPauseReason.MENU | SCR_EPauseReason.MUSIC);

		SCR_HUDManagerComponent hud = GetGame().GetHUDManager();
		if (hud)
			hud.SetVisible(true);

		m_OnPauseMenuClosed.Invoke();

		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_FE_HUD_PAUSE_MENU_CLOSE);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		m_bFocused = false;
		m_InputManager.RemoveActionListener(UIConstants.MENU_ACTION_OPEN, EActionTrigger.DOWN, Close);
		m_InputManager.RemoveActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, Close);
		#ifdef WORKBENCH
			m_InputManager.RemoveActionListener(UIConstants.MENU_ACTION_OPEN_WB, EActionTrigger.DOWN, Close);
			m_InputManager.RemoveActionListener(UIConstants.MENU_ACTION_BACK_WB, EActionTrigger.DOWN, Close);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		m_bFocused = true;
		m_InputManager.AddActionListener(UIConstants.MENU_ACTION_OPEN, EActionTrigger.DOWN, Close);
		m_InputManager.AddActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, Close);
		#ifdef WORKBENCH
			m_InputManager.AddActionListener(UIConstants.MENU_ACTION_OPEN_WB, EActionTrigger.DOWN, Close);
			m_InputManager.AddActionListener(UIConstants.MENU_ACTION_BACK_WB, EActionTrigger.DOWN, Close);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	void FadeBackground(bool fade, bool animate = true)
	{
		if (!m_wFade)
			return;

		m_wFade.SetVisible(fade);
		m_wFade.SetOpacity(0);
		if (fade && animate)
			AnimateWidget.Opacity(m_wFade, 1, UIConstants.FADE_RATE_FAST, true);
	}

	//------------------------------------------------------------------------------------------------
	private void OnSettings()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.SettingsSuperMenu);
	}

	//------------------------------------------------------------------------------------------------
	private void OnFieldManual()
	{
		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.FieldManualDialog);
	}

	//------------------------------------------------------------------------------------------------
	private void OnGroupMenu()
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager || !groupManager.IsGroupMenuAllowed())
			return;

		GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.GroupMenu);
	}

	//------------------------------------------------------------------------------------------------
	private void OnBlockList()
	{
		SCR_BlockedUsersDialogUI dialog = new SCR_BlockedUsersDialogUI();
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_AccountWidgetComponent.BLOCKED_USER_DIALOG_CONFIG, "blocked_list", dialog);
	}

	//------------------------------------------------------------------------------------------------
	private void OnExit()
	{
		// Create exit dialog
		m_ExitDialog = SCR_CommonDialogs.CreateDialog(DIALOG_SCENARIO_EXIT);
		if (!m_ExitDialog)
			return;

		m_ExitDialog.m_OnConfirm.Insert(OnExitConfirm);
	}

	//------------------------------------------------------------------------------------------------
	private void OnRewind()
	{
		new SCR_RewindDialog();
	}

	//------------------------------------------------------------------------------------------------
	private void OnExitConfirm()
	{
		m_ExitDialog.m_OnConfirm.Remove(OnExitConfirm);

		if (IsSavingOnExit())
		{
			//--- Close only after the save file was created
			GetGame().GetSaveGameManager().RequestSavePoint(ESaveGameType.SHUTDOWN, flags: ESaveGameRequestFlags.BLOCKING, callback: new SaveGameOperationCb(OnExitSaveResult))
		}
		else
		{
			//--- Close instantly
			CloseToMainMenu();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnExitSaveResult(bool success)
	{
		if (success)
		{
			CloseToMainMenu();
			return;
		}

		m_ExitDialog.Close();

		SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateDialog(DIALOG_SAVE_FAILED);
		if (!dialog)
			return;

		dialog.m_OnConfirm.Insert(CloseToMainMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseToMainMenu()
	{
		ChimeraWorld world = GetGame().GetWorld();
		world.PauseGameTime(false);

		Close();
		GameStateTransitions.RequestGameplayEndTransition();
	}

	//------------------------------------------------------------------------------------------------
	private void OnEditorUnlimited()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			if (!editorManager.IsOpened() || editorManager.GetCurrentModeEntity().IsLimited())
			{
				editorManager.SetCurrentMode(false);
				editorManager.Open();
			}
			else
			{
				editorManager.Close();
			}
		}
		Close();
	}

	//Update Editor Mode Button text and if Enabled
	//------------------------------------------------------------------------------------------------
	private void SetEditorUnlimitedButton(SCR_EditorManagerEntity editorManager)
	{
		if (!m_EditorUnlimitedOpenButton || !m_EditorUnlimitedCloseButton) return;

		if (!editorManager || editorManager.IsLimited())
		{
			//--- LIMITED EDITOR

			//--- Show disabled "Open Unlimited Editor" button when editor is *legal* in the mission
			bool isUnlimitedEditorLegal;
			SCR_EditorSettingsEntity settingsEntity = SCR_EditorSettingsEntity.GetInstance();
			if (settingsEntity)
				isUnlimitedEditorLegal = settingsEntity.IsUnlimitedEditorLegal();

			m_EditorUnlimitedOpenButton.GetRootWidget().SetVisible(isUnlimitedEditorLegal);
			m_EditorUnlimitedOpenButton.SetEnabled(false);

			//--- Don't show the "Close Unlimited Editor" button
			m_EditorUnlimitedCloseButton.GetRootWidget().SetVisible(false);

			//--- Don't show the real-time clock
			m_wSystemTime.SetVisible(false);
		}
		else
		{
			//--- UNLIMITED EDITOR
			if (!editorManager.IsOpened() || editorManager.GetCurrentModeEntity().IsLimited())
			{
				//--- Current mode is limited, show the "Open Unlimited Editor" button
				m_EditorUnlimitedOpenButton.GetRootWidget().SetVisible(true);
				m_EditorUnlimitedCloseButton.GetRootWidget().SetVisible(false);
			}
			else
			{
				//--- Current mode is unlimited, show the "Close Unlimited Editor" button
				m_EditorUnlimitedOpenButton.GetRootWidget().SetVisible(false);
				m_EditorUnlimitedCloseButton.GetRootWidget().SetVisible(true);
				m_EditorUnlimitedCloseButton.SetEnabled(editorManager.CanClose());
			}

			//--- Show the real-time clock
			m_wSystemTime.SetVisible(true);
		}
	}

	//Updates Editor and Photomode button if Rights changed
	//------------------------------------------------------------------------------------------------
	protected void OnEditorModeChanged(SCR_EditorModeEntity modeEntity)
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		SetEditorUnlimitedButton(editorManager);
		SetEditorPhotoButton(editorManager);
	}

	//------------------------------------------------------------------------------------------------
	private void OnEditorPhoto()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			if (!editorManager.IsOpened() || editorManager.GetCurrentMode() != EEditorMode.PHOTO)
			{
				editorManager.SetCurrentMode(EEditorMode.PHOTO);
				editorManager.Open();
			}
			else
			{
				editorManager.Close();
			}
		}
		Close();
	}

	//Update Photo Mode Button text and if Enabled
	//------------------------------------------------------------------------------------------------
	private void SetEditorPhotoButton(SCR_EditorManagerEntity editorManager)
	{
		if (!m_EditorPhotoOpenButton || !m_EditorPhotoCloseButton) return;
		if (Replication.IsRunning())
		{
			m_EditorPhotoOpenButton.GetRootWidget().SetVisible(false);
			m_EditorPhotoCloseButton.GetRootWidget().SetVisible(false);
			
		}
		else
		{
			if (!editorManager || !editorManager.HasMode(EEditorMode.PHOTO))
			{
				m_EditorPhotoOpenButton.GetRootWidget().SetVisible(true);
				m_EditorPhotoOpenButton.SetEnabled(false);
				m_EditorPhotoCloseButton.GetRootWidget().SetVisible(false);
			}
			else
			{
				if (!editorManager.IsOpened() || editorManager.GetCurrentMode() != EEditorMode.PHOTO)
				{
					m_EditorPhotoOpenButton.GetRootWidget().SetVisible(true);
					m_EditorPhotoCloseButton.GetRootWidget().SetVisible(false);
	
					//Set enabled
					m_EditorPhotoOpenButton.SetEnabled(!editorManager.IsLimited() || GetGame().GetPlayerController().GetControlledEntity());
				}
				else
				{
					m_EditorPhotoOpenButton.GetRootWidget().SetVisible(false);
					m_EditorPhotoCloseButton.GetRootWidget().SetVisible(true);
					m_EditorPhotoCloseButton.SetEnabled(editorManager.CanClose());
				}
			}
		}		

	}

	//------------------------------------------------------------------------------------------------
	void DisableSettings()
	{
		if (m_SettingsButton)
			m_SettingsButton.SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	void DisableGameMaster()
	{
		if (m_EditorUnlimitedOpenButton)
			m_EditorUnlimitedOpenButton.SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	void DisableArmaVision()
	{
		if (m_EditorPhotoOpenButton)
			m_EditorPhotoOpenButton.SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	private void OnFeedback()
	{
		SCR_FeedbackDialogUI.OpenFeedbackDialog();
	}

	//------------------------------------------------------------------------------------------------
	private void OnRestart()
	{
		// Create dialog
		SCR_ConfigurableDialogUi dialog = SCR_CommonDialogs.CreateDialog(SCR_ScenarioUICommon.DIALOG_RESTART);
		if (!dialog)
			return;

		dialog.m_OnConfirm.Insert(OnRestartConfirm);
	}

	//------------------------------------------------------------------------------------------------
	private void OnRestartConfirm()
	{
		GetGame().GetMenuManager().CloseAllMenus();

		auto manager = GetGame().GetSaveGameManager();
		manager.StartPlaythrough(manager.GetCurrentMissionResource());
	}

	//------------------------------------------------------------------------------------------------
	private void OnPlayers()
	{
		ArmaReforgerScripted.OpenPlayerList();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInviteFriends()
	{
		GetGame().GetPlayerManager().ShowMultiplayerActivityInvite();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the player has a Character he is controling and that is alive
	//! \return True is there is a valid player that is alive, false otherwise
	protected bool CanRespawn()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return false;

		IEntity player = pc.GetControlledEntity();
		if (!player)
			return false;

		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(player);
		if (!char)
			return false;

		CharacterControllerComponent charController = char.GetCharacterController();
		if (!charController)
			return false;

		if (charController.GetLifeState() == ECharacterLifeState.DEAD)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	private void OnRespawn()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_RespawnComponent respawn = SCR_RespawnComponent.Cast(playerController.FindComponent(SCR_RespawnComponent));
		if (!respawn)
			return;

		respawn.RequestPlayerSuicide();
		Close();
	}

	//------------------------------------------------------------------------------------------------
	private void OnLeaveFaction()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return;

		SCR_PlayerFactionAffiliationComponent factionComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
		if (!factionComp)
			return;

		SCR_RespawnComponent rc = SCR_RespawnComponent.Cast(pc.FindComponent(SCR_RespawnComponent));
		if (!rc)
			return;

		factionComp.RequestFaction(null);
		rc.RequestPlayerSuicide();

		Close();
	}

	//------------------------------------------------------------------------------------------------
	private void OnCamera()
	{
		SCR_DebugCameraCore cameraCore = SCR_DebugCameraCore.Cast(SCR_DebugCameraCore.GetInstance(SCR_DebugCameraCore));
		if (cameraCore)
			cameraCore.ToggleCamera();
		Close();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		//Remove Editor modes listener
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();

		if (editorManager)
		{
			editorManager.GetOnModeAdd().Remove(OnEditorModeChanged);
			editorManager.GetOnModeRemove().Remove(OnEditorModeChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsSavingOnExit()
	{
		return !Replication.IsRunning() && GetGame().GetSaveGameManager().IsSavingAllowed();
	}
}
