class SCR_InteractionHandlerComponentClass : InteractionHandlerComponentClass
{
}

//! Provides an identification for SCR_InteractionHandlerComponent which type of input to use.
//! This is added for convenience of quick switching of different methods.
enum SCR_NearbyContextDisplayMode
{
	DISABLED = 0, 			//!< Nearby display will never be allowed.
	ALWAYS_ON = 1,			//!< Nearby display will always be on (when possible).
	ON_INPUT_ACTION = 2,	//!< Nearby display will show on provided input action.
	ON_FREELOOK = 3			//!< Nearby display will show when controlled entity is in freelook.
}

//! This component allows the player to interact with their environment.
//! It collects UserActionContext from ActionsManagerComponent from surrounding entities via queries,
//! filters and finds the most appropriate one and provides script API to work with them.
//!
//! It should always be attached to PlayerController entity and is local only.
class SCR_InteractionHandlerComponent : InteractionHandlerComponent
{
	//! Display (action menu) used to show UI to the player regarding collected actions.
	protected SCR_BaseInteractionDisplay m_pDisplay;

	[Attribute("3", UIWidgets.ComboBox, "Display mode", "", ParamEnumArray.FromEnum(SCR_NearbyContextDisplayMode), category: "Nearby Context Properties")]
	protected SCR_NearbyContextDisplayMode m_eDisplayMode;

	[Attribute("1", UIWidgets.Slider, "Distance in percentage the raycast needs to travel until the context counts as visible", "0 1 0.01", category: "Nearby Context Properties")]
	protected float m_fRaycastThreshold;

	[Attribute("", UIWidgets.EditBox, "Action to listen for when SCR_NearbyContextDisplayMode is set to ON_INPUT_ACTION", category: "Nearby Context Properties")]
	protected string m_sActionName;

	[Attribute("", UIWidgets.EditBox, "Context to activate when SCR_NearbyContextDisplayMode is set to ON_INPUT_ACTION. Mustn't be empty to be activated.", category: "Nearby Context Properties")]
	protected string m_sActionContext;

	//! Last selected context
	protected UserActionContext m_pLastContext;

	//! Last selected user action
	protected BaseUserAction m_pLastUserAction;

	//! Currently selected action index
	protected int m_iSelectedActionIndex;

	protected bool m_bIsPerforming;
	protected bool m_bPerformAction;
	protected bool m_bLastInput;
	protected float m_fSelectAction;
	protected float m_fCurrentProgress;

	//! List of collected entities from which the available contexts will be taken to dispaly (weapon, vehicle, ...)
	protected ref array<IEntity> m_aCollectedEntities = {};
	//! List of collected nearby entities from which the available contexts will be taken to dispaly (weapon, vehicle, ...)
	protected ref array<IEntity> m_aCollectedNearbyEntities = {};

	protected IEntity m_ControlledEntity;

