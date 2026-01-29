/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class CharacterControllerComponentClass: PrimaryControllerComponentClass
{
}

class CharacterControllerComponent: PrimaryControllerComponent
{
	//!------------------------------------------------------------------------
	//! Aiming input sensitivities
	static proto void SetAimingSensitivity(float mouse, float gamepad, float ads);
	static proto void GetAimingSensitivity(out float mouse, out float gamepad, out float ads);
	//!------------------------------------------------------------------------
	//! FOV input curves
	static proto void SetFOVInputCurve(float mouse, float gamepad, float gyro, float trackIR);
	static proto void GetFOVInputCurve(out float mouse, out float gamepad, out float gyro, out float trackIR);
	//!------------------------------------------------------------------------
	//! Gyro input sensitivity yaw/pitch/roll
	static proto void SetGyroSensitivity(float yaw, float pitch, float roll);
	static proto void GetGyroSensitivity(out float yaw, out float pitch, out float roll);
	//!------------------------------------------------------------------------
	//! Gyro control scope
	static proto void SetGyroControl(bool always, bool freelook, bool ads);
	static proto void GetGyroControl(out bool always, out bool freelook, out bool ads);
	//!------------------------------------------------------------------------
	//! Sets gadget handling into persistent mode
	static proto void SetStickyGadget(bool enable);
	//!------------------------------------------------------------------------
	//! Sets ads and gadget focus into persistent mode
	static proto void SetStickyADS(bool enable);
	//!------------------------------------------------------------------------
	//! When enabled, controller freelook is not forced while piloting aircraft
	static proto void SetGamepadControlAircraft(bool enable);
	static proto bool GetGamepadControlAircraft();
	//!------------------------------------------------------------------------
	//! When enabled, mouse freelook is not forced while piloting aircraft
	static proto void SetMouseControlAircraft(bool enable);
	static proto bool GetMouseControlAircraft();
	//!------------------------------------------------------------------------
	//! Track-IR controls
	static proto void SetTrackIREnable(bool enable);
	static proto bool GetTrackIREnable();
	static proto void SetTrackIRLeaningEnable(bool enable);
	static proto bool GetTrackIRLeaningEnable();
	static proto void SetTrackIRFreelookEnable(bool enable);
	static proto bool GetTrackIRFreelookEnable();
	static proto void SetTrackIRWhileADSEnable(bool enable);
	static proto bool GetTrackIRWhileADSEnable();
	static proto void SetTrackIRLeaningMode(ETrackIRLeanMode enable);
	static proto ETrackIRLeanMode GetTrackIRLeaningMode();
	static proto void SetTrackIRFreelookSensitivity(float sensitivity);
	static proto float GetTrackIRFreelookSensitivity();
	static proto void SetTrackIRMoveLeaningSensitivity(float sensitivity);
	static proto float GetTrackIRMoveLeaningSensitivity();
	static proto void SetTrackIRRollLeaningSensitivity(float sensitivity);
	static proto float GetTrackIRRollLeaningSensitivity();
	static proto void SetTrackIRFreelookDeadzoneADS(float deadzone);
	static proto float GetTrackIRFreelookDeadzoneADS();
	static proto void SetTrackIRLeaningDeadzone(float deadzone);
	static proto float GetTrackIRLeaningDeadzone();
	static proto void SetTrackIRLeaningActiveYawRange(float range);
	static proto float GetTrackIRLeaningActiveYawRange();
	proto external CharacterAimingComponent GetAimingComponent();
	proto external CharacterHeadAimingComponent GetHeadAimingComponent();
	proto external CharacterAnimationComponent GetAnimationComponent();
	proto external BaseWeaponManagerComponent GetWeaponManagerComponent();
	proto external CameraHandlerComponent GetCameraHandlerComponent();
	proto external InventoryStorageManagerComponent GetInventoryStorageManager();
	proto external VoNComponent GetVONComponent();
	proto external AIControlComponent GetAIControlComponent();
	proto external EntitySlotInfo GetRightHandPointInfo();
	proto external EntitySlotInfo GetLeftHandPointInfo();
	proto external CharacterInputContext GetInputContext();
	proto external float GetMovementSpeed();
	//! Update animation about state of movement, define speed and direction in local space of character
	proto external void SetMovement(float movementSpeed, vector movementDirModel);
	proto external void SetHeadingAngle(float newHeadingAngle, bool adjustAimingYaw = false);
	//! Returns the current stance of the character.
	proto external ECharacterStance GetStance();
	//! Returns the current controlled character.
	proto external SCR_ChimeraCharacter GetCharacter();
	//! Set wanted input action values
	proto external void SetFireWeaponWanted(bool val);
	//! Set wanted input action values
	proto external void SetThrow(bool val, bool cancelThrow);
	//! Update simulation state with difference of world position
	proto external void SetMovementDirWorld(vector movementDirWorld);
	proto external vector GetMovementDirWorld();
	//! Set the current weapon-raised state.
	proto external void SetWeaponRaised(bool val);
	//! Set the current weapon ADS state.
	proto external void SetWeaponADS(bool val);
	proto external void SetFreeLook(bool input, bool mouse, bool controller);
	proto external bool GetFreeLookInput();
	proto external bool GetFreeLookMouse();
	proto external bool GetFreeLookController();
	//! Force character to stay in freelook
	proto external void SetForcedFreeLook(bool enabled);
	proto external bool IsFreeLookForced();
	proto external void ResetPersistentStates(bool resetADSState = true, bool resetGadgetState = true);
	/*!
	Sets dynamic speed of this character.
	\param value Desired speed as percentage <0,1>.
	*/
	proto external void SetDynamicSpeed(float value);
	/*!
	Returns dynamic speed value. Set by mousewheel or by pressing walk button.
	Not set by gamepad analogue stick.
	\return Dynamic speed value as <0, 1>.
	*/
	proto external float GetDynamicSpeed();
	proto external void SetShouldApplyDynamicSpeedOverride(bool shouldApply);
	/*!
	Sets dynamic stance of this character.
	\param value Desired stance height as percentage of full erect <0,1>.
	*/
	proto external void SetDynamicStance(float value);
	proto external float GetVehicleRagdollSpeed();
	/*
	Gets vehicle ragdoll speed in m/s, squared
	*/
	proto external float GetVehicleRagdollSpeedMsSq();
	/*!
	Returns whether provided dynamic stance can be set for this character.
	\see CharacterControllerComponent::SetDynamicStance(float value)
	\param value Desired stance height as percentage of full erect <0,1>.
	*/
	proto external bool CanSetDynamicStance(float value);
	/*!
	Returns current dynamic stance value.
	\return Dynamic stance value as <0,1>.
	*/
	proto external float GetDynamicStance();
	/*!
	Enables or disables inspection mode.
	\param targetItem Item to enable the inspection mode on.
	*/
	proto external void SetInspect(IEntity targetItem);
	/*!
	Returns whether inspection mode can be set.
	\return True in case inspection mode can be set, false otherwise.
	*/
	proto external bool CanInspect(IEntity targetItem);
	/*!
	Returns whether character is in inspection mode.
	\return True in case character is in inspection mode, false otherwise.
	*/
	proto external bool GetInspect();
	/*!
	Returns whether inspected item (if any) is currently equipped weapon or not.
	\return True in case inspected weapon is valid and is current weapon.
	*/
	proto external bool GetInspectCurrentWeapon();
	/*!
	Returns entity that is currently inspected or null if none.
	*/
	proto external IEntity GetInspectEntity();
	/*!
	Sets inspection state if inspection is enabled.
	\param state Target state, 0 = default, 1 = alternate.
	*/
	proto external void SetInspectState(int state);
	/*!
	Returns inspection state if inspection is enabled.
	\return Returns desired state, see SetInspectState.
	*/
	proto external int GetInspectState();
	/*!
	Returns true if weapon is deployed and stabilized.
	*/
	proto external bool GetIsWeaponDeployed();
	/*!
	Returns true if weapon can be deployed and stabilized.
	This method uses traces to see if the surface is suitable for weapon deployment.
	*/
	proto external bool CanDeployWeapon();
	/*!
	Returns true if weapon is deployed and stabilized using a bipod.
	*/
	proto external bool GetIsWeaponDeployedBipod();
	proto external IEntity GetCurrentItemInHands();
	//! CharacterStanceChange STANCECHANGE_NONE = 0, STANCECHANGE_TOERECTED = 1, STANCECHANGE_TOCROUCH = 2, STANCECHANGE_TOPRONE = 3
	proto external void SetStanceChange(int stance);
	//! CharacterStanceChange STANCECHANGE_NONE = 0, STANCECHANGE_TOERECTED = 1, STANCECHANGE_TOCROUCH = 2, STANCECHANGE_TOPRONE = 3
	proto external bool CanChangeStance(int stance);
	proto external void ForceStance(int stance);
	proto external void ForceStanceUp(int stance);
	//! 2 - right, 1 - left
	proto external void SetRoll(int val);
	proto external bool IsRoll();
	proto external bool CanRoll(int val);
	proto external bool ShouldHoldInputForRoll();
	proto external void EnableHoldInputForRoll(bool enable);
	proto external bool IsPlayerControlled();
	proto external void SetJump(float val);
	proto external void SetWantedLeaning(float val);
	proto external void SetBanking(float val);
	proto external void SetMeleeAttack(bool val);
	//! Either character wants to lean
	proto external float GetLeaning();
	proto external float GetWantedLeaning();
	proto external bool IsAdjustingLeaning();
	//! Returns current amount of leaning applied
	proto external float GetCurrentLeanAmount();
	proto external bool IsLeaning();
	proto external float GetADSTime();
	proto external bool IsWeaponRaised();
	proto external EWeaponObstructedState GetWeaponObstructedState();
	proto external float GetObstructionAlpha();
	proto external bool IsClimbing();
	proto external bool IsSwimming();
	proto external bool IsSprinting();
	proto external bool IsChangingStance();
	proto external bool IsWeaponADS();
	proto external bool IsChangingFireMode();
	proto external bool IsPlayingGesture();
	proto external bool IsFreeLookEnabled();
	proto external bool IsTrackIREnabled();
	proto external bool IsFocusMode();
	proto external bool IsOpeningVehicleDoor();
	proto external bool IsClosingVehicleDoor();
	proto external bool GetWeaponADSInput();
	proto external bool IsChangingItem();
	proto external bool IsFalling();
	proto external bool IsReloading();
	proto external bool CanFire();
	proto external bool IsDead();
	proto external bool IsUsingItem();
	proto external bool IsMeleeAttack();
	proto external bool CanEngageChangeItem();
	//! Set weapon on character with switching animations. If true, the request was successful
	proto external bool SelectWeapon(BaseWeaponComponent newWeapon);
	proto external bool SetMuzzle(int index);
	proto external bool SetFireMode(int index);
	proto external void SetSightsRange(int index);
	proto external void SetWeaponADSInput(bool val);
	/*!
	Set the safety of the current weapon.
	\param safety True to set the weapon safety.
	\param automatic True to set the automatic safety otherwise false to set manual safety.
	\return Returns true if the action has been successfull otherwise false.
	*/
	proto external bool SetSafety(bool safety, bool automatic);
	//Returns the current stamina value in <0, 1>. -1 if there is no stamina component attached to the current owner.
	proto external float GetStamina();
	//! Request weapon reload. If true, request was sucessful
	proto external bool ReloadWeapon();
	// mag or projectile
	proto external bool DetachCurrentMagazine();
	proto external bool ReloadWeaponWith(IEntity ammunitionEntity, bool bForceDetach = false);
	/*!
	Returns the current controller state.
	Differs from IsUnconscious which also considers the animation status.
	E.g.: When waking up, the GetLifeState will be ALIVE, But the IsUnconscious will return true for as long as the wake-up animation is playing.
	*/
	proto external ECharacterLifeState GetLifeState();
	proto external void SetUnconscious(bool enabled);
	proto external bool IsUnconscious();
	proto external bool CanReviveCharacter();
	// Returns true if there is nothing blocking starting unconscious - it does not check whether unconsciousness should actually start.
	proto external bool CanEnterUnconsciousness();
	proto external void EnableReviveCharacter(bool enabled);
	/*!
	Starts ragdoll on the character.
	This should be called only on owner.
	Replication is handled in gamecode.
	This ragdoll is handled the same way as the unconscious ragdoll, where it ends after some delay
		and when the bones stop moving and then its blended into animated pose(ideally prone one).
	During this ragdoll, the character stance is changed to prone.
	To better control ragdoll time, use RefreshRagdoll method.
	*/
	proto external void Ragdoll();
	/*!
	Refreshes ragdoll, so it doesn't get turned off for fWarmupTime seconds. This doesn't work if the character is already transitioning into animation from ragdoll.
	This should be called only on owner.
	This can also be used to shorten time to end ragdoll.
	If you want ragdoll to be "controlled" from script. You can call SCR_Ragdoll, then SCR_RefreshRagdoll(10000.0) to start "endless" ragdoll
		and then SCR_RefreshRagdoll(0.0) when you want the ragdoll to start to blend into animated pose.
	If you'll call this when the ragdoll hasn't started, it will be applied to the next ragdoll.
	*/
	proto external void RefreshRagdoll(float fWarmupTime);
	//! Kills the character. Skips invincibility checks.
	proto external void ForceDeath();
	/* @NOTE(Leo): Temp solution, eventually will be solved by setting respective gadget graph attachments,
	unfortunately current state of Enf animation system is not allowing it and will be solved not earlier then 10.07 by @Michal Žák
	from conversation with @Théo Escamez:
	so heres how it works :
	for now we have 4 items> compass adrianov, compass SY183, Radio ANPRC68 and Radio R148
	...where they are triggered respectively by integers 1 2 3 and 4...
	*/
	proto external void TakeGadgetInLeftHand(IEntity gadget, int gadgetType, bool autoFocus = false, bool skipAnimations = false);
	// Performs gadget equip validation
	proto external bool CanEquipGadget(IEntity gadget);
	proto external IEntity GetAttachedGadgetAtLeftHandSlot();
	/*!
	Try to use bandage on specified body part for a specific duration.
	\param bandage Bandage entity used for bandage action.
	\param bodyPart Body part where the bandage will be applied.
	\param duration Duration of the bandage in seconds.
	\param bandageSelf True if the player is bandaging himself.
	*/
	proto external bool TryUseBandage(IEntity bandage, int bodyPart, float duration = 4.0, bool bandageSelf = true);
	//! Remove held gadget
	proto external void RemoveGadgetFromHand(bool skipAnimations = false);
	//! Put held gadget on hold
	proto external void RecoverHiddenGadget(bool respectSettings, bool skipAnims);
	//! Returns true if there is a gadget in hands.
	proto external bool IsGadgetInHands();
	//! Returns true if focus will be changed to requested
	proto external bool SetGadgetRaisedModeWanted(bool newRaised);
	//! Returns true if character will be (or is) using gadget in raised mode
	proto external bool IsGadgetRaisedModeWanted();
	//! Generic item
	//! Equippes an item in right hand, if swap is true then action performed without animations, accepts optional callback that will be triggered when action is completed
	proto external bool TryEquipRightHandItem(IEntity item, EEquipItemType type, bool swap = false, BaseUserAction callbackAction = null);
	proto external bool TryRecoverLastRightHandItem(bool swap = false, BaseUserAction callbackAction = null);
	//! Returns generic item attached to right hand. Returns null if there's none (or if active item is weapon)
	proto external IEntity GetRightHandItem();
	/*!
	Try to use equipped item.
	\return Returns true if the equipped item has been used.
	*/
	proto external bool TryUseItem(IEntity item, bool allowMovementDuringAction = false, bool keepInHandAfterSuccessfulAction = false);
	/*!
	Try to use equipped item with custom command and variables.
	\param item - the item which we want to use. Must be either the current gadget, or the current weapon.
	\param allowMovementDuringAction - if true, any movement input will be disabled during the item use.
	\param cmdId - Id of the command to be called - use AnimationComponent.BindCommand(commandName) to receive it.
	\param cmdIntArg, cmdFloatArg - parameters with which the command will be called.
	\param intParam, floatParam, boolParam - Currently, BodyPart variable will be set to the intParam value, other two are yet not in use.
	\param animLength - maximum length of the animation - if the animation graph will finish sooner (TagRItemAction or TagLItemAction tags will not be active), the animation can end before this time is out.
	\param alignmentPoint - point of the item to which the ItemUsePrediction predictioned bone of the character will keep being aligned during the animation.
	\return Returns true if the equipped item has been used.
	*/
	proto external bool TryUseItemOverrideParams(notnull ItemUseParameters params);
	//! Returns true if the character can use an item.
	proto external bool CanUseItem();
	//! Starts character gesture with specified duration in milliseconds (if duration <= 0, it will be played until StopCharacterGesture is called)
	proto external bool TryStartCharacterGesture(int gesture, int durationMS = 0);
	proto external void StopCharacterGesture();
	/*!
	Try to play the specified gesture.
	\param gesture The gesture which should be played.
	\param callbackAction Optional UserAction as callback that will be called when target animation event is hit or gesture is complete
	\param confirmEvent
	\return Returns true if the gesture has been played.
	*/
	proto external bool TryPlayItemGesture(EItemGesture gesture, BaseUserAction callbackAction = null, string confirmEvent = "");
	//! Returns true if the character is playing a gesture.
	proto external bool IsPlayingItemGesture();
	//! Returns true if the character can play a gesture.
	proto external bool CanPlayItemGesture();
	//! Returns true if the character can use provided ladder.
	//! if optional maxTestDistance or maxEntryAngle is below 0, distance for test will be taken from character's ladder auto detection settings
	proto external bool CanUseLadder(IEntity pLadderOwner, int ladderComponentIndex = 0, float maxTestDistance = -1.0, float maxEntryAngle = -1.0, bool performTraceCheck = false);
	//! Start climbing provided ladder.
	//! Returns true if request was successful.
	proto external bool TryUseLadder(IEntity pLadderOwner, int ladderComponentIndex = 0, float maxTestDistance = -1.0, float maxEntryAngle = -1.0);
	//! Makes character drop weapon from the weapon slot given by parameter.
	proto external void DropWeapon(WeaponSlotComponent weaponSlot);
	//! Makes character drop item from left hand.
	proto external void DropItemFromLeftHand();
	proto external void StopDeployment();
	// Script
	proto external void RequestActionByID(int actionID, float value);
	//! Returns true if the character is partially lowered.
	proto external bool IsPartiallyLowered();
	//! Returns true if the character can partially lower (weapon).
	proto external bool CanPartialLower();
	//! Sets desired partial lower state, if allowed.
	proto external void SetPartialLower(bool state);
	//------------------------------------------------------------------------
	proto external vector GetCameraWeaponOffset();
	//------------------------------------------------------------------------
	proto external bool GetDisableMovementControls();
	//------------------------------------------------------------------------
	proto external void SetDisableMovementControls(bool other);
	//------------------------------------------------------------------------
	proto external bool GetDisableViewControls();
	//------------------------------------------------------------------------
	proto external void SetDisableViewControls(bool other);
	//------------------------------------------------------------------------
	proto external bool GetDisableWeaponControls();
	//------------------------------------------------------------------------
	proto external void SetDisableWeaponControls(bool other);
	//------------------------------------------------------------------------
	proto external vector GetMovementVelocity();
	//------------------------------------------------------------------------
	proto external vector GetMovementInput();
	//------------------------------------------------------------------------
	proto external vector GetVelocity();
	//------------------------------------------------------------------------
	proto external bool GetIsSprintingToggle();
	//------------------------------------------------------------------------
	proto external float GetLastStanceChangeDelay();
	//------------------------------------------------------------------------
	proto external int GetCurrentMovementPhase();
	//------------------------------------------------------------------------
	proto external bool GetCanFireWeapon();
	//------------------------------------------------------------------------
	proto external bool GetCanThrow();
	//------------------------------------------------------------------------
	proto external void SetWeaponNoFireTime(float t);
	//------------------------------------------------------------------------
	proto external bool IsInThirdPersonView();
	// Caching third person view since profiling showed minor impact on cpu time when calling in to scripted method
	proto external void SetInThirdPersonView(bool state);
	proto external float GetStanceChangeDelayTime();
	proto external float GetJumpSpeed();
	proto external bool GetMeleeAttackInput();
	//! Returns true if freelook is enforced by game logic.
	proto external bool IsFreeLookEnforced();
	//! Overrides max speed of the character to be a fraction (given by the parameter) of current max speed.
	//! If value < 0, it doesn't override it. if value > 1, it will use the normal max speed.
	proto external void OverrideMaxSpeed(float fraction);
	/*! Returns whether a position is in the character's view
	\param pos World Position to check is within view
	\param angMax Maximum(exclusive) angular offset in Degrees to consider the position within view
	*/
	proto external bool GetPositionInView(vector pos, float angMax);
	/*! Returns modifiers for weapon sway/recoil based on movement, stance, etc
	Fills scaleA and scaleB with following modifier values:
	scaleA.x - modifier scaleX
	scaleA.y - modifier scaleY
	scaleB.x - speed scale
	scaleB.y - translation scale
	*/
	[Obsolete("This method will be removed soon!")]
	proto external void GetWeaponModifiers(float baseScale, float moveScale, out vector scaleA, out vector scaleB);
	/*!
	Returns locally cached stamina component or null if none.
	*/
	proto external CharacterStaminaComponent GetStaminaComponent();
	// Check if character is not moving and not any other locomotion related action is being performed
	proto external bool IsCharacterStationary();

