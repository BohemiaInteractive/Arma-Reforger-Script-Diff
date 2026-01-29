class SCR_CampaignBuildingGadgetToolComponentClass : SCR_GadgetComponentClass
{
}

class SCR_CampaignBuildingGadgetToolComponent : SCR_GadgetComponent
{
	[Attribute(defvalue: "25", desc: "Max distance at which the ghost preview is shown.")]
	protected float m_fDistanceToShowPreview;

	[Attribute(defvalue: "10", desc: "Max distance from which the composition can be build.")]
	protected float m_fDistanceToBuildComposition;

	[Attribute(defvalue: "10", desc: "How much of construction value this tool adds to composition per one action")]
	protected int m_iConstructionValue;

	protected const static float TRACE_DELAY_VALUE = 0.5;

	protected float m_fTraceDelay;

	protected ref array<SCR_CampaignBuildingLayoutComponent> m_aShownPreview = {};
	protected ref array<SCR_CampaignBuildingLayoutComponent> m_aShownPreviewOld = {};

	protected bool m_bToolActiveWhenEditorOpen;

	protected ref array<IEntity> m_aFoundDeployables;
	protected SCR_MultiPartDeployableItemComponent m_CurrentlyHandledComponent;
	protected int m_iCurrentVariant = -1;
	protected bool m_bPlacementModeEnabled;
	protected bool m_bCanEnterPlacementMode;
	protected bool m_bHasMoreItemsToPlace;
	protected bool m_bHasMoreVariantsToChoose;

//	protected SCR_CampaignBuildingLayoutComponent m_LayoutComponent;

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		m_fTraceDelay += timeSlice;

		if (m_fTraceDelay < TRACE_DELAY_VALUE)
			return;

		TraceCompositionToShowPreview();

		m_fTraceDelay = 0.0;
	}

	//------------------------------------------------------------------------------------------------
	override void OnToggleActive(bool state)
	{
		m_fTraceDelay = 0.0;

		if (state)
			ToolToHand();
		else
			ToolToInventory();
	}

	//------------------------------------------------------------------------------------------------
	override EGadgetType GetType()
	{
		return EGadgetType.BUILDING_TOOL;
	}

