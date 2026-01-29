/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

class CharacterCommandHandlerComponent: BaseCommandHandlerComponent
{
	//-----------------------------------------------------------------------------
	proto external IEntity GetCharacter();
	//! Returns the current character controller component.
	proto external CharacterControllerComponent GetControllerComponent();
	//! gets some basic info about movement
	proto external void GetMovementState(out notnull CharacterMovementState pMovementState);
	proto external void AlignNewTurns();
	proto external bool IsVehicleSwitchingSeats();
	proto external bool IsTurnedOut();
	proto external bool IsWeaponADSAllowed(bool allowSprint);
	proto external bool IsSprintingAllowed();
	proto external bool IsItemInspectionAllowed();
	proto external bool IsWeaponInspectionAllowed();
	proto external bool IsWeaponDeploymentAllowed();
	proto external bool IsThrowingAction();
	proto external bool IsProneStanceTransition();
	proto external bool IsUsingItem();
	proto external bool IsItemActionLoopTag();
	// Cancel use immediately (no out animation), hide gadget.
	proto external void CancelItemUse();
	// Finish use (play out animation), keep gadget if possible.
	proto external void FinishItemUse(bool finishAnimations);
	proto external bool WasMovement();
	proto external bool WasRotation();
	proto external CharacterCommandFall		StartCommand_Fall(float pYVelocity);
	proto external CharacterCommandLadder	StartCommand_Ladder(LadderComponent pLadder);
	proto external CharacterCommandVehicle	StartCommand_Vehicle(BaseCompartmentSlot pCompartment);
	//----------------------------------------------------------------------------
	proto external CharacterCommandMove		StartCommand_Move();
	proto external CharacterCommandClimb	StartCommand_Climb(CharacterCommandClimbResult pClimbResult, int pType);
	proto external CharacterCommandSwim		StartCommand_Swim();
	proto external CharacterCommandDamageFullBody StartCommand_DamageFullBody(float direction, int pType);
	proto external CharacterCommandSlide	StartCommand_Slide();
	//----------------------------------------------------------------------------
	proto external CharacterCommandDamage		AddCommandModifier_Damage(float direction, int pType);
	//----------------------------------------------------------------------------
	proto external void						DeleteCommandModifier_Damage(CharacterCommandDamage pDamage);
	proto external CharacterCommandMove		GetCommandMove();
	proto external CharacterCommandFall		GetCommandFall();
	proto external CharacterCommandClimb	GetCommandClimb();
	proto external CharacterCommandVehicle	GetCommandVehicle();
	proto external CharacterCommandLadder	GetCommandLadder();
	proto external CharacterCommandDeath	GetCommandModifier_Death();
	proto external CharacterCommandUnconscious	GetCommandModifier_Unconscious();
	proto external CharacterCommandMelee	GetCommandModifier_Melee();
	proto external CharacterCommandSwim		GetCommandSwim();
	proto external CharacterCommandSlide	GetCommandSlide();
	proto external CharacterCommandDamageFullBody GetCommandDamageFullBody();
	proto external CharacterCommandDamage	GetCommandModifier_Damage();
	proto external CharacterCommandWeapon	GetCommandModifier_Weapon();
	proto external CharacterCommandItemChange		GetCommandModifier_ItemChange();
	proto external CharacterCommandItemUse		GetCommandModifier_ItemUse();
	proto external CharacterCommandGadget	GetCommandModifier_Gadget();
	proto external CharacterCommandClimbSettings GetCommandClimbSettings();
	proto external CharacterCommandSwimSettings GetCommandSwimSettings();
	//! Returns the current cached ladder.
	proto external LadderComponent GetTargetLadder();
	//! Returns the current ladder. Use carefully, you should use GetTargetLadder when it's possible.
	proto external LadderComponent FindTargetLadder();
	proto external bool IsMovingInProne();
	//! default full body handlers
	proto external bool HandleFinishedCommandsDefault(bool pCurrentCommandFinished);
	proto external bool HandleDeathDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID, bool pCurrentCommandFinished);
	proto external bool HandleUnconsciousDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleVehicleDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleSwimmingDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleLaddersDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleClimbingDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleMeleeDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleFallingDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleDamageHitDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleSlideDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	//! default additive handlers
	proto external bool HandleWeaponsDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleThrowingDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleItemChangeDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleItemUseDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleItemGestureDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleWeaponObstructionDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleWeaponReloadingDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleWeaponADSDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleWeaponDeploymentDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleWeaponFireDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleLeftHandGadgetDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool HandleDynamicStanceDefault(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	proto external bool TransitionMove_JumpClimbDefault(CharacterInputContext pInputCtx);
	proto external bool TransitionMove_SwimmingDefault(CharacterInputContext pInputCtx);
	proto external bool TransitionMove_LadderDefault(CharacterInputContext pInputCtx);
	proto external void DropLiveGrenadeFromHand(bool throwWithForce);
	proto external void CancelThrowDefault();

	// callbacks

	/*
	----------------------------------------------------------------------------
	 full body handlers
	----------------------------------------------------------------------------
	*/
	event bool HandleFinishedCommands(bool pCurrentCommandFinished);
	event bool HandleDeath(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID, bool pCurrentCommandFinished);
	event bool HandleUnconscious(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleVehicle(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleSwimming(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleLadders(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleClimbing(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleMelee(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleFalling(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleDamageHit(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleSlide(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	/*
	----------------------------------------------------------------------------
	 additive handlers
	----------------------------------------------------------------------------
	*/
	event bool HandleWeapons(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleThrowing(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleItemChange(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleItemUse(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleItemGesture(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleWeaponReloading(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleWeaponADS(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleWeaponDeployment(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleWeaponFire(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleWeaponObstruction(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleLeftHandGadget(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool HandleDynamicStance(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	/*
	----------------------------------------------------------------------------
	transitions from move state
	----------------------------------------------------------------------------
	*/
	event bool TransitionMove_JumpClimb(CharacterInputContext pInputCtx);
	event bool TransitionMove_Swimming(CharacterInputContext pInputCtx);
	event bool TransitionMove_Ladder(CharacterInputContext pInputCtx);
	/*
	----------------------------------------------------------------------------
	callback for handling scripted commands from default commmand handler
	----------------------------------------------------------------------------
	*/
	event bool SubhandlerStatesBegin(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool SubhandlerStatesEnd(CharacterInputContext pInputCtx, float pDt, int pCurrentCommandID);
	event bool SubhandlerTransitionsMove(CharacterInputContext InputCtx);
	/*
	----------------------------------------------------------------------------
	additional callbacks
	----------------------------------------------------------------------------
	*/
	event int SelectMeleeAnimationType();
}

/*!
\}
*/