	// callbacks

	event void OnInspectionModeChanged(bool newState);
	event void UpdateDrowning(float timeSlice, vector waterLevel);
	event bool IsUsingBinoculars();
	/*!
	When this character gets hit, this function gets called.
	float damageValue: Raw damage of the hit (no damage multipliers have been applied). Can be negative for "healing" damage.
	EDamageType damageType:
	return EHitReactionType: Type of hit reaction that should be played for this impact.
	*/
	event EHitReactionType ComputeHitReaction(float damageValue, EDamageType damageType) {return EHitReactionType.HIT_REACTION_NONE;};
	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event protected void OnInit(IEntity owner);
	/*!
	Called during EOnDiag.
	\param owner Entity this component is attached to.
	\poaram timeSlice Delta time since last update.
	*/
	event protected void OnDiag(IEntity owner, float timeslice);
	event protected void OnReloaded(IEntity owner, BaseWeaponComponent weapon);
	event protected void OnPrepareControls(IEntity owner, ActionManager am, float dt, bool player);
	event protected void OnApplyControls(IEntity owner, float timeSlice);
	//! Handling of melee events. Sends true if melee started, false, when melee ends
	event protected void OnMeleeDamage(bool started);
	event bool GetCanMeleeAttack() { return true; };
	event bool GetCanEquipGadget(IEntity gadget) { return true; };
	//! Override to handle what happens after pressing F button, return false to use default cpp behavior
	event protected bool OnPerformAction() { return false; };
	//! Override to handle whether character can get out of vehicle via GetOut input action
	event bool CanGetOutVehicleScript() { return true; };
	//! Override to handle whether character can eject from vehicle via JumpOut input action
	event bool CanJumpOutVehicleScript() { return true; };
	//! Will be called when the life state of the character changes.
	event protected void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState, bool isJIP);
	//! Will be called when the consciousness of the character changes.
	event protected void OnConsciousnessChanged(bool conscious);
	//! Handling of death. If instigatorEntity is null, you can use instigator.GetInstigatorEntity() if appropiate.
	event protected void OnDeath(IEntity instigatorEntity, notnull Instigator instigator);
	//! Will be called when gadget taken/removed from hand
	event protected void OnGadgetStateChanged(IEntity gadget, bool isInHand, bool isOnGround);
	//! Will be called when gadget fully transitioned to or canceled focus mode
	event protected void OnGadgetFocusStateChanged(IEntity gadget, bool isFocused);
	//! Will be called when item use action is started
	event protected void OnItemUseBegan(ItemUseParameters itemUseParams);
	//! Will be called when item use action is complete
	event protected void OnItemUseEnded(ItemUseParameters itemUseParams, bool successful);
	//! Will be called when weapon raising started
	event protected void OnWeaponRaisingStarted();
	//! Will be called when weapon raising finished
	event protected void OnWeaponRaisingFinished();
	//! Will be called when weapon lowering started
	event protected void OnWeaponLoweringStarted();
	//! Will be called when weapon lowering finished
	event protected void OnWeaponLoweringFinished();
	event protected void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd);
	/*!
	Output target angles vector is used during inspection to adjust look at.
	Returned value is speed at which the look angles are adjusted,
	or 0 if no adjustment is to be made.
	*/
	event float GetInspectTargetLookAt(out vector targetAngles);
	//! This is called when a climb or jump is requested by input.
	//! If it returns false, the character will not attempt to jump/climb.
	event bool CanJumpClimb();
	/*Should return true if during CharacterHeadingAnimComponent aligning, the aiming angles should influence aiming angles.*/
	event bool ShouldAligningAdjustAimingAngles();
	event bool ShouldGadgetBeDropped(IEntity gadget);
	//! Runs after a weapon is dropped from hands. Returns dropped weapon entity and slot that the weapon was dropped from.
	event protected void OnWeaponDropped(IEntity pWeaponEntity, WeaponSlotComponent pWeaponSlot);
	//! Runs after the left hand item is dropped. Returns dropped item entity.
	event protected void OnItemDroppedFromLeftHand(IEntity pItemEntity);
	//------------------------------------------------------------------------
	event bool SCR_GetDisableMovementControls();
	//------------------------------------------------------------------------
	event void SCR_OnDisabledJumpAction();
	//------------------------------------------------------------------------
	event bool SCR_GetDisableViewControls();
	//! Called when a player has been assigned to this controller
	event protected void OnControlledByPlayer(IEntity owner, bool controlled);
}

/*!
\}
*/
