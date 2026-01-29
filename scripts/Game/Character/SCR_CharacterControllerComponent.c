[ComponentEditorProps(category: "GameScripted/Character", description: "Scripted character controller", icon: HYBRID_COMPONENT_ICON)]
class SCR_CharacterControllerComponentClass : CharacterControllerComponentClass
{
}

//------------------------------------------------------------------------------------------------
void OnPlayerDeathWithParam(SCR_CharacterControllerComponent characterController, IEntity killerEntity, notnull Instigator killer);
typedef func OnPlayerDeathWithParam;
typedef ScriptInvokerBase<OnPlayerDeathWithParam> OnPlayerDeathWithParamInvoker;

//------------------------------------------------------------------------------------------------
void OnControlledByPlayer(IEntity ownerEntity, bool controlled);
typedef func OnControlledByPlayer;
typedef ScriptInvokerBase<OnControlledByPlayer> OnControlledByPlayerInvoker;

//------------------------------------------------------------------------------------------------
typedef func OnLifeStateChanged;
typedef ScriptInvokerBase<OnLifeStateChanged> OnLifeStateChangedInvoker;

//------------------------------------------------------------------------------------------------
void OnItemUseBegan(IEntity item, ItemUseParameters animParams);
typedef func OnItemUseBegan;
typedef ScriptInvokerBase<OnItemUseBegan> OnItemUseBeganInvoker;

//------------------------------------------------------------------------------------------------
void OnItemUseEnded(IEntity item, bool successful, ItemUseParameters animParams);
typedef func OnItemUseEnded;
typedef ScriptInvokerBase<OnItemUseEnded> OnItemUseEndedInvoker;

//------------------------------------------------------------------------------------------------
void OnItemUseFinished(IEntity item, bool successful, ItemUseParameters animParams);
typedef func OnItemUseFinished;
typedef ScriptInvokerBase<OnItemUseFinished> OnItemUseFinishedInvoker;

class SCR_CharacterControllerComponent : CharacterControllerComponent
{
	[Attribute(defvalue: "10", uiwidget: UIWidgets.EditBox, params:"1 inf 0.1", desc: "Maximum duration it takes for character to drown\n[s]")]
	protected float m_fDrowningDuration;
	
	[Attribute(defvalue: "4", uiwidget: UIWidgets.EditBox, params:"0 inf 0.1", desc: "Amount of raw damage needed for character to play an animated hitreaction")]
	protected float m_fAnimatedHitReactionThreshold;
	
	protected static ref array<EDamageType> s_aHitReactionDamageTypes = {
		EDamageType.COLLISION,
		EDamageType.MELEE,
		EDamageType.KINETIC,
		EDamageType.FRAGMENTATION,
		EDamageType.EXPLOSIVE
	};
	
	// Private members
	protected SCR_CharacterCameraHandlerComponent m_CameraHandler; // Set from the camera handler itself
	protected SCR_MeleeComponent m_MeleeComponent;
	protected bool m_bOverrideActions = true;
	protected bool m_bInspectionFocus;
	protected bool m_bCharacterIsDrowning;
	protected float m_fDrowningTime;
	protected ref SCR_ScriptedCharacterInputContext m_pScrInputContext;
	protected CharacterSoundComponent m_CharacterSoundComponent;

	//! Contains information about for which shell player saved some information for future use
	protected ref array<ref SCR_ShellConfig> m_aSavedShellConfigs;

	// Character event invokers
	ref ScriptInvokerVoid m_OnPlayerDeath = new ScriptInvokerVoid();
	ref OnPlayerDeathWithParamInvoker m_OnPlayerDeathWithParam = new OnPlayerDeathWithParamInvoker(); // has a getter!
	ref OnLifeStateChangedInvoker m_OnLifeStateChanged = new OnLifeStateChangedInvoker();
	ref OnControlledByPlayerInvoker m_OnControlledByPlayer = new OnControlledByPlayerInvoker();
	ref ScriptInvokerFloat2<float> m_OnPlayerDrowning = new ScriptInvokerFloat2();
	ref ScriptInvokerVoid m_OnPlayerStopDrowning = new ScriptInvokerVoid();
	
	// Gadget event invokers
	ref ScriptInvoker<IEntity, bool, bool> m_OnGadgetStateChangedInvoker = new ScriptInvoker<IEntity, bool, bool>();
	ref ScriptInvoker<IEntity, bool> m_OnGadgetFocusStateChangedInvoker = new ScriptInvoker<IEntity, bool>();

	// Item event invokers
	ref OnItemUseBeganInvoker m_OnItemUseBeganInvoker = new OnItemUseBeganInvoker();
	ref OnItemUseEndedInvoker m_OnItemUseEndedInvoker = new OnItemUseEndedInvoker();
	// called when all listeners reacted on ItemUseEnded invoker - may delete the item now thus listerners to ItemUseFinished may get first parameter null! 
	ref OnItemUseFinishedInvoker m_OnItemUseFinishedInvoker = new OnItemUseFinishedInvoker();
	protected ref ScriptInvoker<AnimationEventID, AnimationEventID, int, float, float, SCR_CharacterControllerComponent> m_OnAnimationEvent;
	
	// Weapon raising/lowering event invokers
	ref ScriptInvokerVoid m_OnWeaponRaisingStartedInvoker = new ScriptInvokerVoid();
	ref ScriptInvokerVoid m_OnWeaponRaisingFinishedInvoker = new ScriptInvokerVoid();
	ref ScriptInvokerVoid m_OnWeaponLoweringStartedInvoker = new ScriptInvokerVoid();
	ref ScriptInvokerVoid m_OnWeaponLoweringFinishedInvoker = new ScriptInvokerVoid();
	
	// Diagnostics, debugging
	#ifdef ENABLE_DIAG
	private static bool s_bDiagRegistered;
	private static bool m_bEnableDebugUI;
	private Widget m_wDebugRootWidget;
	private ECharacterStance m_bDebugLastStance = ECharacterStance.STAND;
	#endif