//	//------------------------------------------------------------------------------------------------
//	// Perform one build step - add a given build value to a composition player is building.
//	protected void Build(notnull SCR_CampaignBuildingLayoutComponent layoutComponent)
//	{
//		layoutComponent.AddBuildingValue(m_iConstructionValue);
//	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when an operation with gadget is performed - picked up, taken into hands, back to backpack etc.
	//! \param[in] mode
	//! \param[in] charOwner must have a GadgetManager component
	override void ModeSwitch(EGadgetMode mode, IEntity charOwner)
	{
		if (!charOwner || charOwner != EntityUtils.GetPlayer())
		{
			super.ModeSwitch(mode, charOwner);
			return;
		}

		if (SCR_GadgetManagerComponent.GetGadgetManager(charOwner).GetHeldGadgetComponent() == this)
			ToolToHand();
		else
			ToolToInventory();

		super.ModeSwitch(mode, charOwner);

		if (mode != EGadgetMode.IN_HAND)
			return;

		if (m_CharacterOwner && m_CharacterOwner == SCR_PlayerController.GetLocalControlledEntity())
			m_bCanEnterPlacementMode = FindDeployableItems(skipUniqueCountCheck: true) > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Building tool taken to hand - show preview etc.
	void ToolToHand()
	{
		// Disabled for future
//		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(m_CharacterOwner.FindComponent(EventHandlerManagerComponent));
//		if (eventHandlerManager)
//			eventHandlerManager.RegisterScriptHandler("OnADSChanged", this, TraceCompositionToBuild);

		//ToDo: Later the frame has to run on server too because of tracing the composition to build
		if (!System.IsConsoleApp())
			ActivateGadgetUpdate();

		SCR_CharacterControllerComponent characterController = GetCharacterControllerComponent();
		if (!characterController)
			return;
			
		characterController.GetOnPlayerDeath().Insert(ToolToInventory);

		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		if (!editorManagerEntity)
			return;

		editorManagerEntity.GetOnOpened().Insert(OnEditorOpened);
		editorManagerEntity.GetOnClosed().Insert(OnEditorClosed);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerID
	//! \param[in] isPossessing
	//! \param[in] mainEntityID
	void OnPossessed(int playerID, bool isPossessing, RplId mainEntityID)
	{
		DeactivateGadgetUpdate();

		SCR_CharacterControllerComponent characterController = GetCharacterControllerComponent();
		if (!characterController)
			return;
		
		characterController.GetOnPlayerDeath().Remove(ToolToInventory);
		
		if (isPossessing)
			return;

		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		if (!editorManagerEntity)
			return;

		editorManagerEntity.GetOnOpened().Remove(OnEditorOpened);
		editorManagerEntity.GetOnClosed().Remove(OnEditorClosed);

		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(SCR_PlayerController.GetLocalControlledEntity());
		if (!gadgetManager)
			return;

		SCR_GadgetComponent gadgetComponent = gadgetManager.GetHeldGadgetComponent();
		if (!gadgetComponent)
			return;

		ToolToInventory();
		if (gadgetComponent.GetType() == EGadgetType.BUILDING_TOOL)
			gadgetManager.SetGadgetMode(gadgetManager.GetGadgetByType(EGadgetType.BUILDING_TOOL), EGadgetMode.IN_SLOT);
	}

	//------------------------------------------------------------------------------------------------
	//! Building tool out of hands - show preview.
	void ToolToInventory()
	{
		//ToDo: Later the frame has to run on server too because of tracing the composition to build
		if (!System.IsConsoleApp())
		{
			if (!GetBuildingModeEntity())
				RemovePreviews();

			DeactivateGadgetUpdate();

			RemoveEventHandlers();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Search for an editor mode entity (exists only when the player is in editor mode)
	//! \return
	SCR_EditorModeEntity GetBuildingModeEntity()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		if (!editorManager)
			return null;

		return editorManager.FindModeEntity(EEditorMode.BUILDING);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemovePreviews()
	{
		foreach (SCR_CampaignBuildingLayoutComponent layoutComponent : m_aShownPreviewOld)
		{
			if (layoutComponent)
				layoutComponent.DeletePreview();
		}

		m_aShownPreviewOld.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when character has a shovel in hands and any editor mode is opned.
	protected void OnEditorOpened()
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!char)
			return;
		
		m_bToolActiveWhenEditorOpen = true;
		ToolToInventory();
		
		SCR_CharacterControllerComponent controllerComponent = SCR_CharacterControllerComponent.Cast(char.GetCharacterController());
		if (!controllerComponent)
			return;
		
		controllerComponent.m_OnLifeStateChanged.Insert(OnConsciousnessChanged);
		
		SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
		if (!possessingManager)
			return;
			
		possessingManager.GetOnPossessedProxy().Insert(OnPossessed);
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when character has a shovel in hands and any editor mode is closed.
	protected void OnEditorClosed()
	{
		RemoveOnConsciousnessChanged();
		
		SCR_PossessingManagerComponent possessingManager = SCR_PossessingManagerComponent.GetInstance();
		if (!possessingManager)
			return;
		
		possessingManager.GetOnPossessedProxy().Remove(OnPossessed);

		// If the editor is closed, because player possessed entity, don't continue and keep this event hooked for another call.
		if (possessingManager.IsPossessing(SCR_PlayerController.GetLocalPlayerId()))
		{
			m_bToolActiveWhenEditorOpen = false;
			return;
		}

		m_bToolActiveWhenEditorOpen = false;
		RemoveEventHandlers();
		GetGame().GetCallqueue().CallLater(ToolToHand, 0, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnConsciousnessChanged(bool conscious)
	{
		if (conscious)
			return;
		
		m_bToolActiveWhenEditorOpen = false;
		RemoveOnConsciousnessChanged();
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveOnConsciousnessChanged()
	{
		ChimeraCharacter char = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!char)
			return;
		
		SCR_CharacterControllerComponent controllerComponent = SCR_CharacterControllerComponent.Cast(char.GetCharacterController());
		if (!controllerComponent)
			return;
		
		controllerComponent.m_OnLifeStateChanged.Remove(OnConsciousnessChanged);
	}
//	//------------------------------------------------------------------------------------------------
//	// Trace a composition to build commented out for future
//	void TraceCompositionToBuild()
//	{
//		if (!m_CharacterOwner)
//			return;
//
//		ChimeraCharacter character = ChimeraCharacter.Cast(m_CharacterOwner);
//
//		TraceParam param = new TraceParam();
//		param.Start = character.EyePosition();
//		param.End = param.Start + GetPlayersDirection() * m_fDistanceToBuildComposition;
//
//		param.Flags =TraceFlags.ENTS;
//		param.Exclude = character;
//		param.LayerMask = EPhysicsLayerPresets.Interaction;
//		BaseWorld world = GetOwner().GetWorld();
//		float traceDistance = world.TraceMove(param, EvaluateBuildEntity);
//
//		// This has to be done after the trace finished, because spawning / deleting or moving with the entity while trace is running can cause an issues.
//		if (m_LayoutComponent)
//			{
//				Build(m_LayoutComponent);
//				m_LayoutComponent = null;
//			}
//
//	}

	//------------------------------------------------------------------------------------------------
	//! Trace the possible previews
	//! \return
	protected bool TraceCompositionToShowPreview()
	{
		BaseWorld world = GetOwner().GetWorld();
		if (!world)
			return false;

		world.QueryEntitiesBySphere(GetOwner().GetOrigin(), m_fDistanceToShowPreview, EvaluatePreviewEntity);

		// Delete those preview which are no more on the list of traced one and then copy the array for next iteration cycle.
		for (int i = m_aShownPreviewOld.Count() - 1; i >= 0; i--)
		{
			if (!m_aShownPreviewOld[i] || !m_aShownPreviewOld[i].HasBuildingPreview())
			{
				m_aShownPreviewOld.Remove(i);
				continue;
			}

			if (!m_aShownPreview.Contains(m_aShownPreviewOld[i]))
				m_aShownPreviewOld[i].DeletePreview();
		}

		foreach (SCR_CampaignBuildingLayoutComponent component : m_aShownPreview)
		{
			if (component && !m_aShownPreviewOld.Contains(component))
				component.SpawnPreview();
		}

		m_aShownPreviewOld.Copy(m_aShownPreview);
		m_aShownPreview.Clear();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool EvaluatePreviewEntity(IEntity ent)
	{
		if (!ent)
			return true;

		SCR_CampaignBuildingLayoutComponent layoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(ent.FindComponent(SCR_CampaignBuildingLayoutComponent));
		if (!layoutComponent)
			return true;

		m_aShownPreview.Insert(layoutComponent);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the value how much this tool adds to a building value with one action
	int GetToolConstructionValue()
	{
		return m_iConstructionValue;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_CharacterControllerComponent GetCharacterControllerComponent()
	{
		IEntity playerEntity = SCR_PlayerController.GetLocalMainEntity();
		if (!playerEntity)
			return null;

		return SCR_CharacterControllerComponent.Cast(playerEntity.FindComponent(SCR_CharacterControllerComponent));
	}

//	//------------------------------------------------------------------------------------------------
//	// Disabled for future
//	bool EvaluateBuildEntity(IEntity ent, vector start = "0 0 0", vector dir = "0 0 0")
//	{
//		IEntity parent = ent.GetParent();
//		if (!parent)
//			return true;
//
//		SCR_CampaignBuildingLayoutComponent layoutComponent = SCR_CampaignBuildingLayoutComponent.Cast(parent.FindComponent(SCR_CampaignBuildingLayoutComponent));
//		if (!layoutComponent)
//			return true;
//
//		m_LayoutComponent = layoutComponent;
//		return false;
//	}

//	//------------------------------------------------------------------------------------------------
//	//! Returns the direction the player is looking Disabled debug
//	private vector GetPlayersDirection()
//	{
//		vector aimMat[4];
//		ChimeraCharacter character = ChimeraCharacter.Cast(m_CharacterOwner);
//		Math3D.AnglesToMatrix(CharacterControllerComponent.Cast(character.FindComponent(CharacterControllerComponent)).GetInputContext().GetAimingAngles() * Math.RAD2DEG, aimMat);
//		return aimMat[2];
//	}

	//------------------------------------------------------------------------------------------------
	//!
	void RemoveEventHandlers()
	{
		SCR_CharacterControllerComponent characterController = GetCharacterControllerComponent();
		if (characterController)
			characterController.GetOnPlayerDeath().Remove(ToolToInventory);

		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		if (editorManagerEntity)
		{
			editorManagerEntity.GetOnOpened().Remove(OnEditorOpened);

			if (!m_bToolActiveWhenEditorOpen)
				editorManagerEntity.GetOnClosed().Remove(OnEditorClosed);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	bool GetCanEnterPlacementMode()
	{
		return m_bCanEnterPlacementMode;
	}

	//------------------------------------------------------------------------------------------------
	//!
	bool HasMoreItemsToPlace()
	{
		return m_bHasMoreItemsToPlace;
	}

	//------------------------------------------------------------------------------------------------
	//!
	bool HasMoreVariants()
	{
		return m_bHasMoreVariantsToChoose;
	}

	//------------------------------------------------------------------------------------------------
	//!
	bool GetIsPlacementModeEnabled()
	{
		return m_bPlacementModeEnabled;
	}

	//------------------------------------------------------------------------------------------------
	//!
	SCR_MultiPartDeployableItemComponent GetCurrentlyHandledComponent()
	{
		return m_CurrentlyHandledComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! Searches the inventory for compatible deployable items
	//! \param[in] ignoredItem
	//! \param[in] skipUniqueCountCheck true if game should not try validate if player can cycle between available deployable items
	//! \return number of found items
	protected int FindDeployableItems(IEntity ignoredItem = null, bool skipUniqueCountCheck = false)
	{
		if (!m_CharacterOwner)
			return 0;

		CharacterControllerComponent controller = m_CharacterOwner.GetCharacterController();
		if (!controller)
			return 0;

		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (!storageManager)
			return 0;

		SCR_DeployableItemSearchPredicate searchPredicate = new SCR_DeployableItemSearchPredicate(ignoredItem);
		if (!m_aFoundDeployables)
			m_aFoundDeployables = {};
		else
			m_aFoundDeployables.Clear();

		int count = storageManager.FindItems(m_aFoundDeployables, searchPredicate);

		m_bHasMoreItemsToPlace = false;
		if (skipUniqueCountCheck)
			return count;

		GetGame().GetInputManager().RemoveActionListener(SCR_ItemPlacementComponent.ACTION_NAME_CHANGE_ITEM, EActionTrigger.DOWN, CycleItems);
		if (count > 1)
		{
			BaseContainer firstPrefab = m_aFoundDeployables[0].GetPrefabData().GetPrefab();
			foreach (IEntity ent : m_aFoundDeployables)
			{
				if (firstPrefab != ent.GetPrefabData().GetPrefab())
				{
					m_bHasMoreItemsToPlace = true;
					GetGame().GetInputManager().AddActionListener(SCR_ItemPlacementComponent.ACTION_NAME_CHANGE_ITEM, EActionTrigger.DOWN, CycleItems);
					return count;
				}
			}
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Action listener callback
	override protected void ActivateAction()
	{
		SetPlacementMode(!m_bPlacementModeEnabled);
	}

	//------------------------------------------------------------------------------------------------
	override protected void ModeClear(EGadgetMode mode)
	{
		super.ModeClear(mode);

		if (m_CharacterOwner && m_CharacterOwner == SCR_PlayerController.GetLocalControlledEntity())
			SetPlacementMode(false);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] newState
	protected void SetPlacementMode(bool newState)
	{
		if (m_bPlacementModeEnabled == newState)
			return;

		PlayerController controller = GetGame().GetPlayerController();
		if (!controller)
			return;

		SCR_ItemPlacementComponent placementComp = SCR_ItemPlacementComponent.Cast(controller.FindComponent(SCR_ItemPlacementComponent));
		if (!placementComp)
			return;

		if (!newState)
		{
			EnablePreview(false, placementComp);

			if (m_CurrentlyHandledComponent)
			{
				m_CurrentlyHandledComponent.ClearCache();
				SCR_DeployablePlaceableItemComponent placeableItemComp = SCR_DeployablePlaceableItemComponent.Cast(m_CurrentlyHandledComponent.GetOwner().FindComponent(SCR_DeployablePlaceableItemComponent));
				if (placeableItemComp)
					placeableItemComp.SetPlacingGadget(null);

				m_CurrentlyHandledComponent = null;
			}

			m_aFoundDeployables.Clear();
			m_iCurrentVariant = -1;

			placementComp.EnablePlacement(false);
			placementComp.SetPlacedItem(null);

			InputManager inputMgr = GetGame().GetInputManager();
			inputMgr.RemoveActionListener(SCR_ItemPlacementComponent.ACTION_NAME_CHANGE_ITEM, EActionTrigger.DOWN, CycleItems);
			inputMgr.RemoveActionListener(SCR_ItemPlacementComponent.ACTION_NAME_CHANGE_VARIANT, EActionTrigger.DOWN, CycleVariants);
			m_bPlacementModeEnabled = newState;
			return;
		}

		m_bCanEnterPlacementMode = FindDeployableItems() > 0;
		if (!m_bCanEnterPlacementMode)
			return;

		m_bPlacementModeEnabled = newState;
		SelectNextPlaceableItem(placementComp);

		placementComp.EnablePlacement(true);
	}

	//------------------------------------------------------------------------------------------------
	//!Callback method used to cycle between all unique deployable items
	//! \param[in] value
	//! \param[in] reason
	//! \param[in] actionName
	protected void CycleItems(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		PlayerController controller = GetGame().GetPlayerController();
		if (!controller)
			return;

		SCR_ItemPlacementComponent placementComp = SCR_ItemPlacementComponent.Cast(controller.FindComponent(SCR_ItemPlacementComponent));
		if (!placementComp)
			return;

		EnablePreview(false, placementComp);
		SelectNextPlaceableItem(placementComp);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to cycle between all available variants for currently used deployable
	//! \param[in] value
	//! \param[in] reason
	//! \param[in] actionName
	protected void CycleVariants(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		PlayerController controller = GetGame().GetPlayerController();
		if (!controller)
			return;

		SCR_ItemPlacementComponent placementComp = SCR_ItemPlacementComponent.Cast(controller.FindComponent(SCR_ItemPlacementComponent));
		if (!placementComp)
			return;

		EnablePreview(false, placementComp);

		if (!SelectNextVariant())
		{
			SetPlacementMode(false);
			return;
		}

		EnablePreview(true, placementComp);
	}

	//------------------------------------------------------------------------------------------------
	//! \return currently placed deployable max allowed tilt, and if such data is not available then it will return -1
	float GetMaxAllowedTilt()
	{
		if (!m_CurrentlyHandledComponent)
			return -1;

		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(m_CurrentlyHandledComponent.GetComponentData(m_CurrentlyHandledComponent.GetOwner()));
		if (!data)
			return -1;

		SCR_DeployableVariantContainer container = data.GetVariantContainer(m_iCurrentVariant);
		if (!container)
			return -1;

		return container.GetMaxAllowedTilt();
	}

	//------------------------------------------------------------------------------------------------
	//! Method used to check if there is enough space for currently handled deployable variant
	//! \param[in] caller
	//! \param[in,out] transform of the position for which validation should be performed. This position has already applied offset of 1% of its up vector (1cm)
	//! \param[out] cantPlaceReason
	//! \return
	bool ValidateSpace(notnull SCR_ItemPlacementComponent caller, inout vector transform[4], out ENotification cantPlaceReason)
	{
		if (!m_CurrentlyHandledComponent)
			return false;

		vector angle = Math3D.MatrixToAngles(transform);
		vector matCopy[4] = transform;
		// remove the offset to ensure that CheckAvailableSpace will have the same position when it is called by the user action
		matCopy[3] = matCopy[3] - matCopy[1] * 0.01;

		if (m_CurrentlyHandledComponent.CheckAvailableSpace(matCopy, angle))
		{
			m_CurrentlyHandledComponent.SetPreviewState(SCR_EPreviewState.PLACEABLE);
			return true;
		}

		cantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_NOT_ENOUGH_SPACE;
		m_CurrentlyHandledComponent.SetPreviewState(SCR_EPreviewState.BLOCKED);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] caller
	void OnItemPlacementStart(notnull SCR_ItemPlacementComponent caller)
	{
		caller.EnablePlacement(false);
		EnablePreview(false, caller);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] caller
	//! \param[in] item
	//! \param[in] succes
	//! \param[in] equipNext
	void OnAfterItemPlaced(notnull SCR_ItemPlacementComponent caller, notnull IEntity item, bool success, bool equipNext)
	{
		if (!success || !equipNext)
		{
			SetPlacementMode(false);
			return;
		}

		m_bCanEnterPlacementMode = FindDeployableItems(item) > 0;
		if (!m_bCanEnterPlacementMode)
		{
			SetPlacementMode(false);
			return;
		}

		//find the same prefab
		BaseContainer previousPrefab = item.GetPrefabData().GetPrefab();
		BaseContainer nextPrefab;
		if (m_CurrentlyHandledComponent)
			EnablePreview(false, caller);

		m_CurrentlyHandledComponent = null;
		foreach (IEntity otherItem : m_aFoundDeployables)
		{
			nextPrefab = otherItem.GetPrefabData().GetPrefab();
			if (previousPrefab != nextPrefab)
				continue;

			m_CurrentlyHandledComponent = SCR_MultiPartDeployableItemComponent.Cast(otherItem.FindComponent(SCR_MultiPartDeployableItemComponent));
			break;
		}

		if (!m_CurrentlyHandledComponent)
		{
			SelectNextPlaceableItem(caller);
			if (m_CurrentlyHandledComponent)
				caller.EnablePlacement(true);

			return;
		}

		IEntity nowCurrentItem = m_CurrentlyHandledComponent.GetOwner();
		SCR_DeployablePlaceableItemComponent placeableItemComp = SCR_DeployablePlaceableItemComponent.Cast(nowCurrentItem.FindComponent(SCR_DeployablePlaceableItemComponent));
		if (!placeableItemComp)
			return;

		placeableItemComp.SetPlacingGadget(this);
		caller.SetPlaceableItemComponent(placeableItemComp);
		caller.SetPlacedItem(nowCurrentItem);

		if (!m_CurrentlyHandledComponent.FetchVariantData(m_iCurrentVariant))
		{
			SetPlacementMode(false);
			return;
		}

		EnablePreview(true, caller);
		caller.EnablePlacement(true);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool SelectNextVariant()
	{
		if (!m_CurrentlyHandledComponent)
			return false;

		GetGame().GetInputManager().RemoveActionListener(SCR_ItemPlacementComponent.ACTION_NAME_CHANGE_VARIANT, EActionTrigger.DOWN, CycleVariants);
		IEntity deployableEntity = m_CurrentlyHandledComponent.GetOwner();
		if (!deployableEntity)
			return false;

		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(m_CurrentlyHandledComponent.GetComponentData(deployableEntity));
		if (!data)
			return false;

		array<ref SCR_DeployableVariantContainer> variantsArrayPointer = data.GetVariants();
		if (!variantsArrayPointer)
			return false;

		int numberOfVariants = variantsArrayPointer.Count();
		m_bHasMoreVariantsToChoose = numberOfVariants > 1;

		if (m_iCurrentVariant < 0 || !m_bHasMoreVariantsToChoose)
		{
			m_iCurrentVariant = variantsArrayPointer[0].GetVariantId();
			m_CurrentlyHandledComponent.FetchVariantData(m_iCurrentVariant);
			if (m_bHasMoreVariantsToChoose)
				GetGame().GetInputManager().AddActionListener(SCR_ItemPlacementComponent.ACTION_NAME_CHANGE_VARIANT, EActionTrigger.DOWN, CycleVariants);

			return true;
		}

		int nextVariantId = -1;
		foreach (int i, SCR_DeployableVariantContainer variant : variantsArrayPointer)
		{
			if (variant.GetVariantId() == m_iCurrentVariant)
			{
				nextVariantId = i + 1;
				break;
			}
		}

		if (nextVariantId < 0)
			return false;

		GetGame().GetInputManager().AddActionListener(SCR_ItemPlacementComponent.ACTION_NAME_CHANGE_VARIANT, EActionTrigger.DOWN, CycleVariants);
		if (nextVariantId >= numberOfVariants)
			nextVariantId = 0;

		SCR_DeployableVariantContainer variantPointer = variantsArrayPointer[nextVariantId];
		if (!variantPointer)
			return false;

		m_iCurrentVariant = variantPointer.GetVariantId();

		return m_CurrentlyHandledComponent.FetchVariantData(m_iCurrentVariant);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] placementComp
	protected void SelectNextPlaceableItem(notnull SCR_ItemPlacementComponent placementComp)
	{
		if (!m_aFoundDeployables)
			return;

		if (!m_bPlacementModeEnabled)
			return;

		int numberOfPlaceables = m_aFoundDeployables.Count();
		if (!m_CurrentlyHandledComponent)
		{
			if (numberOfPlaceables < 1)
				return;

			m_CurrentlyHandledComponent = SCR_MultiPartDeployableItemComponent.Cast(m_aFoundDeployables[0].FindComponent(SCR_MultiPartDeployableItemComponent));
		}
		else
		{
			if (numberOfPlaceables < 2)
				return;

			IEntity currentItem = m_CurrentlyHandledComponent.GetOwner();
			if (!currentItem)
				return;

			int currentId = m_aFoundDeployables.Find(currentItem);
			if (currentId < 0)
			{
				m_CurrentlyHandledComponent = null;
				SelectNextPlaceableItem(placementComp);
				return;
			}

			BaseContainer currentPrefab = currentItem.GetPrefabData().GetPrefab();
			IEntity otherItem;
			for (int i = currentId; i < numberOfPlaceables; i++)
			{
				otherItem = m_aFoundDeployables[i];
				if (otherItem && otherItem != currentItem && currentPrefab != otherItem.GetPrefabData().GetPrefab())
					break;

				otherItem = null;
				if (currentId > 0 && i == numberOfPlaceables - 1)
				{
					//restart from the begining
					i = -1;
					numberOfPlaceables = currentId;
					currentId = -1;
				}
			}

			if (!otherItem)
				return;

			m_CurrentlyHandledComponent = SCR_MultiPartDeployableItemComponent.Cast(otherItem.FindComponent(SCR_MultiPartDeployableItemComponent));
		}

		if (!m_CurrentlyHandledComponent)
			return;

		m_iCurrentVariant = -1;
		if (!SelectNextVariant())
		{
			SetPlacementMode(false);
			return;
		}

		if (!placementComp)
		{
			PlayerController controller = GetGame().GetPlayerController();
			if (!controller)
				return;

			placementComp = SCR_ItemPlacementComponent.Cast(controller.FindComponent(SCR_ItemPlacementComponent));
			if (!placementComp)
				return;
		}

		IEntity nowCurrentItem = m_CurrentlyHandledComponent.GetOwner();
		SCR_DeployablePlaceableItemComponent placeableItemComp = SCR_DeployablePlaceableItemComponent.Cast(nowCurrentItem.FindComponent(SCR_DeployablePlaceableItemComponent));
		if (!placeableItemComp)
			return;

		placeableItemComp.SetPlacingGadget(this);
		placementComp.SetPlaceableItemComponent(placeableItemComp);
		placementComp.SetPlacedItem(nowCurrentItem);

		EnablePreview(m_bPlacementModeEnabled, placementComp);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] enabled
	//! \param[in] placementComp
	protected void EnablePreview(bool enabled, notnull SCR_ItemPlacementComponent placementComp)
	{
		if (!enabled)
		{
			placementComp.SetPreviewEntity(null, false);
			m_CurrentlyHandledComponent.SetPreviewState(SCR_EPreviewState.NONE);
			return;
		}

		if (!m_CurrentlyHandledComponent)
			return;

		IEntity deployableEntity = m_CurrentlyHandledComponent.GetOwner();
		if (!deployableEntity)
			return;

		SCR_MultiPartDeployableItemComponentClass data = SCR_MultiPartDeployableItemComponentClass.Cast(m_CurrentlyHandledComponent.GetComponentData(deployableEntity));
		if (!data)
			return;

		vector transform[4];
		Math3D.MatrixIdentity4(transform);
		m_CurrentlyHandledComponent.VisualizeReplacementEntity(transform);

		IEntity previewEnt = data.GetPreviewEntity();
		if (!previewEnt)
			return;

		data.SetPreviewState(SCR_EPreviewState.PLACEABLE);
		placementComp.SetPreviewEntity(previewEnt, false);
	}
}
