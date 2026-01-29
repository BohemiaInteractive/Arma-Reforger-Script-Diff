class SCR_PlayerControllerClass : PlayerControllerClass
{
}

//------------------------------------------------------------------------------------------------
void OnControlledEntityChangedPlayerController(IEntity from, IEntity to);
typedef func OnControlledEntityChangedPlayerController;
typedef ScriptInvokerBase<OnControlledEntityChangedPlayerController> OnControlledEntityChangedPlayerControllerInvoker;

//------------------------------------------------------------------------------------------------
void OwnershipChangedDelegate(bool isChanging, bool becameOwner);
typedef func OwnershipChangedDelegate;
typedef ScriptInvokerBase<OwnershipChangedDelegate> OnOwnershipChangedInvoker;

//------------------------------------------------------------------------------------------------
void OnDestroyedPlayerController(Instigator killer, IEntity killerEntity);
typedef func OnDestroyedPlayerController;
typedef ScriptInvokerBase<OnDestroyedPlayerController> OnDestroyedPlayerControllerInvoker;

//------------------------------------------------------------------------------------------------
void OnPossessed(IEntity entity);
typedef func OnPossessed;
typedef ScriptInvokerBase<OnPossessed> OnPossessedInvoker;

//------------------------------------------------------------------------------------------------
void OnBeforePossessed(IEntity entity);
typedef func OnBeforePossessed;
typedef ScriptInvokerBase<OnBeforePossessed> OnBeforePossessedInvoker;

class SCR_PlayerController : PlayerController
{
	static PlayerController s_pLocalPlayerController;
	protected static const float WALK_SPEED = 0.5;
	protected static const float FOCUS_ACTIVATION = 0.1;
	protected static const float FOCUS_DEACTIVATION = 0.05;
	protected static const float FOCUS_TIMEOUT = 0.3;
	protected static const float FOCUS_TOLERANCE = 0.005;
	protected static const float FOCUS_ANALOGUE_SCALE = 3.0;
	protected static float s_fADSFocus = 0.5;
	protected static float s_fPIPFocus = 1;
	protected static float s_fFocusTimeout;
	protected static float s_fFocusAnalogue;
	protected static bool s_bWasADS;

	protected CharacterControllerComponent m_CharacterController;
	protected bool m_bIsLocalPlayerController;
	protected bool m_bIsPaused;
	bool m_bRetain3PV;
	protected bool m_bGadgetFocus;
	protected SCR_EFocusToggleMode m_eFocusToggle;
	protected float m_fCharacterSpeed;


	[RplProp(onRplName: "OnRplMainEntityFromID")]
	protected RplId m_MainEntityID;
	protected IEntity m_MainEntity;

	[RplProp()]
	protected bool m_bIsPossessing;