	//------------------------------------------------------------------------------------------------
	//! Get the interaction status of the character
	//! \return true if the character is alive ad not using items or vehicles
	//! not on a ladder, not aiming down sights, not in 3rd person in a vehicle, not already using an item nor weapon-deployed
	bool CanInteract()
	{
		if (GetLifeState() != ECharacterLifeState.ALIVE)
			return false;
		
		if (IsUsingItem())
			return false;
		
		if (IsClimbing())
			return false;

		ChimeraCharacter character = GetCharacter();
		if (!character || character.IsInVehicleADS())
			return false;
		
		// Disable in vehicle 3pp
		if (character.IsInVehicle() && IsInThirdPersonView())
			return false;
		
		if (GetIsWeaponDeployed())
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnConsciousnessChanged(bool conscious)
	{
		if (GetLifeState() != ECharacterLifeState.INCAPACITATED)
			return;

		AIControlComponent aiControl = AIControlComponent.Cast(GetOwner().FindComponent(AIControlComponent));
		if (!aiControl || !aiControl.IsAIActivated())
			return;
		
		IEntity currentWeapon;
		BaseWeaponManagerComponent wpnMan = GetWeaponManagerComponent();
		if (wpnMan && wpnMan.GetCurrentWeapon())
			currentWeapon = wpnMan.GetCurrentWeapon().GetOwner();
				
		if (currentWeapon)
		{
			bool dropGrenade = false;
			
			SCR_CharacterCommandHandlerComponent handler = SCR_CharacterCommandHandlerComponent.Cast(GetAnimationComponent().GetCommandHandler());
			
			EWeaponType wt = wpnMan.GetCurrentWeapon().GetWeaponType();
			if (currentWeapon.FindComponent(GrenadeMoveComponent))
			{
				BaseTriggerComponent triggerComp = BaseTriggerComponent.Cast(currentWeapon.FindComponent(BaseTriggerComponent));
				
				if ((triggerComp && triggerComp.WasTriggered()) || (handler && handler.IsThrowingAction()))
				{
					dropGrenade = true;
				}
			}
			
			if (dropGrenade)
				handler.DropLiveGrenadeFromHand(false); 
			else 
				TryEquipRightHandItem(null, EEquipItemType.EEquipTypeUnarmedContextual, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanJumpClimb()
	{
		SCR_ExtendedDamageManagerComponent damageMan = SCR_ExtendedDamageManagerComponent.Cast(GetCharacter().GetDamageManager());
		if (!damageMan)
			return true;
		
		array<ref SCR_PersistentDamageEffect> effects = damageMan.GetAllPersistentEffectsOfType(SCR_SpecialCollisionDamageEffect, true);
		foreach (SCR_PersistentDamageEffect effect : effects)
		{
			SCR_SpecialCollisionDamageEffect collisionEffect = SCR_SpecialCollisionDamageEffect.Cast(effect);
			if (!collisionEffect.GetJumpingAndClimbingAllowed())
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return a script invoker with method signature (AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	ScriptInvoker GetOnAnimationEvent()
	{
		if (!m_OnAnimationEvent)
			m_OnAnimationEvent = new ScriptInvoker();

		return m_OnAnimationEvent;
	}

	//------------------------------------------------------------------------------------------------
	override void OnGadgetStateChanged(IEntity gadget, bool isInHand, bool isOnGround)
	{
		m_OnGadgetStateChangedInvoker.Invoke(gadget, isInHand, isOnGround);
	}

	//------------------------------------------------------------------------------------------------
	override void OnGadgetFocusStateChanged(IEntity gadget, bool isFocused)
	{
		m_OnGadgetFocusStateChangedInvoker.Invoke(gadget, isFocused);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerVoid GetOnWeaponRaisingStarted()
	{
		return m_OnWeaponRaisingStartedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerVoid GetOnWeaponRaisingFinished()
	{
		return m_OnWeaponRaisingFinishedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerVoid GetOnWeaponLoweringStarted()
	{
		return m_OnWeaponLoweringStartedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerVoid GetOnWeaponLoweringFinished()
	{
		return m_OnWeaponLoweringFinishedInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnWeaponRaisingStarted()
	{
		m_OnWeaponRaisingStartedInvoker.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnWeaponRaisingFinished()
	{
		m_OnWeaponRaisingFinishedInvoker.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnWeaponLoweringStarted()
	{
		m_OnWeaponLoweringStartedInvoker.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnWeaponLoweringFinished()
	{
		m_OnWeaponLoweringFinishedInvoker.Invoke();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnItemUseBegan(ItemUseParameters itemUseParams) 
	{
		m_OnItemUseBeganInvoker.Invoke(itemUseParams.GetEntity(), itemUseParams);
	}

	//------------------------------------------------------------------------------------------------
	override void OnItemUseEnded(ItemUseParameters itemUseParams, bool successful)
	{
		m_OnItemUseEndedInvoker.Invoke(itemUseParams.GetEntity(), successful, itemUseParams);
		// now all interested in non-null item have listened, we can call Finished to delete the item
		m_OnItemUseFinishedInvoker.Invoke(itemUseParams.GetEntity(), successful, itemUseParams);

		IEntity gadget = GetItemInHandSlot();
		if (!gadget)
			return;

		SCR_CharacterInventoryStorageComponent characterInventory = SCR_CharacterInventoryStorageComponent.Cast(GetOwner().FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!characterInventory)
			return;

		characterInventory.UseItem(gadget);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	{
		if (m_OnAnimationEvent)
			m_OnAnimationEvent.Invoke(animEventType, animUserString, intParam, timeFromStart, timeToEnd, this);
	}

	//------------------------------------------------------------------------------------------------
	// handling of melee events. Sends true if melee started, false, when melee ends
	override void OnMeleeDamage(bool started)
	{
		m_MeleeComponent.SetMeleeAttackStarted(started);
	}
	
	//------------------------------------------------------------------------------------------------
	// play hitreaction when taking momentary physical damage 
	protected override EHitReactionType ComputeHitReaction(float damageValue, EDamageType damageType)
	{
		if (damageValue < m_fAnimatedHitReactionThreshold)
			return EHitReactionType.HIT_REACTION_NONE;
		
		if (!s_aHitReactionDamageTypes.Contains(damageType))
			return EHitReactionType.HIT_REACTION_NONE;
		
		return EHitReactionType.HIT_REACTION_LIGHT;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerVoid GetOnPlayerDeath()
	{
		return m_OnPlayerDeath;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	OnPlayerDeathWithParamInvoker GetOnPlayerDeathWithParam()
	{
		return m_OnPlayerDeathWithParam;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDeath(IEntity instigatorEntity, notnull Instigator instigator)
	{
		m_OnPlayerDeath.Invoke();
		m_OnPlayerDeathWithParam.Invoke(this, instigatorEntity, instigator);

		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (pc && m_CameraHandler && m_CameraHandler.IsInThirdPerson())
			pc.m_bRetain3PV = true;

		IEntity character = GetCharacter();
		auto garbageSystem = SCR_GarbageSystem.GetByEntityWorld(character);
		if (garbageSystem)
			garbageSystem.Insert(character);
	}

	//------------------------------------------------------------------------------------------------
	override void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState, bool isJIP)
	{
		m_OnLifeStateChanged.Invoke(previousLifeState, newLifeState, isJIP);
				
		IEntity vehicle = CompartmentAccessComponent.GetVehicleIn(GetCharacter());
		if (!vehicle)
			return;
		
		SCR_VehicleFactionAffiliationComponent vehicleFactionAff = SCR_VehicleFactionAffiliationComponent.Cast(vehicle.FindComponent(SCR_VehicleFactionAffiliationComponent));
		if (!vehicleFactionAff)
			return;
		
		vehicleFactionAff.UpdateOccupantsCount();	
	}

	//------------------------------------------------------------------------------------------------
	override bool ShouldGadgetBeDropped(IEntity gadget)
	{
		if (!gadget)
			return false;

		if (GetItemInHandSlot() != gadget)
			return false;

		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(GetOwner().FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryManager)
			return true;

		BaseInventoryStorageComponent storage = inventoryManager.FindStorageForItem(gadget, EStoragePurpose.PURPOSE_DEPOSIT);
		if (storage && inventoryManager.TryMoveItemToStorage(gadget, storage))
		{
			if (GetLifeState() != ECharacterLifeState.DEAD && GetOwner() == SCR_PlayerController.GetLocalControlledEntity())
				SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_HAND_ITEM_PUT_IN_INVENTORY);

			return false;
		}

		if (GetLifeState() != ECharacterLifeState.DEAD && GetCharacter() == SCR_PlayerController.GetLocalControlledEntity())
			SCR_NotificationsComponent.SendLocal(ENotification.PLAYER_HAND_ITEM_DROPPED);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Return item currently stored in the hand slot
	IEntity GetItemInHandSlot()
	{
		SCR_HandSlotStorageComponent handSlot = SCR_HandSlotStorageComponent.Cast(GetOwner().FindComponent(SCR_HandSlotStorageComponent));
		if (!handSlot)
			return null;

		InventoryStorageSlot slot = handSlot.GetSlot(0);
		if (!slot)
			return null;

		return slot.GetAttachedEntity();
	}

	//------------------------------------------------------------------------------------------------
	//! Saves new or updates existing shell config with provided data
	//! \param[in] config
	void SetShellConfig(notnull SCR_ShellConfig config)
	{
		SCR_ShellConfig oldShellConfig = GetSavedShellConfig(config.GetPrefabData());
		if (!oldShellConfig)
		{
			if (!m_aSavedShellConfigs)
				m_aSavedShellConfigs = {};

			m_aSavedShellConfigs.Insert(config);
			return;
		}

		oldShellConfig.SetSavedTime(config.GetSavedTime());
		oldShellConfig.SetManualTimeUsage(config.IsUsingManualTime());
		oldShellConfig.SetChargeRingConfigId(config.GetChargeRingConfigId());
	}

	//------------------------------------------------------------------------------------------------
	SCR_ShellConfig GetSavedShellConfig(notnull EntityPrefabData shellPrefab)
	{
		if (!m_aSavedShellConfigs)
			return null;

		foreach (SCR_ShellConfig savedConfig : m_aSavedShellConfigs)
		{
			if (!savedConfig)
				continue;

			if (shellPrefab != savedConfig.GetPrefabData())
				continue;

			return savedConfig;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Method called by the owner of this controller for synchronizing the charge ring config
	//! \param[in] shellComp component of the shell for which charge ring config should be changed
	//! \param[in] configId id of the charge ring config that should be used by other clients
	void SyncShellChargeRingConfig(notnull SCR_MortarShellGadgetComponent shellComp, int configId)
	{
		RplComponent rplComp = SCR_EntityHelper.GetEntityRplComponent(shellComp.GetOwner());
		if (!rplComp)
			return;

		Rpc(RPC_AskSetShellChargeRing, rplComp.Id(), configId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_AskSetShellChargeRing(RplId replicationId, int configId)
	{
		if (!replicationId.IsValid())
			return;

		Rpc(RPC_DoSetShellChargeRing, replicationId, configId);
		RPC_DoSetShellChargeRing(replicationId, configId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoSetShellChargeRing(RplId replicationId, int configId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(replicationId));
		if (!rplComp)
			return;

		SCR_MortarShellGadgetComponent shellComp = SCR_MortarShellGadgetComponent.Cast(rplComp.GetEntity().FindComponent(SCR_MortarShellGadgetComponent));
		if (!shellComp)
			return;

		shellComp.SetChargeRingConfig(configId, true, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Method for proxy in order to ask the authority to sync the change that he requested for currently used turret weapon system
	//! \param[in] change that was made in the FireModeManagerComponent
	//! \param[in] newValue
	//! \param[in] turretRplId in replication system
	void ReplicateTurretFireModeChange(SCR_EFireModeChange change, int newValue, RplId turretRplId)
	{
		if (!turretRplId.IsValid())
			return;

		Rpc(RPC_AskTurretFireModeChange, change, newValue, turretRplId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_AskTurretFireModeChange(SCR_EFireModeChange change, int newValue, RplId turretRplId)
	{
		if (!turretRplId.IsValid())
			return;

		RplComponent turretRplComp = RplComponent.Cast(Replication.FindItem(turretRplId));
		if (!turretRplComp)
			return;

		Turret turret = Turret.Cast(turretRplComp.GetEntity());
		if (!turret)
			return;

		SCR_FireModeManagerComponent fireModeMgr = SCR_FireModeManagerComponent.Cast(turret.FindComponent(SCR_FireModeManagerComponent));
		if (!fireModeMgr)
			return;

		fireModeMgr.ChangeFireModeValues(change, newValue);
	}

	//------------------------------------------------------------------------------------------------
	//! Method for proxy in order to ask the authority to sync the state of the helicopter collimator sight with other clients
	//! \param[in] newState
	//! \param[in] sightId in replication system
	void ReplicateHelicopterSightState(bool newState, RplId sightId)
	{
		if (!sightId.IsValid())
			return;

		Rpc(RPC_AskSyncSightState, newState, sightId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RPC_AskSyncSightState(bool newState, RplId sightId)
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(sightId));
		if (!rplComp)
			return;

		IEntity owner = rplComp.GetEntity();
		if (!owner)
			return;

		SCR_HelicopterCollimatorSightComponent collimator = SCR_HelicopterCollimatorSightComponent.Cast(owner.FindComponent(SCR_HelicopterCollimatorSightComponent));
		if (!collimator)
			return;

		collimator.OwnerSyncSightState(newState);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetCanMeleeAttack()
	{
		if (!m_MeleeComponent)
			return false;

		if (IsFalling())
			return false;

		if (GetStance() == ECharacterStance.PRONE)
			return false;

		// TODO: Gadget melee weapon properties in case we want to be able to have melee component, like a shovel?
		if (IsGadgetInHands())
			return false;

		//! check presence of MeleeWeaponProperties component to ensure it is an Melee weapon or not
		BaseWeaponManagerComponent weaponManager = GetWeaponManagerComponent();
		if (weaponManager && !SCR_WeaponLib.CurrentWeaponHasComponent(weaponManager, SCR_MeleeWeaponProperties))
			return false;

		return true;
	}
	
	override bool GetCanEquipGadget(IEntity gadget)
	{
		SCR_CharacterCommandHandlerComponent handler = SCR_CharacterCommandHandlerComponent.Cast(GetAnimationComponent().GetCommandHandler());
		if (handler.IsLoitering() || GetScrInputContext().m_iLoiteringType != -1)
			return false;

		return true;		
	}

	//------------------------------------------------------------------------------------------------
	//! Override to handle what happens after pressing F button
	//! \return true to override default behaviour, false to use default behaviour - Perform Action key will immediately perform the action
	override bool OnPerformAction()
	{
		return m_bOverrideActions;
	}

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity owner)
	{
		ChimeraCharacter character = GetCharacter();

		if (!m_MeleeComponent)
			m_MeleeComponent = SCR_MeleeComponent.Cast(character.FindComponent(SCR_MeleeComponent));
		if (!m_CameraHandler)
			m_CameraHandler = SCR_CharacterCameraHandlerComponent.Cast(character.FindComponent(SCR_CharacterCameraHandlerComponent));
		
		m_pScrInputContext = new SCR_ScriptedCharacterInputContext();
		
		m_CharacterSoundComponent = CharacterSoundComponent.Cast(character.FindComponent(CharacterSoundComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnApplyControls(IEntity owner, float timeSlice)
	{
		if (GetScrInputContext().m_iLoiteringType >= 0)
		{
			SCR_CharacterCommandHandlerComponent handler = SCR_CharacterCommandHandlerComponent.Cast(GetAnimationComponent().GetCommandHandler());
			if (!handler.IsLoitering())
			{
				if (TryStartLoiteringInternal())
					return;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsUsingBinoculars()
	{
		return SCR_BinocularsComponent.IsZoomedView();
	}

	//------------------------------------------------------------------------------------------------
	protected override void UpdateDrowning(float timeSlice, vector waterLevel)
	{
		if (GetLifeState() == ECharacterLifeState.DEAD)
			return;

		ChimeraCharacter char = GetCharacter();
		
		CompartmentAccessComponent accesComp = char.GetCompartmentAccessComponent();
		bool isInWatertightCompartment = accesComp && accesComp.GetCompartment() && accesComp.GetCompartment().GetIsWaterTight();

		const float drowningTimeStartFX = 4;
		if (waterLevel[2] > 0 && !isInWatertightCompartment)
		{
			if (m_fDrowningTime < drowningTimeStartFX && (m_fDrowningTime + timeSlice) > drowningTimeStartFX)
			{
				m_OnPlayerDrowning.Invoke(m_fDrowningDuration, drowningTimeStartFX);
				m_bCharacterIsDrowning = true;
			}
			
			m_fDrowningTime += timeSlice;
		}
		else
		{
			if (m_fDrowningTime != 0)
			{
				m_OnPlayerStopDrowning.Invoke();
				m_bCharacterIsDrowning = false;
			}

			m_fDrowningTime = 0;
			return;
		}

		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(char.GetDamageManager());
		if (!damageMan)
			return;
		
		if (m_fDrowningTime > m_fDrowningDuration)
		{
			HitZone targetHitZone;
			if (IsUnconscious())
				targetHitZone = damageMan.GetHeadHitZone();
			else
				targetHitZone = damageMan.GetResilienceHitZone();
			
			if (!targetHitZone)
				return;
			
			vector hitPosDirNorm[3];
			SCR_DamageContext context = new SCR_DamageContext(EDamageType.TRUE, 1000, hitPosDirNorm, char, targetHitZone, null, null, -1, -1);
			context.damageEffect = new SCR_DrowningDamageEffect();
			damageMan.HandleDamage(context);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetDrowningTime()
	{
		return m_fDrowningTime;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsCharacterDrowning()
	{
		return m_bCharacterIsDrowning;
	}

	//------------------------------------------------------------------------------------------------
	override bool ShouldAligningAdjustAimingAngles()
	{
		return IsAligningBeforeLoiter();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsAligningBeforeLoiter()
	{
		return GetScrInputContext().m_iLoiteringType != -1 && GetScrInputContext().m_bLoiteringShouldAlignCharacter && GetAnimationComponent().GetHeadingComponent().IsAligning();
	}
	
	//------------------------------------------------------------------------------------------------
	// why the SCR_ prefix?
	override bool SCR_GetDisableMovementControls()
	{
		SCR_CharacterCommandHandlerComponent handler = SCR_CharacterCommandHandlerComponent.Cast(GetAnimationComponent().GetCommandHandler());
		if (!handler)
			return false;
		
		if (GetScrInputContext().m_iLoiteringType > 0)
		{
			bool isAligningBeforeLoiter = IsAligningBeforeLoiter();
			bool isHolsteringBeforeLoiter = GetScrInputContext().m_iLoiteringShouldHolsterWeapon && IsChangingItem();
			
			if (isAligningBeforeLoiter || isHolsteringBeforeLoiter)
				return true;
		}
		
		return handler.IsLoitering();
	}

	//------------------------------------------------------------------------------------------------
	override void SCR_OnDisabledJumpAction()
	{
		SCR_CharacterCommandHandlerComponent handler = SCR_CharacterCommandHandlerComponent.Cast(GetAnimationComponent().GetCommandHandler());
		if (!handler)
			return;
		
		if (handler.IsLoitering())
		{
			if (m_pScrInputContext.m_bLoiteringDisablePlayerInput)
				return;

			StopLoitering(false);
		}
		
		if (IsAligningBeforeLoiter())
		{
			CharacterHeadingAnimComponent headingComponent = GetAnimationComponent().GetHeadingComponent();
			if (headingComponent)
			{
				headingComponent.ResetAligning();
				GetScrInputContext().m_iLoiteringType = -1;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// #ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	#ifdef ENABLE_DIAG
		//------------------------------------------------------------------------------------------------
		override void OnDiag(IEntity owner, float timeslice)
		{
			ChimeraCharacter character = GetCharacter();
			if (IsDead())
				return;
			
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_NOBANKING))
				SetBanking(0);

			bool diagTransform = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_TRANSFORMATION);
			bool bIsLocalCharacter = SCR_PlayerController.GetLocalControlledEntity() == character;

			// Character Transformation Diag
			if (bIsLocalCharacter && diagTransform)
			{
				DbgUI.Begin("Character Transformation");
				const string CHAR_POS = "X:\t%1\nY:\t%2\nZ:\t%3";
				const string CHAR_ROT = "PITCH:\t%1\nYAW:\t%2\nROLL:\t%3";
				const string CHAR_SCALE = "%1";
				const string CHAR_AIM = "X:\t%1\nY:\t%2\nZ:\t%3";
				vector pos = character.GetOrigin();
				vector rot = character.GetYawPitchRoll();
				float scale = character.GetScale();
				vector aimRot = GetInputContext().GetAimingAngles();
				string strPosition = string.Format(CHAR_POS, pos[0].ToString(), pos[1].ToString(), pos[2].ToString());
				string strRotation = string.Format(CHAR_ROT, rot[1].ToString(), rot[0].ToString(), rot[2].ToString());
				string strScale = string.Format(CHAR_SCALE, scale.ToString());
				string strAiming = string.Format(CHAR_AIM, aimRot[0].ToString(), aimRot[1].ToString(), aimRot[2].ToString());
				DbgUI.Text("POSITION:\n" + strPosition);
				DbgUI.Text("ROTATION:\n" + strRotation);
				DbgUI.Text("SCALE:" + strScale);
				DbgUI.Text("AIMING ANGLES:\n" + strAiming);

				if (DbgUI.Button("Print plaintext to console"))
					Print("TransformInfo:\nPOSITION:\n" + strPosition + "\nROTATION:\n" + strRotation + "\nSCALE:" + strScale + "\nAIMING ANGLES:\n" + strAiming, LogLevel.NORMAL);

				DbgUI.End();
			}

			// Character Controller diag (inputs...)
			m_bEnableDebugUI = bIsLocalCharacter && DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_MENU);
			if (m_bEnableDebugUI && !m_wDebugRootWidget) // Currently controlled player
				CreateDebugUI();
			else if (!m_bEnableDebugUI && m_wDebugRootWidget)
				DeleteDebugUI();

			if (m_bEnableDebugUI)
				UpdateDebugUI();
		}

		//------------------------------------------------------------------------------------------------
		private void ReloadDebugUI()
		{
			if (m_wDebugRootWidget)
				DeleteDebugUI();

			CreateDebugUI();
		}

		//------------------------------------------------------------------------------------------------
		private void DeleteDebugUI()
		{
			m_wDebugRootWidget.RemoveFromHierarchy();
			m_wDebugRootWidget = null;
		}

		//------------------------------------------------------------------------------------------------
		private void UpdateInputCircles()
		{
			if (!m_wDebugRootWidget)
				return;

			CharacterAnimationComponent animComponent = GetAnimationComponent();
			if (!animComponent)
				return;

			Widget wInput = m_wDebugRootWidget.FindAnyWidget("Input");
			if (!wInput)
				return;

			WorkspaceWidget workspaceWidget = GetGame().GetWorkspace();
			
			float topSpeed = animComponent.GetTopSpeed(-1, false);

			TextWidget wMaxSpeed = TextWidget.Cast(wInput.FindAnyWidget("MaxSpeed"));
			if (wMaxSpeed)
			{
				float speed = Math.Ceil(topSpeed * 10) * 0.1;
				wMaxSpeed.SetText(speed.ToString() + "m/s");
			}

			float ringSize = FrameSlot.GetSizeX(wInput);
			float degSizeDivider = topSpeed * ringSize;
			if (degSizeDivider > 0)
			for (int spd = 0; spd < 3; spd++)
			{
				int spdType;
				if (spd == 0)
					spdType = EMovementType.WALK;
				else
				if (spd == 1)
					spdType = EMovementType.RUN;
				else
				if (spd == 2)
					spdType = EMovementType.SPRINT;

				int color;
				if (spd == 0)
					color = ARGB(255, 150, 255, 150);
				else
				if (spd == 1)
					color = ARGB(255, 200, 200, 150);
				else
				if (spd == 2)
					color = ARGB(255, 255, 150, 150);

				for (int deg = 0; deg < 360; deg++)
				{
					vector velInput = GetMovementVelocity();
					float inputForward = velInput[2];
					float inputRight = velInput[0];

					float degSize = animComponent.GetMaxSpeed(inputForward, inputRight, spdType) / degSizeDivider;
					if (spdType == EMovementType.SPRINT && !IsSprinting())
						degSize = 0;

					float degOff = (ringSize - degSize) * 0.5;

					string iRingname = "iRing_" + spd.ToString() + "_" + deg.ToString();

					ImageWidget wImg = ImageWidget.Cast(wInput.FindAnyWidget(iRingname));
					if (!wImg)
					{
						wImg = ImageWidget.Cast(workspaceWidget.CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.BLEND | WidgetFlags.VISIBLE | WidgetFlags.STRETCH | WidgetFlags.NOWRAP, Color.FromInt(color), spd + 2, wInput));
						wImg.LoadImageTexture(0, "{90DF4F065D08EF4E}UI/Textures/HUD_obsolete/character/input_360ringslice.edds");
						wImg.SetRotation(deg);
						wImg.SetName(iRingname);
					}

					FrameSlot.SetAnchorMax(wImg, 0, 0);
					FrameSlot.SetAnchorMin(wImg, 0, 0);
					FrameSlot.SetSize(wImg, degSize, degSize);
					FrameSlot.SetPos(wImg, degOff, degOff);
				}
			}
		}

		//------------------------------------------------------------------------------------------------
		private void CreateDebugUI()
		{
			m_wDebugRootWidget = GetGame().GetWorkspace().CreateWidgets("{C70DA11469BBAF67}UI/layouts/Debug/HUD_Debug_Character.layout", null);
			UpdateInputCircles();
		}

		//------------------------------------------------------------------------------------------------
		private void UpdateDebugBoolWidget(string textWidgetName, bool isTrue, float time = 0)
		{
			TextWidget wTxt = TextWidget.Cast(m_wDebugRootWidget.FindAnyWidget(textWidgetName));
			if (!wTxt)
				return;

			time = Math.Ceil(time * 10) * 0.1;
			if (isTrue)
			{
				wTxt.SetColorInt(ARGB(255, 160, 210, 255));
				if (time != 0)
					wTxt.SetText("YES (" + time.ToString() + ")");
				else
					wTxt.SetText("YES");
			}
			else
			{
				wTxt.SetColorInt(ARGB(255, 255, 160, 160));
				if (time != 0)
					wTxt.SetText("NO (" + time.ToString() + ")");
				else
					wTxt.SetText("NO");
			}
		}

		//------------------------------------------------------------------------------------------------
		private void UpdateDebugUI()
		{
			ChimeraCharacter character = GetCharacter();

			// Reload debug UI for stance change
			if (m_bDebugLastStance != GetStance())
			{
				m_bDebugLastStance = GetStance();
				UpdateInputCircles();
			}

			float topSpeed = 0;
			CharacterAnimationComponent animComponent = GetAnimationComponent();
			topSpeed = animComponent.GetTopSpeed(-1, false);

			Widget wCenter = m_wDebugRootWidget.FindAnyWidget("Center");
			Widget wCenter2 = m_wDebugRootWidget.FindAnyWidget("Center2");
			Widget wCenter3 = m_wDebugRootWidget.FindAnyWidget("Center3");

			vector movementInput = GetMovementInput();
			if (movementInput != vector.Zero)
			{
				float inputLength = movementInput.Length();
				if (inputLength > 1)
					movementInput *= 1 / inputLength;
			}

			if (wCenter && wCenter2 && wCenter3 && topSpeed > 0)
			{
				float inputForward = movementInput[0];
				float inputRight = movementInput[2];
				float maxSpeed = animComponent.GetMaxSpeed(inputForward, inputRight, GetCurrentMovementPhase());
				float moveScale = maxSpeed / topSpeed;
				float x = movementInput[0] * moveScale * 0.5 + 0.5;
				float y = -movementInput[2] * moveScale * 0.5 + 0.5;
				FrameSlot.SetAnchorMin(wCenter, x, y);
				FrameSlot.SetAnchorMax(wCenter, x, y);

				vector moveLocal = animComponent.GetInertiaSpeed();
				x = (moveLocal[0] / topSpeed) * 0.5 + 0.5;
				y = (-moveLocal[2] / topSpeed) * 0.5 + 0.5;
				FrameSlot.SetAnchorMin(wCenter2, x, y);
				FrameSlot.SetAnchorMax(wCenter2, x, y);

				moveLocal = character.VectorToLocal(GetVelocity());
				x = (moveLocal[0] / topSpeed) * 0.5 + 0.5;
				y = (-moveLocal[2] / topSpeed) * 0.5 + 0.5;
				FrameSlot.SetAnchorMin(wCenter3, x, y);
				FrameSlot.SetAnchorMax(wCenter3, x, y);
			}

			TextWidget wSpeed = TextWidget.Cast(m_wDebugRootWidget.FindAnyWidget("Speed"));
			TextWidget wSpeed2 = TextWidget.Cast(m_wDebugRootWidget.FindAnyWidget("Speed2"));
			if (wSpeed && wSpeed2)
			{
				float speed = Math.Ceil(animComponent.GetInertiaSpeed().Length() * 10) * 0.1;
				wSpeed.SetText(speed.ToString() + "m/s");

				speed = Math.Ceil(GetVelocity().Length() * 10) * 0.1;
				wSpeed2.SetText(speed.ToString() + "m/s");
			}

			TextWidget wAdjustedSpeed = TextWidget.Cast(m_wDebugRootWidget.FindAnyWidget("AdjustedSpeed"));
			if (wAdjustedSpeed)
				wAdjustedSpeed.SetText(GetDynamicSpeed().ToString());

			TextWidget wAdjustedStance = TextWidget.Cast(m_wDebugRootWidget.FindAnyWidget("AdjustedStance"));
			if (wAdjustedStance)
				wAdjustedStance.SetText(GetDynamicStance().ToString());

			UpdateDebugBoolWidget("ToggleSprint", GetIsSprintingToggle());
			UpdateDebugBoolWidget("IsInADS", IsWeaponADS());
			UpdateDebugBoolWidget("IsWeaponHolstered", !IsWeaponRaised());
			UpdateDebugBoolWidget("CanFireWeapon", GetCanFireWeapon());
			UpdateDebugBoolWidget("CanThrow", GetCanThrow());
			UpdateDebugBoolWidget("InFreeLook", IsFreeLookEnabled());

			TextWidget wMovementAngle = TextWidget.Cast(m_wDebugRootWidget.FindAnyWidget("MovementAngle"));
			CharacterCommandHandlerComponent handler = animComponent.GetCommandHandler();
			if (!wMovementAngle || !handler)
				return;

			CharacterCommandMove moveCmd = handler.GetCommandMove();
			if (moveCmd)
			{
				float currAngle = moveCmd.GetMovementSlopeAngle();
				wMovementAngle.SetText(currAngle.ToString() + "deg");
			}
		}

	//------------------------------------------------------------------------------------------------
	// #endif ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	#endif

	//------------------------------------------------------------------------------------------------
	override void OnPrepareControls(IEntity owner, ActionManager am, float dt, bool player)
	{
		if (am.GetActionTriggered("JumpOut") && CanJumpOutVehicleScript())
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(owner);
			CompartmentAccessComponent compAccess = character.GetCompartmentAccessComponent();
			
			BaseCompartmentSlot compartment = compAccess.GetCompartment();
			if (compartment)
			{
				CompartmentUserAction action = compartment.GetJumpOutAction();
				if (action && action.CanBePerformed(GetOwner()))
				{
					action.PerformAction(compartment.GetOwner(), GetOwner());
					return;
				}

				// Add the following once the correct action exists
				//if (!action && compAccess.CanGetOutVehicle())
				//{
				//	compAccess.GetOutVehicle(-1);
				//}
			}			
		}
		
		if (am.GetActionTriggered("GetOut") && CanGetOutVehicleScript())
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(owner);
			CompartmentAccessComponent compAccess = character.GetCompartmentAccessComponent();
			
			BaseCompartmentSlot compartment = compAccess.GetCompartment();
			if (compartment)
			{
				CompartmentUserAction action = compartment.GetGetOutAction();
				if (action && action.CanBePerformed(GetOwner()))
				{
					action.PerformAction(compartment.GetOwner(), GetOwner());
					return;
				}

				if (!action && compAccess.CanGetOutVehicle())
				{
					compAccess.GetOutVehicle(EGetOutType.ANIMATED, -1, ECloseDoorAfterActions.INVALID, false);
				}
			}
		}

		if (GetStance() == ECharacterStance.PRONE)
		{
			float value = am.GetActionValue("CharacterRoll");
			int rollValue = 0;
			if (value < -0.5)
				rollValue = 1;
			else if (value > 0.5)
				rollValue = 2;

			// If one wants to use hold action - it needs too be allowed on CharacterControllerComponent at character prefab or by calling EnableHoldInputForRoll(true) during construction/initialization
			if (ShouldHoldInputForRoll())
				SetRoll(rollValue);
			else if (rollValue != 0)
				SetRoll(rollValue);
		}
		
		if (player && GetAnimationComponent().IsWeaponADSTag())
		{
			SightsComponent sights = GetWeaponManagerComponent().GetCurrentSights();
			if (sights)
			{
				SCR_SightsZoomFOVInfo fovInfo = SCR_SightsZoomFOVInfo.Cast(sights.GetFOVInfo());
				if (fovInfo && fovInfo.GetStepsCount() > 1)
					am.ActivateContext("CharacterWeaponMagnificationContext");
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_CharacterControllerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		#ifdef ENABLE_DIAG
		if (System.IsCLIParam("CharacterDebugUI")) // If the startup parameter is set, enable the debug UI
			m_bEnableDebugUI = true;

		if (!s_bDiagRegistered)
		{
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_MENU, "", "Enable Debug UI", "Character");
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CHARACTER_MENU, m_bEnableDebugUI);
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_NOBANKING, "", "Disable Banking", "Character");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_TRANSFORMATION, "", "Enable transform info", "Character");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_RECOIL_CAMERASHAKE_DISABLE, "", "Disable recoil cam shake", "Character");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_ADDITIONAL_CAMERASHAKE_DISABLE, "", "Disable additional cam shake", "Character");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_CAMERASHAKE_TEST_WINDOW, "", "Show shake diag", "Character");
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_ALLOW_INSPECTION_LOOKAT, "", "Inspection LookAt", "Character");
			s_bDiagRegistered = true;
		}
		#endif
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CharacterControllerComponent()
	{
		#ifdef ENABLE_DIAG
		if (m_wDebugRootWidget)
			DeleteDebugUI();
		#endif
	}

	//------------------------------------------------------------------------------------------------
	//! If a weapon with sights is equipped, advances to desired sights FOV info.
	//! \param[in] direction If above zero, advances to next info. If equal to or below zero, advances to previous info.
	//! \param[in] allowCycling If enabled, selection will cycle from end to start and from start to end, otherwise it will be clamped.
	void SetNextSightsFOVInfo(int direction = 1, bool allowCycling = false)
	{
		if (direction == 0)
			return;

		SightsFOVInfo fovInfo = GetSightsFOVInfo();
		if (!fovInfo)
			return;

		SCR_BaseVariableSightsFOVInfo variableFovInfo = SCR_BaseVariableSightsFOVInfo.Cast(fovInfo);
		if (!variableFovInfo)
			return;

		if (direction > 0)
			variableFovInfo.SetNext(allowCycling);
		else
			variableFovInfo.SetPrevious(allowCycling);
	}

	//------------------------------------------------------------------------------------------------
	//! If a weapon with multiple sights is equipped, switch the next or previous sights on the weapon (if any)
	//! \param[in] direction If above zero, advances to next sights. If equal to or below zero, advances to previous sights.
	void SetNextSights(int direction = 1)
	{
		// Check if we are in a turret and switch the turret's optics instead
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetOwner());
		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (compartmentAccess)
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (compartment)
			{
				TurretControllerComponent turretController = TurretControllerComponent.Cast(compartment.GetController());
				if (turretController)
				{
					TurretComponent turretComponent = turretController.GetTurretComponent();
					if (turretComponent)
					{
						if (direction > 0)
							turretComponent.SwitchNextSights();
						else 
							turretComponent.SwitchPrevSights();
					
						return;
					}
				}
			}
		}
		
		BaseWeaponManagerComponent weaponManager = GetWeaponManagerComponent();
		if (!weaponManager)
			return;

		BaseWeaponComponent weaponComponent = weaponManager.GetCurrentWeapon();
		if (!weaponComponent)
			return;

		if (direction > 0)
			weaponComponent.SwitchNextSights();
		else
			weaponComponent.SwitchPrevSights();
		
		if (m_CharacterSoundComponent)
			m_CharacterSoundComponent.SoundEvent(SCR_SoundEvent.SOUND_CHAR_MOVEMENT_WEAPON_SIGHT_TOGGLE);
	}

	//------------------------------------------------------------------------------------------------
	//! \return currently used SightsFOVInfo if any, null otherwise.
	SightsFOVInfo GetSightsFOVInfo()
	{
		BaseWeaponManagerComponent weaponManager = GetWeaponManagerComponent();
		if (!weaponManager)
			return null;

		BaseWeaponComponent weaponComponent = weaponManager.GetCurrentWeapon();
		if (!weaponComponent)
			return null;

		BaseSightsComponent sightsComponent = weaponComponent.GetSights();
		if (!sightsComponent)
			return null;

		return sightsComponent.GetFOVInfo();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnControlledByPlayer(IEntity owner, bool controlled)
	{
		// Do initialisation/deinitialisation of character that was given/lost control by plyer here
		if (controlled && owner == SCR_PlayerController.GetLocalControlledEntity())
		{
			GetGame().GetInputManager().AddActionListener("CharacterNextWeapon", EActionTrigger.DOWN, ActionNextWeapon);
			GetGame().GetInputManager().AddActionListener("CharacterUnequipItem", EActionTrigger.DOWN, ActionUnequipItem);
			GetGame().GetInputManager().AddActionListener("CharacterDropItem", EActionTrigger.DOWN, ActionDropItem);
			GetGame().GetInputManager().AddActionListener("CharacterWeaponLowReady", EActionTrigger.DOWN, ActionWeaponLowReady);
			GetGame().GetInputManager().AddActionListener("CharacterWeaponRaised", EActionTrigger.DOWN, ActionWeaponRaised);
			GetGame().GetInputManager().AddActionListener("CharacterWeaponBipod", EActionTrigger.DOWN, ActionWeaponBipod);

			// TODO: This should be handled by camera handler itself
			if (m_CameraHandler)
				GetGame().GetInputManager().AddActionListener("SwitchCameraType", EActionTrigger.DOWN, m_CameraHandler.OnCameraSwitchPressed);
		}
		else
		{
			GetGame().GetInputManager().RemoveActionListener("CharacterNextWeapon", EActionTrigger.DOWN, ActionNextWeapon);
			GetGame().GetInputManager().RemoveActionListener("CharacterUnequipItem", EActionTrigger.DOWN, ActionUnequipItem);
			GetGame().GetInputManager().RemoveActionListener("CharacterDropItem", EActionTrigger.DOWN, ActionDropItem);
			GetGame().GetInputManager().RemoveActionListener("CharacterWeaponLowReady", EActionTrigger.DOWN, ActionWeaponLowReady);
			GetGame().GetInputManager().RemoveActionListener("CharacterWeaponRaised", EActionTrigger.DOWN, ActionWeaponRaised);
			GetGame().GetInputManager().RemoveActionListener("CharacterWeaponBipod", EActionTrigger.DOWN, ActionWeaponBipod);

			// TODO: This should be handled by camera handler itself
			if (m_CameraHandler)
				GetGame().GetInputManager().RemoveActionListener("SwitchCameraType", EActionTrigger.DOWN, m_CameraHandler.OnCameraSwitchPressed);
		}

		m_OnControlledByPlayer.Invoke(owner, controlled);

		// diiferentiate the inventory setup for player and AI
		SCR_CharacterInventoryStorageComponent pCharInvComponent = SCR_CharacterInventoryStorageComponent.Cast(owner.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (pCharInvComponent)
			pCharInvComponent.InitAsPlayer(owner, controlled);
	}

	//------------------------------------------------------------------------------------------------
	//! Alternate between primary weapons
	//! \param[in] value
	//! \param[in] trigger
	void ActionNextWeapon(float value = 0.0, EActionTrigger trigger = 0)
	{
		CharacterInputContext inputContext = GetInputContext();
		if (inputContext && inputContext.IsThrowCanceled())
		{
			inputContext.SetThrow(false);
			return;
		}

		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(GetInventoryStorageManager());
		if (!storageManager)
			return;

		SCR_CharacterInventoryStorageComponent storage = storageManager.GetCharacterStorage();
		if (storage)
			storage.SelectNextWeapon();
	}

	//------------------------------------------------------------------------------------------------
	//! Put current item away to inventory/back
	//! \param[in] value
	//! \param[in] trigger
	void ActionUnequipItem(float value = 0.0, EActionTrigger trigger = 0)
	{
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(GetInventoryStorageManager());
		if (!storageManager)
			return;

		SCR_CharacterInventoryStorageComponent storage = storageManager.GetCharacterStorage();
		if (storage)
			storage.UnequipCurrentItem();
	}

	//------------------------------------------------------------------------------------------------
	//! Drop current item on ground
	//! \param[in] value
	//! \param[in] trigger
	void ActionDropItem(float value = 0.0, EActionTrigger trigger = 0)
	{
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(GetInventoryStorageManager());
		if (!storageManager)
			return;

		SCR_CharacterInventoryStorageComponent storage = storageManager.GetCharacterStorage();
		if (storage)
			storage.DropCurrentItem();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] value
	//! \param[in] trigger
	void ActionWeaponLowReady(float value = 0.0, EActionTrigger trigger = 0)
	{
		if (GetIsWeaponDeployed())
			return;

		if (GetIsWeaponDeployedBipod())
			return;

		if (CanPartialLower() && !IsPartiallyLowered())
			SetPartialLower(true);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] value
	//! \param[in] trigger
	void ActionWeaponRaised(float value = 0.0, EActionTrigger trigger = 0)
	{
		if (!IsWeaponRaised())
			SetWeaponRaised(true);

		if (IsPartiallyLowered())
			SetPartialLower(false);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] value
	//! \param[in] trigger
	void ActionWeaponBipod(float value = 0.0, EActionTrigger trigger = 0)
	{
		BaseWeaponManagerComponent weaponManager = GetWeaponManagerComponent();
		if (!weaponManager)
			return;

		BaseWeaponComponent weapon = weaponManager.GetCurrentWeapon();
		if (!weapon || !weapon.HasBipod())
			return;

		if (GetIsWeaponDeployed() || GetIsWeaponDeployedBipod())
			StopDeployment();

		weapon.SetBipod(!weapon.GetBipod());
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnInspectionModeChanged(bool newState)
	{
		m_bInspectionFocus = newState;
	}

	protected int m_iTargetContext;

	//------------------------------------------------------------------------------------------------
	protected override float GetInspectTargetLookAt(out vector targetAngles)
	{
		#ifdef ENABLE_DIAG
		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_ALLOW_INSPECTION_LOOKAT))
			return 0;
		#else
			return 0; // Disabled for now
		#endif

		// Just for testing
		if (Debug.KeyState(KeyCode.KC_ADD))
		{
			++m_iTargetContext;
			Debug.ClearKey(KeyCode.KC_ADD);
			m_bInspectionFocus = true;
		}

		if (Debug.KeyState(KeyCode.KC_SUBTRACT))
		{
			--m_iTargetContext;
			Debug.ClearKey(KeyCode.KC_SUBTRACT);
			m_bInspectionFocus = true;
		}

		if (!m_bInspectionFocus)
			return 0;

		vector aimChange = GetInputContext().GetAimChange();
		const float threshold = 0.001;
		if (Math.AbsFloat(aimChange[0]) > threshold || Math.AbsFloat(aimChange[1]) > threshold)
		{
			m_bInspectionFocus = false;
			return 0;
		} // not now

		IEntity ent = GetInspectEntity();
		if (!ent)
			return 0;

		SCR_InteractionHandlerComponent handler = SCR_InteractionHandlerComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_InteractionHandlerComponent));
		if (!handler)
			return 0;

		ChimeraCharacter character = GetCharacter();

		// Update target
		array<UserActionContext> contexts = {};
		vector tmp;
		array<IEntity> ents = handler.GetManualOverrideList(null, tmp);
		ActionsManagerComponent ac;
		array<UserActionContext> buff;
		UserActionContext ctx;
		array<BaseUserAction> actions;
		foreach (IEntity e : ents)
		{
			ac = ActionsManagerComponent.Cast(e.FindComponent(ActionsManagerComponent));
			if (!ac)
				continue;

			buff = {};
			int bc = ac.GetContextList(buff);
			for (int i = 0; i < bc; ++i)
			{
				ctx = buff[i];
				if (ctx.GetActionsCount() == 0)
					continue;

				actions = {};
				int actionsCount = ctx.GetActionsList(actions);
				foreach (BaseUserAction action : actions)
				{
					if (action.CanBeShown(character))
					{
						contexts.Insert(ctx);
						break;
					}
				}
			}
		}

		int nonEmptyCount = contexts.Count();
		m_iTargetContext = Math.Repeat(m_iTargetContext, nonEmptyCount);

		if (nonEmptyCount == 0)
			return 0.0;

		ctx = contexts[m_iTargetContext];
		vector contextPos = ctx.GetOrigin();

		vector localAimDirection = GetHeadAimingComponent().GetAimingDirection();
		vector targetDirection = contextPos - character.EyePosition();
		vector localTargetDirection = character.VectorToLocal( targetDirection );
		localTargetDirection.Normalize();

		float lookYaw = Math.Sin(localTargetDirection[0]) * Math.RAD2DEG;
		float lookPitch = Math.Sin(localTargetDirection[1]) * Math.RAD2DEG;

		//Shape shape = Shape.CreateSphere(COLOR_RED, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER, contextPos, 0.01);

		lookPitch -= GetAimingComponent().GetAimingDirection()[1];
		targetAngles = Vector(lookYaw, lookPitch, 0) * Math.DEG2RAD;

		return 4.30 * Math.RAD2DEG ; // speed, approx deg/s
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ScriptedCharacterInputContext GetScrInputContext()
	{
		return m_pScrInputContext;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] loiteringType
	//! \param[in] holsterWeapon
	//! \param[in] allowRootMotion
	//! \param[in] alignToPosition
	//! \param[in] targetPosition
	//! \param[in] disableInput - If true, player cannot interrupt the loiter by pressing space. It is the responsibility of the caller to ensure that the action will be finished. If false, action can be cancelled by player input.
	//! \param[in] customAnimData - data for playing custom animation in graph attachment
	void StartLoitering(int loiteringType, bool holsterWeapon, bool allowRootMotion, bool alignToPosition, vector targetPosition[4] = { "1 0 0", "0 1 0", "0 0 1", "0 0 0" }, bool disableInput = false,
		SCR_LoiterCustomAnimData customAnimData = SCR_LoiterCustomAnimData.Default)
	{
		if (loiteringType == ELoiteringType.NONE)
		{
			Print("SCR_CharacterControllerComponent::StartLoitering is called with loiteringType=0.", LogLevel.WARNING);
			return;
		}

		ChimeraCharacter character = GetCharacter();
		RplComponent rplComp = character.GetRplComponent();
		if (rplComp && !rplComp.IsOwner())
			return;
		
		SCR_CharacterCommandHandlerComponent scrCmdHandler = SCR_CharacterCommandHandlerComponent.Cast(character.GetAnimationComponent().GetCommandHandler());
		if (!scrCmdHandler)
			return;

		if (!scrCmdHandler.GetCommandMove())
			return;
		
		m_pScrInputContext.m_iLoiteringType = loiteringType;
		m_pScrInputContext.m_iLoiteringShouldHolsterWeapon = holsterWeapon;
		m_pScrInputContext.m_bLoiteringShouldAlignCharacter = alignToPosition;
		m_pScrInputContext.m_bLoiteringDisablePlayerInput = disableInput;
		m_pScrInputContext.m_mLoiteringPosition = targetPosition;
		m_pScrInputContext.m_bLoiteringRootMotion = allowRootMotion;
		m_pScrInputContext.m_CustomAnimData = customAnimData;
		
		if (IsGadgetInHands())
			RemoveGadgetFromHand(true);
		
		if (alignToPosition)
			AlignToPositionFromCurrentPosition(targetPosition);
		
		TryStartLoiteringInternal();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool AlignToPositionFromCurrentPosition(vector targetPosition[4], float toleranceXZ = 0.01, float toleranceY = 0.01)
	{
		ChimeraCharacter character = GetCharacter();
		RplComponent rplComponent = character.GetRplComponent();
		if (rplComponent && !rplComponent.IsOwner())
			return false;
		
		vector currentTransform[4];
		character.GetWorldTransform(currentTransform);
		CharacterHeadingAnimComponent headingComponent = GetAnimationComponent().GetHeadingComponent();
		if (headingComponent)
			headingComponent.AlignPosDirWS(currentTransform[3], currentTransform[2], targetPosition[3], targetPosition[2]); 

		return vector.DistanceSqXZ(currentTransform[3], targetPosition[3]) < (toleranceXZ * toleranceXZ)
			&& Math.AbsFloat(currentTransform[3][2] - targetPosition[3][2]) < toleranceY;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_StartLoitering_S(int loiteringType, bool holsterWeapon, bool allowRootMotion, bool alignToPosition, vector targetPosition[4], SCR_LoiterCustomAnimData customAnimData)
	{
		m_pScrInputContext.m_iLoiteringType = loiteringType;
		m_pScrInputContext.m_iLoiteringShouldHolsterWeapon = holsterWeapon;
		m_pScrInputContext.m_bLoiteringShouldAlignCharacter = alignToPosition;
		m_pScrInputContext.m_mLoiteringPosition = targetPosition;
		m_pScrInputContext.m_bLoiteringRootMotion = allowRootMotion;
		m_pScrInputContext.m_CustomAnimData = customAnimData;
		
		SCR_CharacterCommandHandlerComponent scrCmdHandler = SCR_CharacterCommandHandlerComponent.Cast(GetCharacter().GetAnimationComponent().GetCommandHandler());
		scrCmdHandler.StartCommandLoitering(customAnimData);

		Rpc(Rpc_StartLoitering_BCNO,
				m_pScrInputContext.m_iLoiteringType,
				m_pScrInputContext.m_iLoiteringShouldHolsterWeapon,
				m_pScrInputContext.m_bLoiteringRootMotion,
				m_pScrInputContext.m_bLoiteringShouldAlignCharacter,
				m_pScrInputContext.m_mLoiteringPosition,
				m_pScrInputContext.m_CustomAnimData);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast, RplCondition.NoOwner)]
	protected void Rpc_StartLoitering_BCNO(int loiteringType, bool holsterWeapon, bool allowRootMotion, bool alignToPosition, vector targetPosition[4],	SCR_LoiterCustomAnimData customAnimData)
	{
		m_pScrInputContext.m_iLoiteringType = loiteringType;
		m_pScrInputContext.m_iLoiteringShouldHolsterWeapon = holsterWeapon;
		m_pScrInputContext.m_bLoiteringShouldAlignCharacter = alignToPosition;
		m_pScrInputContext.m_mLoiteringPosition = targetPosition;
		m_pScrInputContext.m_bLoiteringRootMotion = allowRootMotion;
		m_pScrInputContext.m_CustomAnimData = customAnimData;
		
		SCR_CharacterCommandHandlerComponent scrCmdHandler = SCR_CharacterCommandHandlerComponent.Cast(GetCharacter().GetAnimationComponent().GetCommandHandler());
		scrCmdHandler.StartCommandLoitering(customAnimData);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool TryStartLoiteringInternal()
	{
		if (IsChangingItem())
			return false;
		
		if (GetScrInputContext().m_iLoiteringShouldHolsterWeapon && GetCurrentItemInHands() != null)
		{
			TryEquipRightHandItem(null, EEquipItemType.EEquipTypeUnarmedContextual);
			return false;
		}
		
		if(m_pScrInputContext.m_bLoiteringShouldAlignCharacter)
		{
			CharacterHeadingAnimComponent headingComponent = GetAnimationComponent().GetHeadingComponent();
			if (headingComponent && headingComponent.IsAligning())
			{
				if (AlignToPositionFromCurrentPosition(GetScrInputContext().m_mLoiteringPosition, 0.02, 0.3)) // distance smaller than 2cm
					return false;
			}
		}

		ChimeraCharacter character = GetCharacter();
		SCR_CharacterCommandHandlerComponent scrCmdHandler = SCR_CharacterCommandHandlerComponent.Cast(character.GetAnimationComponent().GetCommandHandler());
		scrCmdHandler.StartCommandLoitering(m_pScrInputContext.m_CustomAnimData);
		
		RplComponent rplComponent = character.GetRplComponent();
		if (rplComponent && rplComponent.IsProxy())
			Rpc(Rpc_StartLoitering_S,
				m_pScrInputContext.m_iLoiteringType,
				m_pScrInputContext.m_iLoiteringShouldHolsterWeapon,
				m_pScrInputContext.m_bLoiteringRootMotion,
				m_pScrInputContext.m_bLoiteringShouldAlignCharacter,
				m_pScrInputContext.m_mLoiteringPosition,
				m_pScrInputContext.m_CustomAnimData);
		else
			Rpc(Rpc_StartLoitering_BCNO,
				m_pScrInputContext.m_iLoiteringType,
				m_pScrInputContext.m_iLoiteringShouldHolsterWeapon,
				m_pScrInputContext.m_bLoiteringRootMotion,
				m_pScrInputContext.m_bLoiteringShouldAlignCharacter,
				m_pScrInputContext.m_mLoiteringPosition,
				m_pScrInputContext.m_CustomAnimData);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] terminateFast should be true when going into alerted or combat state.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RPC_StopLoitering_S(bool terminateFast)
	{
		SCR_CharacterCommandHandlerComponent scrCmdHandler = SCR_CharacterCommandHandlerComponent.Cast(GetAnimationComponent().GetCommandHandler());
		scrCmdHandler.StopLoitering(terminateFast);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] terminateFast
	//terminateFast should be true when we are going into alerted or combat state.
	void StopLoitering(bool terminateFast)
	{
		ChimeraCharacter character = GetCharacter();
		RplComponent rplComponent = character.GetRplComponent();
		if (rplComponent && !rplComponent.IsOwner())
			return;
		
		SCR_CharacterCommandHandlerComponent scrCmdHandler = SCR_CharacterCommandHandlerComponent.Cast(GetAnimationComponent().GetCommandHandler());
		scrCmdHandler.StopLoitering(terminateFast);
		
		if (rplComponent && rplComponent.IsProxy())
			Rpc(RPC_StopLoitering_S, terminateFast);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsLoitering()
	{
		SCR_CharacterCommandHandlerComponent handler = SCR_CharacterCommandHandlerComponent.Cast(GetAnimationComponent().GetCommandHandler());
		if (!handler)
			return false;
		else
			return handler.IsLoitering();		
	}
}

enum ECharacterGestures // TODO: SCR_
{
	NONE = 0,
	POINT_WITH_FINGER = 1,
	COMMAND_STOP = 2,
	COMMAND_FOLLOW = 3,
	COMMAND_MOVE = 4,
	COMMAND_GET_IN_THAT_VEHICLE = 5,
	COMMAND_GET_IN = 6,
	SALUTE = 15
}
