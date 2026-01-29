[EntityEditorProps(category: "GameScripted/Components", description: "")]
class SCR_ItemPlacementComponentClass : ScriptComponentClass
{
}

class SCR_ItemPlacementComponent : ScriptComponent
{
	[Attribute("{56EBF5038622AC95}Assets/Conflict/CanBuild.emat")]
	protected ResourceName m_sCanBuildMaterial;

	[Attribute("{14A9DCEA57D1C381}Assets/Conflict/CannotBuild.emat")]
	protected ResourceName m_sCannotBuildMaterial;

	[Attribute("{8FBC3A6E946F056E}Common/Materials/Default_Transparent.emat")]
	protected ResourceName m_sTransparentMaterial;

	protected int m_iEquipComplete;
	protected int m_iTargetEntityNodeID;
	protected bool m_bInEditor;
	protected bool m_bIsBeingAttachedToEntity;
	protected vector m_vCurrentMat[4];
	protected vector m_aCamDeploymentPosition[4];
	protected IEntity m_PlacedItem;
	protected ENotification m_eCantPlaceReason;
	protected SCR_PlaceableItemComponent m_PlaceableItem;
	protected IEntity m_PreviewEntity;
	protected SCR_CompartmentAccessComponent m_CompartmnetAccessComponent;
	protected RplId m_TargetId;
	protected IEntity m_TargetEntity;

	static const string ACTION_CONTEXT_PLACEMENT = "CharacterPlacementContext";
	static const string ACTION_NAME_PLACEMENT = "CharacterPlaceItem";
	static const string ACTION_NAME_FORCE_PLACEMENT = "CharacterForcePlaceItem";
	static const string ACTION_NAME_CHANGE_ITEM = "ChangePlacedItem";
	static const string ACTION_NAME_CHANGE_VARIANT = "ChangedPlacedVariant";

	//------------------------------------------------------------------------------------------------
	//! Client side method for informing the authority about where item should be placed upon removal from their inventory
	//! \param[in] right
	//! \param[in] up
	//! \param[in] forward
	//! \param[in] position
	//! \param[in] placeableId replication id of placed item
	//! \param[in] characterId replication id of the character that placed it there
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskSetPlacementPosition(vector right, vector up, vector forward, vector position, RplId placeableId, RplId characterId)
	{
		RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(placeableId));
		if (!rplComponent)
			return;

		IEntity item = rplComponent.GetEntity();
		SCR_PlaceableInventoryItemComponent itemComponent = SCR_PlaceableInventoryItemComponent.Cast(item.FindComponent(SCR_PlaceableInventoryItemComponent));
		if (!itemComponent)
			return;

		if (!characterId.IsValid())
			return;

		RplComponent characterRplComp = RplComponent.Cast(Replication.FindItem(characterId));
		if (!characterRplComp)
			return;

		ChimeraCharacter placingCharacter = ChimeraCharacter.Cast(characterRplComp.GetEntity());
		if (!placingCharacter)
			return;

		SCR_PlaceableItemComponent placebaleItemComp = SCR_PlaceableItemComponent.Cast(item.FindComponent(SCR_PlaceableItemComponent));
		if (!placebaleItemComp)
			return;

		// +10% to compensate for minor movements that may come from the animations, as locally client evaluates the distance from the position at which he started the placement
		float maxPlacementDistance = placebaleItemComp.GetMaxPlacementDistance() * 1.1;
		if (SCR_PlaceableItemComponent.GetDistanceFromCharacter(placingCharacter, position, placebaleItemComp.GetDistanceMeasurementMethod()) > maxPlacementDistance)
			return; // to ensure that someone isnt placing f.e. a explosive charge further than he should be able to