	ref OnBeforePossessedInvoker m_OnBeforePossess = new OnBeforePossessedInvoker();		// Before an entity becomes possesed.
	ref OnPossessedInvoker m_OnPossessed = new OnPossessedInvoker();		// when entity becomes possessed or control returns to the main entity
	ref OnControlledEntityChangedPlayerControllerInvoker m_OnControlledEntityChanged = new OnControlledEntityChangedPlayerControllerInvoker();
	ref OnDestroyedPlayerControllerInvoker m_OnDestroyed = new OnDestroyedPlayerControllerInvoker();		// main entity is destroyed
	ref OnOwnershipChangedInvoker m_OnOwnershipChangedInvoker = new OnOwnershipChangedInvoker();
	//------------------------------------------------------------------------------------------------
	/*!
		\see PlayerController.OnOwnershipChanged for more information.
	*/
	OnOwnershipChangedInvoker GetOnOwnershipChangedInvoker()
	{
		return m_OnOwnershipChangedInvoker;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		\see PlayerController.OnOwnershipChanged for more information.
	*/
	protected override void OnOwnershipChanged(bool changing, bool becameOwner)
	{
		super.OnOwnershipChanged(changing, becameOwner);
		
		if (becameOwner)
		{
			SocialComponent socialComp = SocialComponent.Cast(FindComponent(SocialComponent));
			if (socialComp)
				socialComp.m_OnBlockedPlayerJoinedInvoker.Insert(OnBlockedPlayerJoined);
			
			if (!changing)
			{
				SCR_EditorManagerCore managerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
				managerCore.Event_OnEditorManagerInitOwner.Insert(OnPlayerRegistered);
			}
		}

		SCR_DamageSufferingSystem.GetInstance().StartObservingControlledEntityChanges(this, changing, becameOwner);
		
		m_OnOwnershipChangedInvoker.Invoke(changing, becameOwner);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;

		if (Replication.IsServer()) // this is here because listen server or SP client will not call OnOwnershipChanged as there is no transfer of ownership
			SCR_DamageSufferingSystem.GetInstance().StartObservingControlledEntityChanges(this, false, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerRegistered(SCR_EditorManagerEntity managerEntity)
	{
		SCR_EditorManagerCore managerCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		managerCore.Event_OnEditorManagerInitOwner.Remove(OnPlayerRegistered);
		
		// Register for blocklist update and request it
		GameBlocklist blocklist = GetGame().GetGameBlocklist();
		blocklist.OnBlockListUpdateInvoker.Insert(OnBlockedListUpdated);
		blocklist.UpdateBlockList();
	}

	//------------------------------------------------------------------------------------------------
	void OnBlockedListUpdated(bool success)
	{
		if (!success)
		{
			// Some logging would be nice
			return;
		}

		// Remove only when it succeeds
		GameBlocklist blocklist = GetGame().GetGameBlocklist();
		blocklist.OnBlockListUpdateInvoker.Remove(OnBlockedListUpdated);

		// Request all the authors
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;
		
		core.Event_OnAuthorsRegisteredFinished.Insert(OnAuthorsRequestFinished);
		core.RequestAllAuthors();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnAuthorsRequestFinished(set<SCR_EditableEntityAuthor> activeUGCAuthors)
	{	
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;
		
		core.Event_OnAuthorsRegisteredFinished.Remove(OnAuthorsRequestFinished);
		
		PrintFormat("SCR_PlayerController::OnAuthorsRequestFinished - Number of UGC Authors: %1", activeUGCAuthors.Count(), level: LogLevel.VERBOSE);
		
		GameBlocklist blocklist = GetGame().GetGameBlocklist();
		
		array<string> identityIDs = {};
		foreach (SCR_EditableEntityAuthor author : activeUGCAuthors)
		{
			identityIDs.Insert(author.m_sAuthorUID);
		}
		PlayerManager.s_OnPlayerNameCacheUpdateInvoker.Insert(OnPlayerNameCacheUpdate);
		PlayerManager.RequestPlayerNameCacheUpdate(identityIDs);
		
		bool isBlockedAuthorPresent;
		foreach (SCR_EditableEntityAuthor author : activeUGCAuthors)
		{
			const bool isBlocked = blocklist.IsBlockedIdentity(author.m_sAuthorUID, author.m_ePlatform, author.m_sAuthorPlatformID);
			if (isBlocked)
			{
				PrintFormat(
					"Blocked user left UGC in this world {identity: %1; account: %2; platform: %3}",
					author.m_sAuthorUID,
					author.m_sAuthorPlatformID,
					author.m_ePlatform,
					level: LogLevel.VERBOSE
				);
				isBlockedAuthorPresent = true;
				break;
			}
		}
		
		if (isBlockedAuthorPresent)
		{					
			SCR_ConfigurableDialogUi dialog = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "blocked_ugc_present");
			dialog.m_OnCancel.Insert(DisconnectFromGame);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnPlayerNameCacheUpdate(bool success)
	{
		Print("SCR_PlayerController - Name Cache Updated!", LogLevel.VERBOSE);
		PlayerManager.s_OnPlayerNameCacheUpdateInvoker.Remove(OnPlayerNameCacheUpdate);
	}
	
	//------------------------------------------------------------------------------------------------
	void DisconnectFromGame()
	{
		ChimeraWorld world = GetGame().GetWorld();
		world.PauseGameTime(false);
		
		GameStateTransitions.RequestGameplayEndTransition();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBlockedPlayerJoined(int playerID)
	{
		SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_ON_BLOCKLIST_JOINED, playerID);
	}

	//------------------------------------------------------------------------------------------------
	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		m_OnControlledEntityChanged.Invoke(from, to);

		ChimeraCharacter character = ChimeraCharacter.Cast(to);
		if (character)
			m_CharacterController = character.GetCharacterController();
		else
			m_CharacterController = null;

		SetGameUserSettings();

		UpdateTurretFireModeControlls(from, to);
		SCR_AimSensitivitySettings.SetAimSensitivitySettings();
		SCR_HeadTrackingSettings.SetHeadTrackingSettings();
		SCR_ControllerSettings.SetControllerSettings();
	}

	protected void UpdateTurretFireModeControlls(IEntity from, IEntity to)
	{
		ChimeraCharacter abandonedCharacter = ChimeraCharacter.Cast(from);
		SCR_CompartmentAccessComponent compartmentAccessComp;
		SCR_FireModeManagerComponent fireModeMgr;
		if (abandonedCharacter)
		{
			compartmentAccessComp = SCR_CompartmentAccessComponent.Cast(abandonedCharacter.GetCompartmentAccessComponent());
			fireModeMgr = compartmentAccessComp.GetControlledFireModeManager();
			if (fireModeMgr)
				fireModeMgr.RemoveActionListeners();
		}

		ChimeraCharacter possessedCharacter = ChimeraCharacter.Cast(to);
		if (!possessedCharacter)
			return;

		compartmentAccessComp = SCR_CompartmentAccessComponent.Cast(possessedCharacter.GetCompartmentAccessComponent());
		fireModeMgr = compartmentAccessComp.GetControlledFireModeManager();
		if (fireModeMgr)
			fireModeMgr.SetUpAllActionListeners(possessedCharacter);
	}

	//------------------------------------------------------------------------------------------------
	static void SetGameUserSettings()
	{
		BaseContainer gameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");

		if (gameplaySettings)
		{
			bool stickyADS;
			if (gameplaySettings.Get("m_bStickyADS", stickyADS))
				CharacterControllerComponent.SetStickyADS(stickyADS);

			bool stickyGadgets;
			if (gameplaySettings.Get("m_bStickyGadgets", stickyGadgets))
				CharacterControllerComponent.SetStickyGadget(stickyGadgets);

			bool mouseControlAircraft;
			if (gameplaySettings.Get("m_bMouseControlAircraft", mouseControlAircraft))
				CharacterControllerComponent.SetMouseControlAircraft(mouseControlAircraft);

			bool gamepadFreelookInAircraft;
			if (gameplaySettings.Get("m_bGamepadFreelookInAircraft", gamepadFreelookInAircraft))
				CharacterControllerComponent.SetGamepadControlAircraft(!gamepadFreelookInAircraft);

			EVehicleDrivingAssistanceMode drivingAssistance;
			if (gameplaySettings.Get("m_eDrivingAssistance", drivingAssistance))
				VehicleControllerComponent.SetDrivingAssistanceMode(drivingAssistance);
		}

		BaseContainer fovSettings = GetGame().GetGameUserSettings().GetModule("SCR_FieldOfViewSettings");
		if (fovSettings)
		{
			float focusInADS;
			if (fovSettings.Get("m_fFocusInADS", focusInADS))
				s_fADSFocus = focusInADS;

			float focusInPIP;
			if (fovSettings.Get("m_fFocusInPIP", focusInPIP))
				s_fPIPFocus = focusInPIP;
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set entity which will be possessed by player.
	Possed entity is controlled by player, but it's not *the* player.
	\param entity Entity to be possessed, or null to return control back to the original player
	*/
	void SetPossessedEntity(IEntity entity)
	{
		if (!m_bIsPossessing)
		{
			if (entity)
			{
				m_OnBeforePossess.Invoke(entity);
				//--- Start posessing
				m_bIsPossessing = true;

				//--- Remember previously controlled entity
				IEntity controlledEntity = GetControlledEntity();
				m_MainEntityID = RplId.Invalid();
				if (controlledEntity)
				{
					RplComponent rpl = RplComponent.Cast(controlledEntity.FindComponent(RplComponent));
					if (rpl)
					{
						rpl.GiveExt(RplIdentity.Local(), false);
						m_MainEntityID = rpl.Id();
					}
				}

				OnRplMainEntityFromID(); //--- ToDo: Remove? BumpMe should call it automatically.
				Replication.BumpMe();

				//-- Tell manager we're possessing an entity
				SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
				if (possessingManager)
					possessingManager.SetMainEntity(GetPlayerId(), entity, controlledEntity, m_bIsPossessing);

				//--- Switch control
				RplComponent rpl = RplComponent.Cast(entity.FindComponent(RplComponent));
				if (rpl)
					rpl.GiveExt(GetRplIdentity(), false);
				SetAIActivation(entity, false);
				SetControlledEntity(entity);
				m_OnPossessed.Invoke(entity);
			}
		}
		else
		{
			if (!entity)
			{
				//--- Stop possessing
				m_bIsPossessing = false;

				//--- Forget main entity
				m_MainEntityID = RplId.Invalid();
				OnRplMainEntityFromID(); //--- ToDo: Remove?
				Replication.BumpMe();

				SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
				if (possessingManager)
					possessingManager.SetMainEntity(GetPlayerId(), GetControlledEntity(), m_MainEntity, m_bIsPossessing);

				//--- Switch control
				IEntity controlledEntity = GetControlledEntity();
				if (controlledEntity)
				{
					RplComponent rpl = RplComponent.Cast(controlledEntity.FindComponent(RplComponent));
					if (rpl)
						rpl.GiveExt(RplIdentity.Local(), false);

					SetAIActivation(controlledEntity, true);
				}

				//--- Switch control
				if (m_MainEntity)
				{
					RplComponent rpl = RplComponent.Cast(m_MainEntity.FindComponent(RplComponent));
					if (rpl)
						rpl.GiveExt(GetRplIdentity(), false);
				}
				SetControlledEntity(m_MainEntity);
				m_OnPossessed.Invoke(m_MainEntity);

				//--- SetControlledEntity(null) doesn't work yet. ToDo: Remove this check once it's implemented
				if (GetControlledEntity() != m_MainEntity)
					Print(string.Format("Error when switching control back to m_MainEntity = %1!", m_MainEntity), LogLevel.WARNING);
			}
			else
			{
				//--- Switch possessing
				SetPossessedEntity(null);
				SetPossessedEntity(entity);
				m_OnPossessed.Invoke(entity);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set intial main entity of a player, for a case where an existing entity should be assigned instead of spawning a new one
	\param entity is the subject entity
	*/
	void SetInitialMainEntity(notnull IEntity entity)
	{
		RplComponent rpl = RplComponent.Cast(entity.FindComponent(RplComponent));
		if (!rpl)
			return;

		m_MainEntityID = rpl.Id();
		OnRplMainEntityFromID();
		Replication.BumpMe();

		SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
		if (possessingManager)
			possessingManager.SetMainEntity(GetPlayerId(), GetControlledEntity(), entity, m_bIsPossessing);

		rpl.GiveExt(GetRplIdentity(), false); // transfer ownership
		SetAIActivation(entity, false);
		SetControlledEntity(entity);

		m_OnPossessed.Invoke(entity);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Check if player is currently possessing an entity.
	\return True when possessing
	*/
	bool IsPossessing()
	{
		return m_bIsPossessing;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get player's main entity.
	When not possessing, this will be the same as GetControlledEntity()
	When possessing, this will be player's main entity which was controlled before possessing started
	\return Main player entity
	*/
	IEntity GetMainEntity()
	{
		if (m_bIsPossessing)
			return m_MainEntity;
		else
			return GetControlledEntity();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRplMainEntityFromID()
	{
		//m_MainEntity = IEntity.Cast(Replication.FindItem(m_MainEntityID));
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(m_MainEntityID));
		if (rpl)
			m_MainEntity = rpl.GetEntity();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetAIActivation(IEntity entity, bool activate)
	{
		if (!entity)
			return;

		AIControlComponent aiControl = AIControlComponent.Cast(entity.FindComponent(AIControlComponent));
		if (!aiControl)
			return;

		if (activate)
			aiControl.ActivateAI();
		else
			aiControl.DeactivateAI();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns either a valid ID of local player or 0
	static int GetLocalPlayerId()
	{
		PlayerController pPlayerController = GetGame().GetPlayerController();
		if (!pPlayerController)
			return 0;

		return pPlayerController.GetPlayerId();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get entity controlled by player on this machine.
	\return Controlled entity
	*/
	static IEntity GetLocalControlledEntity()
	{
		PlayerController pPlayerController = GetGame().GetPlayerController();
		if (pPlayerController)
			return pPlayerController.GetControlledEntity();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get player's main entity on this machine.
	When not possessing, this will be the same as GetControlledEntity()
	When possessing, this will be player's main entity which was controlled before possessing started
	\return Main player entity
	*/
	static IEntity GetLocalMainEntity()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerController)
			return playerController.GetMainEntity();
		else
			return null;
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// TODO: Use getter for faction affiliation component
	//------------------------------------------------------------------------------------------------
	/*!
	Get faction of currently controlled local player entity.
	\return Faction
	*/
	static Faction GetLocalControlledEntityFaction()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return null;

		IEntity controlledEntity = playerController.GetControlledEntity();
		if (!controlledEntity)
			return null;

		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(controlledEntity.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
			return factionAffiliation.GetAffiliatedFaction();
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get faction of local player's main entity.
	When not possessing, the entity will be the same as GetControlledEntity()
	When possessing, the entity will be player's main entity which was controlled before possessing started
	\return Faction
	*/
	static Faction GetLocalMainEntityFaction()
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return null;

		IEntity controlledEntity = playerController.GetMainEntity();
		if (!controlledEntity)
			return null;

		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(controlledEntity.FindComponent(FactionAffiliationComponent));
		if (factionAffiliation)
			return factionAffiliation.GetAffiliatedFaction();
		else
			return null;
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	override void OnDestroyed(notnull Instigator killer)
	{
		super.OnDestroyed(killer);
		IEntity killerEntity = killer.GetInstigatorEntity();
		m_OnDestroyed.Invoke(killer, killerEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float timeSlice)
	{
		if (!s_pLocalPlayerController)
			UpdateLocalPlayerController();

		if (m_bIsLocalPlayerController)
		{
			UpdateControls();
			//UpdateUI();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Find if this is local player controller. We assume that this never changes during scenario.
	protected void UpdateLocalPlayerController()
	{
		m_bIsLocalPlayerController = this == GetGame().GetPlayerController();
		if (!m_bIsLocalPlayerController)
			return;

		s_pLocalPlayerController = this;
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;

		inputManager.AddActionListener("WeaponChangeMagnification", EActionTrigger.VALUE, ChangeMagnification);
		inputManager.AddActionListener("CharacterWalk", EActionTrigger.DOWN, OnWalk);
		inputManager.AddActionListener("CharacterWalk", EActionTrigger.UP, OnEndWalk);
		inputManager.AddActionListener("FocusToggle", EActionTrigger.DOWN, ActionFocusToggle);
		inputManager.AddActionListener("FocusToggleVehicle", EActionTrigger.DOWN, ActionFocusToggleVehicle);
		inputManager.AddActionListener("FocusToggleUnarmed", EActionTrigger.DOWN, ActionFocusToggleUnarmed);
		inputManager.AddActionListener("Inventory", EActionTrigger.DOWN, ActionOpenInventory);
		inputManager.AddActionListener("TacticalPing", EActionTrigger.DOWN, ActionGesturePing);
		inputManager.AddActionListener("TacticalPingHold", EActionTrigger.DOWN, ActionGesturePingHold);
		inputManager.AddActionListener("TacticalPingHold", EActionTrigger.UP, ActionGesturePingHold);
		inputManager.AddActionListener("WeaponSwitchOptics", EActionTrigger.UP, ChangeWeaponOptics);
	}

	//------------------------------------------------------------------------------------------------
	//! Update disabling of character controls in menus
	protected void UpdateControls()
	{
		bool disableControls = GetGame().GetMenuManager().IsAnyMenuOpen();
		if (m_bIsPaused != disableControls)
		{
			m_bIsPaused = disableControls;
			SetDisableControls(disableControls);
		}
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// TODO: This is obsolete
	//------------------------------------------------------------------------------------------------
	protected void UpdateUI()
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetControlledEntity());
		if (!char)
			return;
		CharacterAnimationComponent animComp = char.GetAnimationComponent();
		if (!animComp)
			return;
		// Command ladder is present only when character is using ladder
		CharacterCommandLadder ladderCMD = animComp.GetCommandHandler().GetCommandLadder();
		if (!ladderCMD)
			return;
		int lrExitState = ladderCMD.CanExitLR();
		if (lrExitState & 0x1)
		{
			Print("Can exit right");
		}
		if (lrExitState & 0x2)
		{
			Print("Can exit left");
		}
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	protected void ChangeMagnification(float value)
	{
		SCR_CharacterControllerComponent characterController = GetCharacterController();
		if (characterController)
			characterController.SetNextSightsFOVInfo(value);
	}

	//------------------------------------------------------------------------------------------------
	protected void ChangeWeaponOptics()
	{
		SCR_CharacterControllerComponent characterController = GetCharacterController();
		if (characterController)
			characterController.SetNextSights();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CharacterControllerComponent GetCharacterController()
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(GetControlledEntity());
		if (!char)
			return null;

		return SCR_CharacterControllerComponent.Cast(char.GetCharacterController());
	}

	//------------------------------------------------------------------------------------------------
	// Parameter value:
	// TRUE:  Disables the controls
	// FALSE: Enables the controls
	private void SetDisableControls(bool value)
	{
		SCR_CharacterControllerComponent characterController = GetCharacterController();
		if (!characterController)
			return;

		characterController.SetDisableViewControls(value);
		characterController.SetDisableWeaponControls(value);
		characterController.SetDisableMovementControls(value)
	}

	//------------------------------------------------------------------------------------------------
	/*! Focus input degree for analogue input
	\param adsProgress ADS focus percentage
	\param dt Delta time
	\return focus Amount of focus between 0 and 1
	*/
	float GetFocusValue(float adsProgress = 0, float dt = -1)
	{
		if (!m_CharacterController)

			return 0;
		float focus;

		// Autofocus
		if (adsProgress > 0)
		{
			// PIPCQB Separate slider
			if (SCR_2DPIPSightsComponent.IsPIPActive())
				focus = Math.Lerp(s_fADSFocus, 1, s_fPIPFocus);
			else
				focus = s_fADSFocus;

			focus *= Math.Min(adsProgress, 1);

			if (m_CharacterController.IsFreeLookEnabled())
			{
				// Freelook angle effect on ADS focus
				CharacterHeadAimingComponent headAiming = m_CharacterController.GetHeadAimingComponent();
				if (headAiming)
				{
					float freelookAngle = headAiming.GetAimingRotation().Length();
					float freelookFocus = 1 - Math.InverseLerp(1, 6, freelookAngle);
					focus *= Math.Clamp(freelookFocus, 0, 1);
				}
			}
		}

		InputManager inputManager = GetGame().GetInputManager();

		// Cancel toggled focus when focus is held
		bool inputDigital = inputManager.GetActionTriggered("Focus");
		if (inputDigital && m_eFocusToggle != SCR_EFocusToggleMode.DISABLED)
			m_eFocusToggle = SCR_EFocusToggleMode.DISABLED;

		// Conditions must be consistent with ActionFocusToggle and ActionFocusToggleUnarmed
		ChimeraCharacter character = m_CharacterController.GetCharacter();
		if (character && character.IsInVehicle())
		{
			// Vehicle focus toggle mode
			if (m_eFocusToggle == SCR_EFocusToggleMode.VEHICLE)
			{
				// Cancel toggle focus when in vehicle and aiming through gadget (binocular, compass)
				if (m_bGadgetFocus)
					m_eFocusToggle = SCR_EFocusToggleMode.DISABLED;

				// Cancel toggle focus when in vehicle and not in forced freelook
				if (!m_CharacterController.IsFreeLookEnabled() && !m_CharacterController.GetFreeLookInput())
					m_eFocusToggle = SCR_EFocusToggleMode.DISABLED;
			}
		}
		else
		{
			// Unarmed focus toggle mode
			if (m_eFocusToggle == SCR_EFocusToggleMode.UNARMED)
			{
				// Cancel toggle focus when not in vehicle and holding item in hands
				if (m_CharacterController.GetCurrentItemInHands())
					m_eFocusToggle = SCR_EFocusToggleMode.DISABLED;

				// Cancel toggle focus when not in vehicle and holding gadget
				if (m_CharacterController.IsGadgetInHands())
					m_eFocusToggle = SCR_EFocusToggleMode.DISABLED;
			}
		}

		// Vehicles have different focus action to prevent conflict with brakes
		bool isVehicleContextActive = inputManager.IsContextActive("CarContext") || inputManager.IsContextActive("TrackedContext") || inputManager.IsContextActive("HelicopterContext");
		float inputAnalogue;
		if (!isVehicleContextActive)
		{
			// Square root input to focus mapping results in linear change of picture area
			float focusAnalogue = Math.Sqrt(FOCUS_ANALOGUE_SCALE * inputManager.GetActionValue("FocusAnalog"));

			// Tolerance to prevent jittering
			if (focusAnalogue < FOCUS_DEACTIVATION)
				s_fFocusAnalogue = 0;
			else if (!float.AlmostEqual(s_fFocusAnalogue, focusAnalogue, FOCUS_TOLERANCE))
				s_fFocusAnalogue = focusAnalogue;

			inputAnalogue = s_fFocusAnalogue;
		}

		bool isADS = m_CharacterController.GetWeaponADSInput();

		// Check gadget ADS
		if (!isADS && m_CharacterController.IsGadgetInHands())
			isADS = m_CharacterController.IsGadgetRaisedModeWanted();

		// Verify turrets
		if (!isADS && character)
		{
			CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();

			BaseCompartmentSlot compartment;
			if (compartmentAccess)
				compartment = compartmentAccess.GetCompartment();

			TurretControllerComponent turretController;
			if (compartment)
				turretController = TurretControllerComponent.Cast(compartment.GetController());

			if (turretController)
				isADS = turretController.IsWeaponADS();
		}

		// Prevent focus warping back while toggling ADS on controller
		// analogue: track timeout as we have no input filter that has thresholds or delays and returns axis value yet
		if (inputAnalogue < FOCUS_DEACTIVATION)
			s_fFocusTimeout = FOCUS_TIMEOUT; // Below deactivation threshold
		else if (s_bWasADS != isADS && s_fFocusTimeout > 0)
			s_fFocusTimeout = FOCUS_TIMEOUT; // ADS toggled
		else if (inputAnalogue < FOCUS_ACTIVATION && s_fFocusTimeout > 0)
			s_fFocusTimeout = FOCUS_TIMEOUT; // Below activation threshold and not active
		else if (s_fFocusTimeout > dt)
			s_fFocusTimeout -= dt; // Not yet active, decrementing
		else
			s_fFocusTimeout = 0; // Activated

		// Cancel toggle focus with analogue input
		if (m_eFocusToggle != SCR_EFocusToggleMode.DISABLED && s_fFocusTimeout == 0)
			m_eFocusToggle = SCR_EFocusToggleMode.DISABLED;

		s_bWasADS = isADS;

		// Combine all valid input sources
		float input;
		if (m_eFocusToggle != SCR_EFocusToggleMode.DISABLED || inputDigital)
			input = 1;
		else if (s_fFocusTimeout == 0)
			input = inputAnalogue;

		if (input > 0)
			focus = Math.Max(focus, input);

		// Ensure return value always within 0-1
		focus = Math.Clamp(focus, 0, 1);
		return focus;
	}

	//------------------------------------------------------------------------------------------------
	//! Automatic focusing while gadget is aimed down sights
	void SetGadgetFocus(bool gadgetFocus)
	{
		m_bGadgetFocus = gadgetFocus;
	}

	//------------------------------------------------------------------------------------------------
	//! Automatic focusing while gadget is aimed down sights
	bool GetGadgetFocus()
	{
		return m_bGadgetFocus;
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//! TODO: Action listener methods should be protected
	//------------------------------------------------------------------------------------------------
	void ActionFocusToggle(float value = 0.0, EActionTrigger reason = 0)
	{
		// If focus toggle was in different mode, it should be disabled first
		if (m_eFocusToggle == SCR_EFocusToggleMode.DISABLED)
			m_eFocusToggle = SCR_EFocusToggleMode.ENABLED;
		else
			m_eFocusToggle = SCR_EFocusToggleMode.DISABLED
	}

	//------------------------------------------------------------------------------------------------
	void ActionFocusToggleVehicle(float value = 0.0, EActionTrigger reason = 0)
	{
		if (!m_CharacterController)
			return;

		// Conditions must be consistent with GetFocusValue logic
		ChimeraCharacter character = m_CharacterController.GetCharacter();
		if (character && !character.IsInVehicle())
			return;

		// Cancel toggle focus when in vehicle and aiming through gadget (binocular, compass)
		if (m_bGadgetFocus)
			return;

		// Cancel toggle focus when in vehicle and not in forced freelook
		if (!m_CharacterController.IsFreeLookEnabled() && !m_CharacterController.GetFreeLookInput())
			return;

		// If focus toggle was in different mode, it should be disabled first
		if (m_eFocusToggle == SCR_EFocusToggleMode.DISABLED)
			m_eFocusToggle = SCR_EFocusToggleMode.VEHICLE;
		else
			m_eFocusToggle = SCR_EFocusToggleMode.DISABLED
	}

	//------------------------------------------------------------------------------------------------
	void ActionFocusToggleUnarmed(float value = 0.0, EActionTrigger reason = 0)
	{
		if (!m_CharacterController)
			return;

		// Conditions must be consistent with GetFocusValue logic
		ChimeraCharacter character = m_CharacterController.GetCharacter();
		if (character && character.IsInVehicle())
			return;

		// Cancel toggle focus when not in vehicle and holding item in hands
		if (m_CharacterController.GetCurrentItemInHands())
			return;

		// Cancel toggle focus when not in vehicle and holding gadget
		if (m_CharacterController.IsGadgetInHands())
			return;

		// Allow cancelling unarmed focus while picking up items
		// Disallow enabling unarmed focus while picking up items, as it may become irreleant quickly
		// Reason is player may want to enter ADS before ready, while intent is unclear
		if (m_eFocusToggle == SCR_EFocusToggleMode.DISABLED && m_CharacterController.IsPlayingItemGesture())
			return;
		
		// If focus toggle was in different mode, it should be disabled first
		if (m_eFocusToggle == SCR_EFocusToggleMode.DISABLED)
			m_eFocusToggle = SCR_EFocusToggleMode.UNARMED;
		else
			m_eFocusToggle = SCR_EFocusToggleMode.DISABLED
	}

	//------------------------------------------------------------------------------------------------
	void OnWalk()
	{
		if (!m_CharacterController || m_CharacterController.GetDynamicSpeed() == WALK_SPEED)
			return;

		m_fCharacterSpeed = m_CharacterController.GetDynamicSpeed();
		m_CharacterController.SetDynamicSpeed(WALK_SPEED);
		m_CharacterController.SetShouldApplyDynamicSpeedOverride(true);
	}

	//------------------------------------------------------------------------------------------------
	void OnEndWalk()
	{
		if (!m_CharacterController || m_CharacterController.GetDynamicSpeed() == m_fCharacterSpeed)
			return;

		m_CharacterController.SetDynamicSpeed(m_fCharacterSpeed);
		m_CharacterController.SetShouldApplyDynamicSpeedOverride(false);
	}

	//------------------------------------------------------------------------------------------------
	void ActionOpenInventory()
	{
		IEntity entity = s_pLocalPlayerController.GetControlledEntity();
		if (!entity)
			return;

		SCR_InventoryStorageManagerComponent inventory = SCR_InventoryStorageManagerComponent.Cast(entity.FindComponent(SCR_InventoryStorageManagerComponent));
		if (inventory)
			inventory.Action_OpenInventory();
	}

	//------------------------------------------------------------------------------------------------
	void ActionGesturePing(float value = 0.0, EActionTrigger reason = 0)
	{
		if (!m_CharacterController)
			return;

		// Press and forget variant... eg press comma once - character will point with it's finger for 1 second (including blending time from animation graph ~300ms)
		m_CharacterController.TryStartCharacterGesture(ECharacterGestures.POINT_WITH_FINGER, 1500);
	}

	//------------------------------------------------------------------------------------------------
	void ActionGesturePingHold(float value = 0.0, EActionTrigger reason = 0)
	{
		if (!m_CharacterController)
			return;

		// Hold key variant... hold period - character will point with it's finger until period key is released
		if (reason == EActionTrigger.DOWN)
		{
			m_CharacterController.TryStartCharacterGesture(ECharacterGestures.POINT_WITH_FINGER);
		} else if (reason == EActionTrigger.UP)
		{
			m_CharacterController.StopCharacterGesture();
		}
	}
	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	// Set the correct platfrom icon to the provided Image Widget
	//! \param playerID
	//! \param image
	//! \param glowImage
	//! \param set image Visible
	//! \param show on PC
	//! \param show on Xbox
	bool SetPlatformImageTo(int playerID, notnull ImageWidget image, ImageWidget glow = null, bool setVisible = true, bool showOnPC = false, bool showOnXbox = false)
	{		
		PlayerManager playerMgr = GetGame().GetPlayerManager();
		if (!playerMgr)
			return false;
		
		PlatformKind targetPlatformKind = playerMgr.GetPlatformKind(playerID);
		PlatformKind ownPlatformKind = playerMgr.GetPlatformKind(GetLocalPlayerId());
		
		// Always show on PSN
		if (ownPlatformKind == PlatformKind.PSN	)
			SetPlatformImagePSN(targetPlatformKind, image, glow, setVisible);
		else if (!showOnPC && !showOnXbox)
			return false;
		
		switch (ownPlatformKind)
		{
			case PlatformKind.STEAM:
				if (showOnPC)
					SetPlatformImagePC(targetPlatformKind, image, glow, setVisible);
			break;
			
			case PlatformKind.XBOX:
				if (showOnXbox)
					SetPlatformImageXbox(targetPlatformKind, image, glow, setVisible);
			break;
			
			case PlatformKind.NONE:
				if (showOnPC)
					SetPlatformImagePC(targetPlatformKind, image, glow, setVisible);
			break;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Set the correct platfrom icon to the provided Image Widget based on PlatformKind
	//! \param platform
	//! \param image
	//! \param glowImage
	//! \param set image Visible
	//! \param show on PC
	//! \param show on Xbox
	bool SetPlatformImageToKind(PlatformKind targetPlatform, notnull ImageWidget image, ImageWidget glow = null, bool setVisible = true, bool showOnPC = false, bool showOnXbox = false)
	{
		PlatformKind ownPlatformKind = GetGame().GetPlayerManager().GetPlatformKind(GetLocalPlayerId());
		
		// Always show on PSN
		if (ownPlatformKind == PlatformKind.PSN)
			SetPlatformImagePSN(targetPlatform, image, glow, setVisible);
		
		if (!showOnPC && !showOnXbox)
			return true;
		
		switch (ownPlatformKind)
		{
			case PlatformKind.STEAM:
				if (showOnPC)
					SetPlatformImagePC(targetPlatform, image, glow, setVisible);
			break;
			
			case PlatformKind.XBOX:
				if (showOnXbox)
					SetPlatformImageXbox(targetPlatform, image, glow, setVisible);
			break;
			
			case PlatformKind.NONE:
				if (showOnPC)
					SetPlatformImagePC(targetPlatform, image, glow, setVisible);
			break;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetPlatformImagePSN(PlatformKind targetPlatformKind, notnull ImageWidget image, ImageWidget glow = null, bool setVisible = true)
	{
		if (targetPlatformKind == PlatformKind.PSN)
		{
			image.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_PLAYSTATION_ICON_NAME);
			
			if (glow)
				glow.LoadImageFromSet(0, UIConstants.ICONS_GLOW_IMAGE_SET, UIConstants.PLATFROM_PLAYSTATION_ICON_NAME);
		}
		else
		{
			image.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_GENERIC_ICON_NAME);
			
			if (glow)
				glow.LoadImageFromSet(0, UIConstants.ICONS_GLOW_IMAGE_SET, UIConstants.PLATFROM_GENERIC_ICON_NAME);
		}
		
		if (!setVisible)
			return;
		
		image.SetVisible(true);
		if (glow)
			glow.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetPlatformImagePC(PlatformKind targetPlatformKind, notnull ImageWidget image, ImageWidget glow = null, bool setVisible = true)
	{
		string icon;
		switch (targetPlatformKind)
		{
			case PlatformKind.STEAM:
				icon = UIConstants.PLATFROM_PC_ICON_NAME;
			break;
			case PlatformKind.XBOX:
				icon = UIConstants.PLATFROM_XBOX_ICON_NAME;
			break;
			case PlatformKind.PSN:
				icon = UIConstants.PLATFROM_PLAYSTATION_ICON_NAME;
			break;
			case PlatformKind.NONE:
				icon = UIConstants.PLATFROM_PC_ICON_NAME;
			break;
		}
		
		image.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, icon);
		if (glow)
			glow.LoadImageFromSet(0, UIConstants.ICONS_GLOW_IMAGE_SET, icon);
		
		if (!setVisible)
			return;
		
		image.SetVisible(true);
		if (glow)
			glow.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetPlatformImageXbox(PlatformKind targetPlatformKind, notnull ImageWidget image, ImageWidget glow = null, bool setVisible = true)
	{
		string icon;
		switch (targetPlatformKind)
		{
			case PlatformKind.STEAM:
				icon = UIConstants.PLATFROM_PC_ICON_NAME;
			break;
			case PlatformKind.XBOX:
				icon = UIConstants.PLATFROM_XBOX_ICON_NAME;
			break;
			case PlatformKind.PSN:
				icon = UIConstants.PLATFROM_PLAYSTATION_ICON_NAME;
			break;
			case PlatformKind.NONE:
				icon = UIConstants.PLATFROM_PC_ICON_NAME;
			break;
		}
		
		image.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, icon);
		if (glow)
			glow.LoadImageFromSet(0, UIConstants.ICONS_GLOW_IMAGE_SET, icon);
		
		if (!setVisible)
			return;
		
		image.SetVisible(true);
		if (glow)
			glow.SetVisible(true);
	}
}