	//------------------------------------------------------------------------------------------------
	//! Action listeners that are meant to be registered once per user and only for the user who is the owner of this controller
	protected void RegisterActionListeners()
	{
		InputManager pInputManager = GetGame().GetInputManager();
		if (!pInputManager)
			return;

		m_bPerformAction = false;
		m_fSelectAction = 0;
		pInputManager.AddActionListener("PerformAction", EActionTrigger.DOWN, ActionPerform);
		pInputManager.AddActionListener("PerformAction", EActionTrigger.UP, ActionPerform);
		pInputManager.AddActionListener("SelectAction", EActionTrigger.VALUE, ActionScroll);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveActionListeners()
	{
		InputManager pInputManager = GetGame().GetInputManager();
		if (!pInputManager)
			return;

		m_bPerformAction = false;
		m_fSelectAction = 0;
		pInputManager.RemoveActionListener("PerformAction", EActionTrigger.DOWN, ActionPerform);
		pInputManager.RemoveActionListener("PerformAction", EActionTrigger.UP, ActionPerform);
		pInputManager.RemoveActionListener("SelectAction", EActionTrigger.VALUE, ActionScroll);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback for caching the key press
	void ActionPerform(float value, EActionTrigger reason)
	{
		m_bPerformAction = reason == EActionTrigger.DOWN;
	}

	//------------------------------------------------------------------------------------------------
	void ActionScroll(float value, EActionTrigger reason)
	{
		if (value == 0)
			return;

		m_fSelectAction = value;
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when player takes controll over an entity
	//! \param[in] from entity which was previously controlled
	//! \param[in] to entity which is now controlled
	override protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		PlayerController controller = GetGame().GetPlayerController();
		if (!controller)
			return;

		if (controller.FindComponent(SCR_InteractionHandlerComponent) != this)
			return;

		if (from)//Was an owner of the entity that contians this component but that is changing now
			RemoveActionListeners();

		if (to && to == SCR_PlayerController.GetLocalControlledEntity())//Became an owner of the entity that has this component
			RegisterActionListeners();
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_INTERACTION_SKIP_DURATION, "", "Skip action duration", "User Actions");
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_BaseInteractionDisplay FindDisplay(IEntity owner)
	{
		PlayerController playerController = PlayerController.Cast(owner);
		if (!playerController)
		{
			Print("InteractionHandler must be attached to a PlayerController!", LogLevel.ERROR);
			return null;
		}

		HUDManagerComponent hudManager = HUDManagerComponent.Cast(playerController.FindComponent(HUDManagerComponent));
		array<BaseInfoDisplay> displayInfos = {};
		int count = hudManager.GetInfoDisplays(displayInfos);
		for (int i = 0; i < count; i++)
		{
			SCR_BaseInteractionDisplay current = SCR_BaseInteractionDisplay.Cast(displayInfos[i]);
			if (current)
				return current;
		}

		Print("InteractionDisplay not found! InteractionDisplay must be stored in HUDManagerComponent of a PlayerController!", LogLevel.WARNING);
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks input, compares it to previous states and evaluates what the handler should do next.
	//! \param[in] user
	//! \param[in] context
	//! \param[in] action
	//! \param[in] canPerform
	//! \param[in] performInput
	//! \param[in] timeSlice
	//! \param[in] display
	protected void DoProcessInteraction(
		ChimeraCharacter user,
		UserActionContext context,
		BaseUserAction action,
		bool canPerform,
		bool performInput,
		float timeSlice,
		SCR_BaseInteractionDisplay display)
	{
		if (action)
			action.SetActiveContext(context);

		// Can action be performed?
		bool isOk = action && canPerform && action == m_pLastUserAction;
		// We want to perform and action is OK
		if (performInput && isOk)
		{
			// We want to be performing, but we're not yet.
			// Start the action and dispatch events.
			if (!m_bIsPerforming && !m_bLastInput)
			{
				if (!GetCanInteractScript(user))
					return;

				// UI
				if (display)
					display.OnActionStart(user, action);

				// Start the action. Calls action.OnActionStart
				user.DoStartObjectAction(action);

				// Set state
				m_bIsPerforming = true;
				m_fCurrentProgress = 0.0;
			}
			// We want to perform and we already started performing,
			// update continuous handler state until we're finished
			else if (m_bIsPerforming)
			{
				if (DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_INTERACTION_SKIP_DURATION))
				{
					timeSlice += Math.AbsFloat(action.GetActionDuration());

					SCR_ScriptedUserAction scrAction = SCR_ScriptedUserAction.Cast(action);
					if (scrAction)
						timeSlice += scrAction.GetLoopActionHoldDuration();
				}

				// Update elapsed time
				m_fCurrentProgress = action.GetActionProgress(m_fCurrentProgress, timeSlice);

				// Tick action
				if (action.ShouldPerformPerFrame())
					user.DoPerformContinuousObjectAction(action, timeSlice);

				// Get action duration
				float duration = action.GetActionDuration();
				
				// Update UI
				if (display)
					display.OnActionProgress(user, action, m_fCurrentProgress, Math.AbsFloat(duration));
				
				// We are finished, dispatch events and reset state
				// TODO: Why are some actions set with negative duration? Why does using an abs duration check here causes those actions to break? Why Is this not using a proper event system from the actions themselves?!
				if (m_fCurrentProgress >= duration && duration >= 0)
				{
					// Update UI
					if (display)
						display.OnActionFinish(user, action, ActionFinishReason.FINISHED);

					// Finally perform action
					if (!action.ShouldPerformPerFrame())
						user.DoPerformObjectAction(action);

					// Reset state
					m_fCurrentProgress = 0.0;
					m_bIsPerforming = false;
				}
			}
		}
		else
		{
			// Input was released, we were performing previously,
			// stop performing and dispatch necessary events.
			if (m_bIsPerforming)
			{
				// Update UI
				if (display)
					display.OnActionFinish(user, action, ActionFinishReason.INTERRUPTED);

				// Cancel the action. Calls action.OnActionCanceled
				user.DoCancelObjectAction(action);

				// Reset state
				m_bIsPerforming = false;
				m_fCurrentProgress = 0.0;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override bool GetCanInteractScript(IEntity controlledEntity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		if (!character)
			return false;

		// No interactions when menu is open
		MenuManager menuManager = GetGame().GetMenuManager();
		if (menuManager && menuManager.IsAnyMenuOpen())
			return false;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (characterController && !characterController.CanInteract())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool GetIsInteractionAvailableScript()
	{
		return IsContextAvailable();
	}

	//------------------------------------------------------------------------------------------------
	protected override BaseUserAction GetSelectedActionScript()
	{
		return m_pLastUserAction;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool DoIntersectCheck(IEntity controlledEntity)
	{
		if (!controlledEntity)
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);
		if (!character)
			return false;

		if (character.IsInVehicle())
			return true;

		if (character.GetCharacterController().GetInspect())
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnContextChanged(UserActionContext previousContext, UserActionContext newContext)
	{
		// Changed, so hide previous
		if (!newContext || newContext != GetCurrentContext())
		{
			m_iSelectedActionIndex = 0;
			if (m_pDisplay)
				m_pDisplay.HideDisplay();
		}

		// Changed, so show new
		if (m_pDisplay && newContext)
			m_pDisplay.ShowDisplay();
	}

	//------------------------------------------------------------------------------------------------
	protected override event bool CanContextChange(UserActionContext currentContext, UserActionContext newContext)
	{
		// Setting null context might be desirable in certain cases when state should be cleared, see below:
		if (!newContext)
		{
			// Allow clearing if no entity is controlled, to prevent leaking of contexts
			if (!m_ControlledEntity)
				return true;

			// Allow clearing of context if controlled entity is destroyed, at this point interaction should begone
			DamageManagerComponent dmg = DamageManagerComponent.Cast(m_ControlledEntity.FindComponent(DamageManagerComponent));
			if (dmg && dmg.IsDestroyed())
				return true;

			// Otherwise continue with the usual
		}

		// Check whether we are still in range
		if (currentContext && m_ControlledEntity)
		{
			// We will leave a small error threshold
			const float threshold = 1.1;
			// Global action visibility range in meters
			float visRange = currentContext.GetVisibilityRange(GetVisibilityRange());
			// Maximum sq distance we can interact at
			float maxSqDistance = (visRange * visRange) * 1.1;
			// Sq distance to controlled entity
			float sqDistance = vector.DistanceSq(currentContext.GetOrigin(), m_ControlledEntity.GetOrigin());

			if (sqDistance > maxSqDistance)
			{
				// We are out of range, context can change safely
				return true;
			}
		}

		// Suppress context changing when we are interacting with one already
		if (currentContext && m_bIsPerforming)
		{
			// TODO: Validate distance to ctx
			return false;
		}

		return true;
	}

	protected override event bool IsPerformingAction()
	{
		return m_bIsPerforming;
	}

	//------------------------------------------------------------------------------------------------
	//! display mode is set to automatic freelook mode.
	//! \return true if nearby action contexts should be shown when
	protected bool IsFreelookEnabled(ChimeraCharacter character)
	{
		if (!character)
			return false;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;

		// Inspection is priority
		if (controller.GetInspect())
			return true;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		// Hide blips when in TPP while in the vehicle
		if (compartmentAccess && compartmentAccess.IsInCompartment() && controller.IsInThirdPersonView())
			return false;

		// When forced, avoid displaying in certain cases
		// Suppress display when getting in our out
		if (compartmentAccess)
		{
			if (compartmentAccess.IsGettingIn() || compartmentAccess.IsGettingOut())
				return false;
		}

		// Supress display when in a vehicle while in 3rd person
		if (character.IsInVehicle())
		{
			if (controller.IsInThirdPersonView())
				return false;
		}

		// Supress display when falling
		if (controller.IsFalling())
			return false;

		// Or climbing
		if (controller.IsClimbing())
			return false;

		// Or unconscious
		if (controller.GetLifeState() != ECharacterLifeState.ALIVE)
			return false;

		if (controller.GetFreeLookInput())
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool ShouldBeEnabled(SCR_NearbyContextDisplayMode displayMode, ChimeraCharacter character, bool playerCameraOnly = true)
	{
		// Disallow when character is none
		if (!character)
			return false;

		// Disallow out of player camera when true
		if (playerCameraOnly)
		{
			CameraManager cameraManager = GetGame().GetCameraManager();
			if (cameraManager && !PlayerCamera.Cast(cameraManager.CurrentCamera()))
				return false;
		}

#ifdef NEARBY_INTERACTIONS_CONTEXT_DEBUG
		// If debug mode is active show them always to make debugging easier
		return true;
#endif

		// Handle different display mode cases
		switch (displayMode)
		{
			// Always off
			case SCR_NearbyContextDisplayMode.DISABLED:
				return false;

			// Always on
			case SCR_NearbyContextDisplayMode.ALWAYS_ON:
				return true;

			// On action
			case SCR_NearbyContextDisplayMode.ON_INPUT_ACTION:
			{
				if (m_sActionName.IsEmpty())
					return false;

				InputManager inputManager = GetGame().GetInputManager();
				if (!m_sActionContext.IsEmpty())
					inputManager.ActivateContext(m_sActionContext);

				return inputManager.GetActionValue(m_sActionName) > 0;
			}

			// When in freelook
			case SCR_NearbyContextDisplayMode.ON_FREELOOK:
				return IsFreelookEnabled(character);
		}

		// Nope, sorry.
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void GetOverrideListReferencePoint(IEntity owner, out vector referencePoint)
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (cameraManager)
		{
			CameraBase camera = cameraManager.CurrentCamera();
			vector rayDir = camera.GetWorldTransformAxis(2);
			vector rayStart = camera.GetOrigin();
			referencePoint = rayStart + rayDir;

			// Inspection correction
			ChimeraCharacter character = ChimeraCharacter.Cast(m_ControlledEntity);
			if (character)
			{
				// During inspection (of a weapon)
				CharacterControllerComponent controller = character.GetCharacterController();
				if (controller.GetInspect() && controller.GetInspectCurrentWeapon())
				{
					// Assume that while in inspection, weapon is tilted and its
					// left side is pointed towards the player camera
					IEntity inspectedEntity = controller.GetInspectEntity();

					vector origin = inspectedEntity.GetOrigin();
					vector normal = -inspectedEntity.GetWorldTransformAxis(0);

					referencePoint = SCR_Math3D.IntersectPlane(rayStart, rayDir, origin, normal);
					// Shape.CreateSphere(COLOR_RED, ShapeFlags.ONCE, referencePoint, 0.01);
				}
			}
		}
		else
		{
			referencePoint = vector.Zero;
		}
	}

	//------------------------------------------------------------------------------------------------
	override array<IEntity> GetManualNearbyOverrideList(IEntity owner, out vector referencePoint)
	{
		GetOverrideListReferencePoint(owner, referencePoint);
		return m_aCollectedNearbyEntities;
	}

	//------------------------------------------------------------------------------------------------
	override array<IEntity> GetManualOverrideList(IEntity owner, out vector referencePoint)
	{
		GetOverrideListReferencePoint(owner, referencePoint);
		return m_aCollectedEntities;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the player is inside a vehicle or is inspecting something and enable/disable ManualNearbyCollectionOverride and ManualCollectionOverride for the nearby contexts based on it.
	//! \param[in] character controlled by the player
	protected void HandleOverride(notnull ChimeraCharacter character)
	{
		m_aCollectedNearbyEntities.Clear();
		m_aCollectedEntities.Clear();

		const bool bInspection = HandleInspection(character);
		const bool bInVehicle = HandleVehicle(character);

		SetManualNearbyCollectionOverride(false);
		SetManualCollectionOverride(false);

		if (bInspection || bInVehicle)
		{
			SetManualNearbyCollectionOverride(true);
			if (bInspection)
				SetManualCollectionOverride(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Check is player is inside a vehicle and grab all the compartments of that vehicle
	//! \param[in] character controlled by the player
	//! \return True if player is inside a vehicle, false otherwise
	protected bool HandleVehicle(notnull ChimeraCharacter character)
	{
		if (!character.IsInVehicle())
			return false;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		// Check if player is inside a Vehicle
		if (!compartmentAccess || !compartmentAccess.IsInCompartment())
			return false;

		BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
		if (!compartment)
			return false;

		// Get the vehicle the player is in. (Can be the turret of a vehicle, thats why we use GetRootParent().)
		IEntity vehicle = compartment.GetOwner().GetRootParent();

		m_aCollectedNearbyEntities.Insert(vehicle);

		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(vehicle.FindComponent(BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return true;

		array<BaseCompartmentSlot> compartments = {};
		compartmentManager.GetCompartments(compartments);

		foreach (BaseCompartmentSlot comp : compartments)
		{
			IEntity compOwner = comp.GetOwner();
			if (compOwner && !m_aCollectedNearbyEntities.Contains(compOwner))
				m_aCollectedNearbyEntities.Insert(compOwner);
			
			IEntity compOccupant = comp.GetOwner();
			if (compOccupant && !m_aCollectedNearbyEntities.Contains(compOccupant))
				m_aCollectedNearbyEntities.Insert(compOccupant);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if player is inspecting something (weapon, item, etc) and grab the contexts from it
	//! \param[in] character controlled by the player
	//! \return True if player is inspecting something, false otherwise
	protected bool HandleInspection(notnull ChimeraCharacter character)
	{
		if (!character.GetCharacterController().GetInspect())
			return false;

		// Weapon is the priority if inspected, including all attachements
		CharacterControllerComponent ctrlComp = character.GetCharacterController();
		if (ctrlComp.GetInspectCurrentWeapon())
		{
			// Insert all items we can be interested in
			BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(character.FindComponent(BaseWeaponManagerComponent));
			if (!weaponManager)
				return false;

			BaseWeaponComponent weapon = weaponManager.GetCurrentWeapon();
			if (!weapon)
				return false;

			m_aCollectedNearbyEntities.Insert(weapon.GetOwner());
			m_aCollectedEntities.Insert(weapon.GetOwner());

			array<AttachmentSlotComponent> attachments = {};
			weapon.GetAttachments(attachments);

			foreach (AttachmentSlotComponent attachment : attachments)
			{
				IEntity attachedEntity = attachment.GetAttachedEntity();
				if (attachedEntity)
				{
					m_aCollectedNearbyEntities.Insert(attachedEntity);
					m_aCollectedEntities.Insert(attachedEntity);
				}
			}

			BaseMagazineComponent magazineComp = weapon.GetCurrentMagazine();
			if (magazineComp)
			{
				m_aCollectedNearbyEntities.Insert(magazineComp.GetOwner());
				m_aCollectedEntities.Insert(magazineComp.GetOwner());
			}

			return true;
		}
		else
		{
			// Whatever else is inspected kicks in
			IEntity inspectedItem = ctrlComp.GetInspectEntity();
			if (inspectedItem)
			{
				m_aCollectedNearbyEntities.Insert(inspectedItem);
				m_aCollectedEntities.Insert(inspectedItem);
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPostFrame(IEntity owner, IEntity controlledEntity, float timeSlice)
	{
		// TODO@AS: Add a reliable init method and get rid of this monstrosity
		if (!m_pDisplay)
			m_pDisplay = FindDisplay(owner);

		m_ControlledEntity = controlledEntity;
		// Make sure we have a valid character
		ChimeraCharacter character = ChimeraCharacter.Cast(controlledEntity);

		// Nearby context collection?
		bool enableNearbyCollection = ShouldBeEnabled(m_eDisplayMode, character, true);
		SetNearbyCollectionEnabled(enableNearbyCollection);

		// Make sure we have a valid character
		if (!character || character.IsInVehicleADS())
		{
			m_bPerformAction = false;
			return;
		}

		HandleOverride(character);

		UserActionContext currentContext = GetCurrentContext();
		if (currentContext)
		{
			array<BaseUserAction> actions = {};
			array<bool> canPerform = {};
			int count = GetFilteredActions(actions, canPerform);
			if (count > 0)
				GetGame().GetInputManager().ActivateContext("ActionMenuContext", 250);

			foreach (BaseUserAction action : actions)
			{
				if (m_pDisplay)
					m_pDisplay.OnActionProgress(character, action, action.GetActionProgress(0, 0), action.GetActionDuration());
			}

			AggregateActions(actions, canPerform);

			// First of all, prior to doing any destructive changes,
			// find the previous selected action (if any) and
			// update the index, in case it has been shuffled.
			if (m_pLastUserAction)
			{
				BaseUserAction action;
				for (int i = 0, ac = actions.Count(); i < ac; i++)
				{
					action = actions[i];
					if (action && action == m_pLastUserAction)
					{
						m_iSelectedActionIndex = i;
						break;
					}
				}
			}

			// Update selection
			int iScrollAmount = 0;
			int prevActionIndex = m_iSelectedActionIndex;
			// But only if player is not performing an action already
			if (!m_bIsPerforming)
			{
				if (Math.AbsFloat(m_fSelectAction) > 0.5)
					iScrollAmount = Math.Clamp(m_fSelectAction, -1.0, 1.0);

				if (iScrollAmount != 0)
					m_iSelectedActionIndex = m_iSelectedActionIndex - iScrollAmount;
			}

			// Make sure that selected action is always within bounds
			int actionsCount = actions.Count();
			m_iSelectedActionIndex = Math.Clamp(m_iSelectedActionIndex, 0, actionsCount - 1);

			BaseUserAction selectedAction = null;
			bool canPerformSelectedAction = false;

			if (m_bIsPerforming)
			{
				selectedAction = m_pLastUserAction;
				if (actions.Count() > prevActionIndex && actions[prevActionIndex] == m_pLastUserAction)
					canPerformSelectedAction = canPerform[prevActionIndex];
				else if (m_pLastUserAction)
					canPerformSelectedAction = m_pLastUserAction.CanBeShown(character) && m_pLastUserAction.CanBePerformed(character);
			}
			else if (actionsCount > 0)
			{
				selectedAction = actions[m_iSelectedActionIndex];
				canPerformSelectedAction = canPerform[m_iSelectedActionIndex];
			}

			// Process interaction
			if (selectedAction)
				canPerformSelectedAction = canPerformSelectedAction && selectedAction.CanBePerformed(character); // need to call this because the data from GetFilteredActions might not be up to date

			DoProcessInteraction(character, currentContext, selectedAction, canPerformSelectedAction, m_bPerformAction, timeSlice, m_pDisplay);
			m_pLastUserAction = selectedAction;
			SetSelectedAction(selectedAction);

			// Pass data to display
			if (m_pDisplay)
			{
				ActionsTuple pData = new ActionsTuple();
				bool canInteract = GetCanInteractScript(character);

				if (canInteract || m_bIsPerforming)
				{
					pData.param1 = actions;
					pData.param2 = canPerform;
				}
				else
				{
					pData.Init();
				}

				ActionDisplayData pDisplayData = new ActionDisplayData();
				pDisplayData.pUser = controlledEntity;
				pDisplayData.pActionsData = pData;
				pDisplayData.pSelectedAction = selectedAction;
				pDisplayData.pCurrentContext = currentContext;

				m_pDisplay.SetDisplayData(pDisplayData);
			}
		}
		// We don't have a context, but we possibly had, thus reset
		// our current action and make sure to dispatch events.
		else if (m_pLastContext != currentContext)
		{
			// We had valid action
			if (m_pLastUserAction)
			{
				// And we were performing it
				if (m_bIsPerforming)
				{
					// Reset state
					m_bIsPerforming = false;
					m_fCurrentProgress = 0.0;

					// Interruption event
					character.DoCancelObjectAction(m_pLastUserAction);
				}

				// Reset state
				m_pLastUserAction = null;
				SetSelectedAction(m_pLastUserAction);
			}
		}

		// Store last input
		m_bLastInput = m_bPerformAction;

		// Reset cached inputs to ensure that those actions will not be triggered on next frame
		m_fSelectAction = 0;
		if (!m_bIsPerforming)
			m_bPerformAction = false;

		// Update last context
		m_pLastContext = currentContext;
	}

	protected ref array<BaseUserAction> m_ActionsBuffer = {};
	protected ref array<bool> m_PerformBuffer = {};
	protected ref map<string, ref array<int>> m_IndicesBuffer = new map<string, ref array<int>>();

	//! Modifies the input lists (which need to be 1:1 in length and logical sense) so
	//! each action which can be aggregated (BaseUserAction.CanAggregate() returns true)
	//! only displays the first available (or any first if none is available to perform)
	//! action, filtered by BaseUserAction.GetActionName().
	protected void AggregateActions(array<BaseUserAction> actionsList, array<bool> canPerformList)
	{
		m_ActionsBuffer.Copy(actionsList);
		m_PerformBuffer.Copy(canPerformList);
		m_IndicesBuffer.Clear();
		actionsList.Clear();
		canPerformList.Clear();

		// First pass, filter&gather
		for (int i = 0, count = m_ActionsBuffer.Count(); i < count; i++)
		{
			BaseUserAction action = m_ActionsBuffer[i];
			if (!action)
				continue;

			// For non-aggregated actions, skip this process
			bool canPerform = m_PerformBuffer[i];
			if (!action.CanAggregate())
				continue;

			// Group actions of same name for aggregation
			string actionName = action.GetActionName();
			if (!m_IndicesBuffer.Contains(actionName))
				m_IndicesBuffer.Insert(actionName, {});

			m_IndicesBuffer[actionName].Insert(i);
		}

		// Second pass, resolve&output
		for (int i = 0, count = m_ActionsBuffer.Count(); i < count; i++)
		{
			BaseUserAction action = m_ActionsBuffer[i];
			if (!action)
				continue;

			// For non-aggregated actions, append the action straight away
			bool canPerform = m_PerformBuffer[i];
			if (!action.CanAggregate())
			{
				actionsList.Insert(action);
				canPerformList.Insert(canPerform);
				continue;
			}

			// For aggregated actions, find group of given actions
			string actionName = action.GetActionName();
			// If group was sorted, it will be removed from the map,
			// and no longer present, therefore we can ommit rechecking
			if (!m_IndicesBuffer.Contains(actionName))
				continue;

			// If not resolved yet, resolve by finding available action
			int availableIndex = m_IndicesBuffer[actionName][0]; // By default the first action
			foreach (int index : m_IndicesBuffer[actionName])
			{
				// First performable hit
				if (m_PerformBuffer[index])
				{
					availableIndex = index;
					break;
				}
			}

			BaseUserAction aggregatedAction = m_ActionsBuffer[availableIndex];
			bool aggregatedState = m_PerformBuffer[availableIndex];
			actionsList.Insert(aggregatedAction);
			canPerformList.Insert(aggregatedState);
			// And remove the action from the group map
			m_IndicesBuffer.Remove(actionName);
		}
	}

	//------------------------------------------------------------------------------------------------
	float GetRaycastThreshold()
	{
		return m_fRaycastThreshold;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if action can be shown when player is inside a vehicle
	//! \param[in] Character controlled by the player
	//! \param[in] User action that is being checked
	//! \param[in] True, if the action can only be seen from the PilotCompartment
	//! \param[in] True, if the action should be available to other passengers as long as pilot is not there or unconscious
	//! \param[in] True, if the action can only be seen when player is sitting in the vehicle, the action belongs to
	//! \param[in] Array of Compartments that the action should be shown in. (Does not work if pilotOnly = true)
	//! \param[in] Array of Compartments that the action can't be shown in. (Does not work if pilotOnly = true)
	static bool CanBeShownInVehicle(notnull ChimeraCharacter character, notnull BaseUserAction userAction, bool pilotOnly = false, bool pilotUncapableOverride = false, bool interiorOnly = false, array<int> definedCompartmentsOnly = null, array<int> excludeDefinedCompartments = null)
	{
		// See if character is in "this" (owner) vehicle
		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;

		Vehicle vehicle = Vehicle.Cast(userAction.GetOwner().GetRootParent());
		if (!vehicle)
			return false;

		// Check interior only condition
		// Character is in compartment
		// that belongs to owner of this action
		BaseCompartmentSlot slot = compartmentAccess.GetCompartment();
		IEntity slotRootParent;
		if (slot)
			slotRootParent = slot.GetOwner().GetRootParent();

		if (interiorOnly && (!slotRootParent || slotRootParent != vehicle))
			return false;

		// Check pilot only condition
		if (pilotOnly && !PilotCompartmentSlot.Cast(slot))
		{
			ChimeraCharacter pilot = ChimeraCharacter.Cast(vehicle.GetPilot());
			if (!pilotUncapableOverride && pilot != character)
				return false;

			if (pilot && pilot != character)
			{
				CharacterControllerComponent controller = pilot.GetCharacterController();
				if (controller && controller.GetLifeState() == ECharacterLifeState.ALIVE)
					return false;
			}
		}

		int compartmentSection = -1;
		if (slot)
			compartmentSection = slot.GetCompartmentSection();

		if (definedCompartmentsOnly && !definedCompartmentsOnly.IsEmpty() && (!definedCompartmentsOnly.Contains(compartmentSection) || vehicle != slotRootParent))
			return false;

		if (excludeDefinedCompartments && !excludeDefinedCompartments.IsEmpty() && (excludeDefinedCompartments.Contains(compartmentSection) || vehicle != slotRootParent))
			return false;

		return true;
	}
}