		itemComponent.SetPlacementPosition(right, up, forward, position, characterId);
	}

	//------------------------------------------------------------------------------------------------
	//! placeableId = id of items rpl component
	//! \param[in] placeableId
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskPlaceItem(RplId placeableId)
	{
		RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(placeableId));
		if (!rplComponent)
			return;

		IEntity item = rplComponent.GetEntity();
		SCR_PlaceableInventoryItemComponent itemComponent = SCR_PlaceableInventoryItemComponent.Cast(item.FindComponent(SCR_PlaceableInventoryItemComponent));
		if (!itemComponent)
			return;

		itemComponent.PlaceItem();
	}

	//------------------------------------------------------------------------------------------------
	//! placeableId = id of items rpl component
	//! \param[in] placeableId
	//! \param[in] targetId is an RplId of the owner of the surface to which this item is meant to be attached to
	//! \param[in] nodeId is an node id of the surface to which this object is meant to be attached to
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_AskPlaceItemWithParentChange(RplId placeableId, RplId targetId, int nodeId)
	{
		RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(placeableId));
		if (!rplComponent)
			return;

		IEntity item = rplComponent.GetEntity();
		SCR_PlaceableInventoryItemComponent itemComponent = SCR_PlaceableInventoryItemComponent.Cast(item.FindComponent(SCR_PlaceableInventoryItemComponent));
		if (!itemComponent)
			return;

		if (targetId.IsValid())
			itemComponent.PlaceItemWithParentChange(targetId, nodeId);
		else
			itemComponent.PlaceItem();
	}

	//------------------------------------------------------------------------------------------------
	protected void UnregisterEvents(IEntity from)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(from);
		if (!character)
			return;

		BaseWeaponManagerComponent weaponManager = character.GetWeaponManager();
		if (weaponManager)
		{
			weaponManager.m_OnWeaponChangeCompleteInvoker.Remove(OnWeaponChangeEnded);
			weaponManager.m_OnWeaponChangeStartedInvoker.Remove(OnWeaponChanged);
		}

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (characterController)
			characterController.GetOnPlayerDeathWithParam().Remove(OnCharacterDeath);

		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(character.GetCompartmentAccessComponent());
		if (compartmentAccessComponent)
		{
			compartmentAccessComponent.GetOnCompartmentEntered().Remove(OnComparmentEntered);
			compartmentAccessComponent.GetOnCompartmentLeft().Remove(GetOnCompartmentLeft);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterEvents(IEntity to)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(to);
		if (!character)
			return;

		BaseWeaponManagerComponent weaponManager = character.GetWeaponManager();
		if (weaponManager)
		{
			weaponManager.m_OnWeaponChangeCompleteInvoker.Insert(OnWeaponChangeEnded);
			weaponManager.m_OnWeaponChangeStartedInvoker.Insert(OnWeaponChanged);
		}

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (characterController)
			characterController.GetOnPlayerDeathWithParam().Insert(OnCharacterDeath);

		SCR_CompartmentAccessComponent compartmentAccessComponent = SCR_CompartmentAccessComponent.Cast(character.GetCompartmentAccessComponent());
		if (compartmentAccessComponent)
		{
			compartmentAccessComponent.GetOnCompartmentEntered().Insert(OnComparmentEntered);
			compartmentAccessComponent.GetOnCompartmentLeft().Insert(GetOnCompartmentLeft);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		m_PlacedItem = null;

		ChimeraCharacter characterTo = ChimeraCharacter.Cast(to);
		if (characterTo)
		{
			m_CompartmnetAccessComponent = SCR_CompartmentAccessComponent.Cast(characterTo.GetCompartmentAccessComponent());
			m_PlacedItem = characterTo.GetCharacterController().GetRightHandItem();
		}
		else
		{
			m_CompartmnetAccessComponent = null;
		}

		// unregister from previous event handler
		UnregisterEvents(from);

		// register to new event handler
		RegisterEvents(to);

		ChimeraCharacter previousCharacter = ChimeraCharacter.Cast(from);
		if (!previousCharacter)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(previousCharacter.GetCharacterController());
		if (!characterController)
			return;

		characterController.m_OnItemUseEndedInvoker.Remove(OnPlacingEnded);
		GetGame().GetCallqueue().Remove(ValidateTargetEntityExistance);
		if (!characterController.IsUsingItem())
			return;

		CharacterAnimationComponent animationComponent = characterController.GetAnimationComponent();
		if (!animationComponent)
			return;

		animationComponent.GetCommandHandler().CancelItemUse();
		
		SCR_NotificationsComponent.SendLocal(ENotification.PLACEABLE_ITEM_CANT_PLACE_GENERIC); // show some generic information, just to be clear that his placement action has failed
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method triggered by button press
	protected void ForcePlaceItem()
	{
		if (m_eCantPlaceReason == ENotification.PLACEABLE_ITEM_CANT_PLACE_NOT_ENOUGH_SPACE)
			m_eCantPlaceReason = 0;

		StartPlaceItem();
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method triggered by button press
	protected void StartPlaceItem()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;

		if (m_eCantPlaceReason > 0)
		{
			SCR_NotificationsComponent.SendLocal(m_eCantPlaceReason);
			return;
		}

		bool skipItemUsage;
		if (m_PlaceableItem.OverrideStartPlaceAction(this, skipItemUsage))
		{
			if (m_eCantPlaceReason != 0)
			{
				SCR_NotificationsComponent.SendLocal(m_eCantPlaceReason);
				return;
			}

			if (skipItemUsage)
			{
				OnPlacingEnded(m_PlacedItem, true, null);
				return;
			}
		}

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!characterController)
			return;

		characterController.SetDisableWeaponControls(true);
		characterController.SetDisableMovementControls(true);

		if (characterController.IsUsingItem())
			return;

		CharacterAnimationComponent animationComponent = character.GetAnimationComponent();
		int itemActionId = animationComponent.BindCommand("CMD_Item_Action");
		vector mat[4];
		Math3D.MatrixCopy(m_vCurrentMat, mat);
		PointInfo ptWS = new PointInfo();
		mat[2] = (mat[3] - character.GetOrigin()).Normalized();
		mat[1] = vector.Up;
		mat[0] = Vector(mat[2][2], mat[2][1], -mat[2][0]);
		ptWS.Set(null, "", mat);

		ItemUseParameters params = new ItemUseParameters();
		params.SetEntity(m_PlacedItem);
		params.SetAllowMovementDuringAction(false);
		params.SetKeepInHandAfterSuccess(false);
		params.SetCommandID(itemActionId);
		params.SetCommandIntArg(1);
		params.SetMaxAnimLength(15.0);
		params.SetAlignmentPoint(ptWS);

		DisablePreview(); // disable preview and suspend placement logic to prevent executing this 
 		characterController.m_OnItemUseEndedInvoker.Insert(OnPlacingEnded); // subscribe to this invoker now, as there is always a chance that someone will be so fast that we will miss it

		if (!characterController.TryUseItemOverrideParams(params))
			OnPlacingEnded(m_PlaceableItem.GetOwner(), false, params); // if item usage has failed, we need to manually informs ourselves that placement has failed
		else if (m_bIsBeingAttachedToEntity)
			GetGame().GetCallqueue().CallLater(ValidateTargetEntityExistance, 500, true); // used to locally check if the entity to which player wants to attach the object is still there

		characterController.GetAnimationComponent().GetCommandHandler().AlignNewTurns();

	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlacingEnded(IEntity item, bool successful, ItemUseParameters animParams)
	{
		GetGame().GetCallqueue().Remove(ValidateTargetEntityExistance);

		ChimeraCharacter controlledEntity = ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		if (!controlledEntity)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(controlledEntity.GetCharacterController());
		if (!characterController)
			return;

		RplComponent characterRplComp = controlledEntity.GetRplComponent();
		if (!characterRplComp)
			return;

		characterController.SetDisableWeaponControls(true);
		characterController.SetDisableMovementControls(true);

		characterController.m_OnItemUseEndedInvoker.Remove(OnPlacingEnded);

		if (!m_PlacedItem)
			return;

		if (!successful)
		{
			EnablePreview(item);
			return;
		}

		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(characterController.GetInventoryStorageManager());
		if (!storageManager)
			return;

		SCR_CharacterInventoryStorageComponent storage = storageManager.GetCharacterStorage();
		if (!storage)
			return;

		RplComponent rplComponent = RplComponent.Cast(m_PlacedItem.FindComponent(RplComponent));
		if (!rplComponent)
			return;

		SCR_PlaceableInventoryItemComponent placeableItem = SCR_PlaceableInventoryItemComponent.Cast(m_PlacedItem.FindComponent(SCR_PlaceableInventoryItemComponent));
		if (placeableItem)
		{
			SCR_PlacementCallback cb = new SCR_PlacementCallback();
			cb.m_PlaceableId = rplComponent.Id();
			cb.m_TargetId = m_TargetId;
			cb.m_iNodeId = m_iTargetEntityNodeID;
			cb.m_bIsBeingAttachedToEntity = m_bIsBeingAttachedToEntity;

			BaseContainer gameplaySettings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
			bool autoEquipNext;
			if (gameplaySettings && gameplaySettings.Get("m_bAutoEquipNextPlaceableItem", autoEquipNext) && !m_PlaceableItem.OverrideAutoEquipMechanism(cb.m_ChainedCallback, this, m_PlacedItem, autoEquipNext))
			{
				SCR_InvEquipAnyItemCB equipmentCallback = new SCR_InvEquipAnyItemCB();
				if (storageManager.TryAssigningNextItemToQuickSlot(m_PlacedItem, equipCallback: equipmentCallback, equipNewItem: autoEquipNext))
					cb.m_ChainedCallback = equipmentCallback;
			}
			
			Rpc(RPC_AskSetPlacementPosition, m_vCurrentMat[0], m_vCurrentMat[1], m_vCurrentMat[2], m_vCurrentMat[3], rplComponent.Id(), characterRplComp.Id());
			if (storageManager.TryRemoveItemFromStorage(m_PlacedItem, placeableItem.GetParentSlot().GetStorage(), cb))
				return;
		}

		if (!m_PlaceableItem.OverrideOnPlacingEnded(this))
			DisablePreview();
	}

	//------------------------------------------------------------------------------------------------
	//! Client side method for finishing item placement when that item was successfully removed from their inventory
	//! \param[in] placeableId replication id of placed item
	//! \param[in] targetId replication id of object to which this item is being attached to
	//! \param[in] nodeId node of the target object to which placed item will be attached to
	//! \param[in] isBeingAttachedToEntity
	void AskPlaceItem(RplId placeableId, RplId targetId, int nodeId, bool isBeingAttachedToEntity)
	{
		if (!m_bIsBeingAttachedToEntity)
			Rpc(RPC_AskPlaceItem, placeableId);
		else if (targetId.IsValid())
			Rpc(RPC_AskPlaceItemWithParentChange, placeableId, targetId, nodeId);
	}

	//------------------------------------------------------------------------------------------------
	void ValidateTargetEntityExistance()
	{
		TraceParam param = new TraceParam();
		param.Start = m_aCamDeploymentPosition[3];
		param.End = param.Start + m_aCamDeploymentPosition[2] * m_PlaceableItem.GetMaxPlacementDistance();
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.Exclude = SCR_PlayerController.GetLocalControlledEntity();
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		BaseWorld world = GetOwner().GetWorld();
		float traceDistance = world.TraceMove(param, FilterCallback);
		IEntity tracedEntity = param.TraceEnt;

		if (!tracedEntity && m_TargetId.IsValid())
		{
			m_eCantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_SURFACE_NO_LONGER_THERE;//entity to which we were attaching is no longer there
		}
		else if (tracedEntity)
		{
			RplComponent rplComp = RplComponent.Cast(tracedEntity.FindComponent(RplComponent));
			if (!rplComp || rplComp.Id() != m_TargetId)
				m_eCantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_SURFACE_NO_LONGER_THERE;//entity to which we were attaching is no longer there
			else if (m_iTargetEntityNodeID != param.NodeIndex)
				m_eCantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_DIFFERENT_SURFACE;//same entity but different part of it is now there
			else
				m_eCantPlaceReason = 0;
		}

		if (!m_PlacedItem)
			m_eCantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_GENERIC; // we lost the reference so we need to abort the placement logic

		if (m_eCantPlaceReason > 0)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
			if (!character)
				return;

			SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
			if (!characterController)
				return;

			if (!characterController.IsUsingItem())
			{
				GetGame().GetCallqueue().Remove(ValidateTargetEntityExistance);
			}
			else
			{
				CharacterAnimationComponent animationComponent = characterController.GetAnimationComponent();
				if (!animationComponent)
					return;

				CharacterCommandHandlerComponent commandHandler = animationComponent.GetCommandHandler();
				commandHandler.CancelItemUse();
				SCR_NotificationsComponent.SendLocal(m_eCantPlaceReason);
				GetGame().GetCallqueue().Remove(ValidateTargetEntityExistance);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] item
	void SetPlacedItem(IEntity item)
	{
		m_PlacedItem = item;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] preview
	//! \param[in] deleteOldPreview true if previous preview should be deleted
	void SetPreviewEntity(IEntity preview, bool deleteOldPreview = true)
	{
		if (deleteOldPreview && m_PreviewEntity)
			delete m_PreviewEntity;

		m_PreviewEntity = preview;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] placeableItemComp
	void SetPlaceableItemComponent(SCR_PlaceableItemComponent placeableItemComp)
	{
		m_PlaceableItem = placeableItemComp;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return currently used component for placing an item
	SCR_PlaceableItemComponent GetPlaceableItemComponent()
	{
		return m_PlaceableItem;
	}

	//------------------------------------------------------------------------------------------------
	//! Enable listening for actions and position validation
	//! \param[in] enabled true when placement should be enabled, otherwise false
	void EnablePlacement(bool enabled)
	{
		if (enabled)
		{
			GetGame().GetInputManager().AddActionListener(ACTION_NAME_PLACEMENT, EActionTrigger.DOWN, StartPlaceItem);
			if (m_PlaceableItem && m_PlaceableItem.GetCanBeForcedPlaced())
				GetGame().GetInputManager().AddActionListener(ACTION_NAME_FORCE_PLACEMENT, EActionTrigger.DOWN, ForcePlaceItem);

			SetEventMask(GetOwner(), EntityEvent.FRAME);	
		}
		else
		{
 			GetGame().GetInputManager().RemoveActionListener(ACTION_NAME_PLACEMENT, EActionTrigger.DOWN, StartPlaceItem);
			GetGame().GetInputManager().RemoveActionListener(ACTION_NAME_FORCE_PLACEMENT, EActionTrigger.DOWN, ForcePlaceItem);
			ClearEventMask(GetOwner(), EntityEvent.FRAME);
		}
	}

	//------------------------------------------------------------------------------------------------
	void DisablePreview()
	{
		EnablePlacement(false);
		delete m_PreviewEntity;
	}

	//------------------------------------------------------------------------------------------------
	protected void EnablePreview(IEntity weapon)
	{
		m_PlaceableItem = SCR_PlaceableItemComponent.Cast(weapon.FindComponent(SCR_PlaceableItemComponent));
		if (!m_PlaceableItem)
			return;

		EnablePlacement(true);

		if (!m_PreviewEntity)
			m_PreviewEntity = GetGame().SpawnEntity(GenericEntity, GetOwner().GetWorld());

		m_PreviewEntity.SetObject(m_PlaceableItem.GetPreviewVobject(), string.Empty);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnComparmentEntered(IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		DisablePreview();
	}

	//------------------------------------------------------------------------------------------------
	protected void GetOnCompartmentLeft()
	{
		TogglePreview(m_PlacedItem);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] weaponComponent
	//! \return
	IEntity GetWeaponFromWeaponComponent(BaseWeaponComponent weaponComponent)
	{
		WeaponSlotComponent weaponSlot = WeaponSlotComponent.Cast(weaponComponent);
		if (!weaponSlot)
			return null;

		return weaponSlot.GetWeaponEntity();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWeaponChangeEnded(BaseWeaponComponent newWeaponSlot)
	{
		m_PlacedItem = GetWeaponFromWeaponComponent(newWeaponSlot);
		TogglePreview(m_PlacedItem);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnWeaponChanged(BaseWeaponComponent newWeaponSlot)
	{
		DisablePreview(); // Automatically disable preview to handle cases where anim event could be missing
		m_PlacedItem = GetWeaponFromWeaponComponent(newWeaponSlot);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCharacterDeath(SCR_CharacterControllerComponent characterController, IEntity killerEntity, notnull Instigator killer)
	{
		UnregisterEvents(characterController.GetOwner());
		m_PlacedItem = null; // Reset this cached value
		DisablePreview();
	}
	
	//------------------------------------------------------------------------------------------------
	// Reacts to weapon changes of local character
	protected void TogglePreview(IEntity weapon)
	{
		if (m_bInEditor || !weapon || !weapon.FindComponent(SCR_PlaceableItemComponent))
		{
			DisablePreview();
		}
		else
		{
			if (weapon.GetRootParent() != SCR_PlayerController.GetLocalControlledEntity())
				return;

			ChimeraCharacter char = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
			SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(char.GetCharacterController());
			if (characterController)
				characterController.m_OnItemUseEndedInvoker.Dump();

			EnablePreview(weapon);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_CompartmnetAccessComponent.IsGettingIn())
		{
			DisablePreview();
			return;
		}

		if (!m_PlacedItem || !m_PreviewEntity)
		{
			// This handles situations where f. e. land mines explode in characters hands
			DisablePreview();
			return;
		}

		GetGame().GetInputManager().ActivateContext(ACTION_CONTEXT_PLACEMENT);
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return;

		CameraBase currentCamera = cameraManager.CurrentCamera();
		if (!currentCamera)
			return;

		vector cameraMat[4];
		currentCamera.GetTransform(cameraMat);
		float maxPlacementDistance = m_PlaceableItem.GetMaxPlacementDistance();
		SCR_EPlacementType placementType = m_PlaceableItem.GetPlacementType();

		m_eCantPlaceReason = 0;
		switch (placementType)
		{
			case SCR_EPlacementType.XZ_FIXED:
				UseXZFixedPlacement(owner, maxPlacementDistance, cameraMat);
				break;

			case SCR_EPlacementType.XYZ:
				UseXYZPlacement(owner, maxPlacementDistance, cameraMat);
				break;
		}

		if (m_eCantPlaceReason == 0)
			SCR_Global.SetMaterial(m_PreviewEntity, m_sCanBuildMaterial);
		else if (m_eCantPlaceReason == ENotification.PLACEABLE_ITEM_CANT_PLACE_DISTANCE)
			SCR_Global.SetMaterial(m_PreviewEntity, m_sTransparentMaterial);
		else
			SCR_Global.SetMaterial(m_PreviewEntity, m_sCannotBuildMaterial);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in] maxPlacementDistance
	//! \param[in] cameraMat
	void UseXZFixedPlacement(IEntity owner, float maxPlacementDistance, vector cameraMat[4])
	{
		vector direction = cameraMat[2];
		direction[1] = 0;
		direction.Normalize();

		IEntity character = SCR_PlayerController.GetLocalControlledEntity();

		// Trace against terrain and entities to detect item placement position
		TraceParam param = new TraceParam();
		param.Start = character.GetOrigin() + maxPlacementDistance * direction + vector.Up;
		param.End = param.Start - 5 * vector.Up;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.Exclude = character;
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		BaseWorld world = owner.GetWorld();
		float traceDistance = world.TraceMove(param, FilterCallback);
		m_PreviewEntity.GetTransform(m_vCurrentMat);
		m_vCurrentMat[3] = param.Start + ((param.End - param.Start) * traceDistance);
		vector up = param.TraceNorm;

		SCR_EntityHelper.OrientUpToVector(up, m_vCurrentMat);
		m_PreviewEntity.SetTransform(m_vCurrentMat);
		m_PreviewEntity.Update();

		ValidatePlacement(up, param.TraceEnt, world, character, m_eCantPlaceReason);
	}

	//------------------------------------------------------------------------------------------------
	protected bool FilterCallback(IEntity e)
	{
		Physics physics = e.GetPhysics();
		if (physics)
		{
			if ((physics.IsDynamic() || physics.IsKinematic()) && !m_PlaceableItem.CanAttachToDynamicObject())
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	//! \param[in] maxPlacementDistance
	//! \param[in] cameraMat
	void UseXYZPlacement(IEntity owner, float maxPlacementDistance, vector cameraMat[4])
	{
		// Trace against terrain and entities to detect item placement position
		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;

		TraceParam param = new TraceParam();
		param.Start = character.EyePosition();
		param.End = param.Start + cameraMat[2] * maxPlacementDistance;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.Exclude = SCR_PlayerController.GetLocalControlledEntity();
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		BaseWorld world = owner.GetWorld();
		float traceDistance = world.TraceMove(param, FilterCallback);
		m_PreviewEntity.GetTransform(m_vCurrentMat);
		m_vCurrentMat[3] = param.Start + ((param.End - param.Start) * traceDistance);
		vector up = param.TraceNorm;
		m_vCurrentMat[3] = m_vCurrentMat[3] + up * 0.01;

		IEntity tracedEntity = param.TraceEnt;

		if (traceDistance == 1) // Assume we didn't hit anything and snap item on the ground
		{
			// Trace against terrain and entities to detect new placement position
			TraceParam paramGround = new TraceParam();
			paramGround.Start = param.End + vector.Up;
			paramGround.End = paramGround.Start - vector.Up * 20;
			paramGround.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			paramGround.Exclude = SCR_PlayerController.GetLocalControlledEntity();
			paramGround.LayerMask = EPhysicsLayerPresets.Projectile;
			float traceGroundDistance = world.TraceMove(paramGround, FilterCallback);
			m_PreviewEntity.GetTransform(m_vCurrentMat);
			m_vCurrentMat[3] = paramGround.Start + ((paramGround.End - paramGround.Start) * traceGroundDistance) + vector.Up * 0.01; // adding 1 cm to avoid collision with object under

			if (traceGroundDistance < 1)
				up = paramGround.TraceNorm;

			tracedEntity = paramGround.TraceEnt;
		}

		SCR_EntityHelper.OrientUpToVector(up, m_vCurrentMat);
		vector right = up * cameraMat[0];
		vector forward = up * right;
		right.Normalize();
		forward.Normalize();
		if (m_PlaceableItem.GetForwardAwayFromPlayer())
		{
			m_vCurrentMat[0] = -forward;
			m_vCurrentMat[2] = -right;
		}
		else
		{
			m_vCurrentMat[0] = forward;
			m_vCurrentMat[2] = right;
		}

#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_ITEM_PLACEMENT_VISUALIZATION))
		{
			ShapeFlags shapeFlags = ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOOUTLINE;
			Shape.CreateArrow(m_vCurrentMat[3], m_vCurrentMat[3] + up * 0.5, 0.1, Color.BLUE, shapeFlags);
			Shape.CreateArrow(m_vCurrentMat[3], m_vCurrentMat[3] + right * 0.5, 0.1, Color.ORANGE, shapeFlags);
			Shape.CreateArrow(m_vCurrentMat[3], m_vCurrentMat[3] + forward * 0.5, 0.1, Color.GREEN, shapeFlags);
		}
#endif

		m_PreviewEntity.SetTransform(m_vCurrentMat);
		m_PreviewEntity.Update();

		// Reject based on distance from character
		if (SCR_PlaceableItemComponent.GetDistanceFromCharacter(character, m_vCurrentMat[3], m_PlaceableItem.GetDistanceMeasurementMethod()) > maxPlacementDistance)
		{
			m_eCantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_DISTANCE;//this position is too far away
			return;
		}

		m_iTargetEntityNodeID = param.NodeIndex;
		m_aCamDeploymentPosition = cameraMat;
		m_aCamDeploymentPosition[3] = character.EyePosition();
		ValidatePlacement(m_vCurrentMat[1], tracedEntity, world, character, m_eCantPlaceReason);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \return
	protected bool ValidateEntity(notnull IEntity entity)
	{
		Physics physics = entity.GetPhysics();
		if (physics && physics.IsDynamic() && !m_PlaceableItem.CanAttachToDynamicObject())
			return false;

		// F. e. Slotted vehicle parts are physically static, but their main parent (vehicle) is not, we need to check that
		IEntity mainEntity = entity.GetRootParent();
		if (mainEntity && mainEntity != entity)
		{
			physics = mainEntity.GetPhysics();
			if (physics && physics.IsDynamic() && !m_PlaceableItem.CanAttachToDynamicObject())
				return false;
		}

		IEntity parent = entity;
		array<typename> ignoredComponents = {};
		m_PlaceableItem.GetIgnoredComponents(ignoredComponents);
		while (parent)
		{
			physics = parent.GetPhysics();
			if (physics && (physics.GetInteractionLayer() & m_PlaceableItem.GetIgnoredPhysicsLayers()))
				return false;

			if (ignoredComponents.IsEmpty())
			{
				parent = parent.GetParent();
				continue;
			}

			foreach (typename comp : ignoredComponents)
			{
				if (parent.FindComponent(comp))
					return false;
			}
			parent = parent.GetParent();
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] up
	//! \param[in] tracedEntity
	//! \param[in] world
	//! \param[in] character
	//! \return
	void ValidatePlacement(vector up, IEntity tracedEntity, BaseWorld world, IEntity character, out ENotification cantPlaceReason)
	{
		float maxAllowedTilt = m_PlaceableItem.GetMaxAllowedTilt();
		if (maxAllowedTilt >= 0) // Early reject based on the angle of placement
		{
			float currentTilt = Math.Acos(vector.Dot(m_vCurrentMat[1], vector.Up)) * Math.RAD2DEG;
			if (currentTilt > maxAllowedTilt)
			{
				SCR_Global.SetMaterial(m_PreviewEntity, m_sCannotBuildMaterial);
				cantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_TOO_STEEP;//angle too steep
				return;
			}
		}

		if (tracedEntity != m_TargetEntity)
		{
			m_bIsBeingAttachedToEntity = false;
			m_TargetEntity = null;
			m_TargetId = -1;
			m_iTargetEntityNodeID = -1;

			if (!ValidateEntity(tracedEntity)) // Reject on items with dynamic physics
			{
				cantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_GENERIC;//cant attach to this
				return;
			}

			Physics entPhys = tracedEntity.GetPhysics();
			if (m_PlaceableItem.GetAttachPlacedItemToTheSurfaceEntity() && entPhys && !(entPhys.GetInteractionLayer() & EPhysicsLayerDefs.Terrain))
				m_bIsBeingAttachedToEntity = true;
		}

		array<IEntity> excludeArray = {};
		excludeArray.Insert(m_PreviewEntity);
		excludeArray.Insert(character);

		// Reject based on bbox collision with surrounding objects - last check, as it's the most expensive one
		// Check bbox
		TraceOBB paramOBB = new TraceOBB();
		if (m_PlaceableItem.OverrideSpaceValidation(this, m_vCurrentMat, cantPlaceReason))
		{
			if (cantPlaceReason != 0)
				return;
		}
		else
		{
			Math3D.MatrixIdentity3(paramOBB.Mat);
			paramOBB.Mat[0] = m_vCurrentMat[0];
			paramOBB.Mat[1] = m_vCurrentMat[1];
			paramOBB.Mat[2] = m_vCurrentMat[2];
			paramOBB.Start = m_vCurrentMat[3] + 0.05 * paramOBB.Mat[1];
			paramOBB.Flags = TraceFlags.ENTS;
			paramOBB.ExcludeArray = excludeArray;
			paramOBB.LayerMask = EPhysicsLayerPresets.Projectile;
			m_PreviewEntity.GetBounds(paramOBB.Mins, paramOBB.Maxs);
			world.TracePosition(paramOBB, TraceEntitiesCallback);
		}

#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_ITEM_PLACEMENT_VISUALIZATION))
		{
			vector pos = m_vCurrentMat[3] + 0.05 * paramOBB.Mat[1];
			ShapeFlags shapeFlags = ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOOUTLINE;
			Shape.CreateArrow(pos, pos + m_vCurrentMat[1] * 0.5, 0.1, Color.WHITE, shapeFlags);
			Shape.CreateArrow(pos, pos + m_vCurrentMat[2] * 0.5, 0.1, Color.PINK, shapeFlags);
			Shape.CreateArrow(pos, pos + m_vCurrentMat[0] * 0.5, 0.1, Color.YELLOW, shapeFlags);
		}
#endif

		// collides with another entity
		if (paramOBB.TraceEnt)
		{
			cantPlaceReason = ENotification.PLACEABLE_ITEM_CANT_PLACE_NOT_ENOUGH_SPACE;//not enough space
			return;
		}

		if (tracedEntity && m_bIsBeingAttachedToEntity)
		{
			RplComponent rplComp = RplComponent.Cast(tracedEntity.FindComponent(RplComponent));
			if (rplComp)
				m_TargetId = rplComp.Id();
			else
				m_bIsBeingAttachedToEntity = false;
		}
		m_TargetEntity = tracedEntity;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] e
	//! \return
	bool TraceEntitiesCallback(notnull IEntity e, vector start = "0 0 0", vector dir = "0 0 0")
	{
		if (!e)
			return false;

		if (ChimeraCharacter.Cast(e.GetRootParent()))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorOpened()
	{
		m_bInEditor = true;
		DisablePreview();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorClosed()
	{
		m_bInEditor = false;
		TogglePreview(m_PlacedItem);
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterToManagerInvokers(SCR_EditorManagerEntity manager)
	{
		manager.GetOnOpened().Insert(OnEditorOpened);
		manager.GetOnClosed().Insert(OnEditorClosed);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	protected void InitEditorListeners()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return;

		SCR_EditorManagerEntity editorManager = core.GetEditorManager();

		if (!editorManager)
			core.Event_OnEditorManagerInitOwner.Insert(RegisterToManagerInvokers);
		else
			RegisterToManagerInvokers(editorManager);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		if (!playerController)
		{
			Print("SCR_ItemPlacementComponent is not attached to SCR_PlayerController!", LogLevel.WARNING);
			return;
		}

		Math3D.MatrixIdentity4(m_vCurrentMat);

		InitEditorListeners();

		playerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_ItemPlacementComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_iEquipComplete = GameAnimationUtils.RegisterAnimationEvent("EquipComplete");
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_ITEM_PLACEMENT_VISUALIZATION, "", "Item placement debugs", "Character");
#endif
	}
}
