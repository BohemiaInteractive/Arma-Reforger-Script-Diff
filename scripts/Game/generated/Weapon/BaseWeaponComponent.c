/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class BaseWeaponComponent: GameComponent
{
	proto external UIInfo GetUIInfo();
	proto external string GetWeaponSlotType();
	proto external EWeaponType GetWeaponType();
	proto external int GetMuzzlesList(out notnull array<BaseMuzzleComponent> outMuzzles);
	proto external ECanBeEquippedResult CanBeEquipped(CharacterControllerComponent charController);
	proto external BaseMuzzleComponent GetCurrentMuzzle();
	//! Returns whether current weapon is able to be reloaded or not.
	proto external bool IsReloadPossible();
	proto external bool IsChamberingNecessary();
	proto external bool IsChamberingPossible();
	// Magazines
	proto external BaseMagazineComponent GetCurrentMagazine();
	proto external string GetCurrentFireModeName();
	proto external EWeaponFiremodeType GetCurrentFireModeType();
	proto external float GetCurrentSightsZeroing();
	proto external bool GetCurrentSightsZeroingTransform(out vector outMat[4]);
	proto external void SightADSActivated();
	proto external void SightADSDeactivated();
	proto external bool IsSightADSActive();
	proto external BaseSightsComponent GetSights();
	proto external BaseSightsComponent GetAttachedSights();
	proto external int GetAttachments(out notnull array<AttachmentSlotComponent> outArray);
	proto external BaseSightsComponent GetSightsAt(int sightsIndex);
	proto external bool HasSightsAt(int sightsIndex);
	proto external bool CanSetSights(int sightsIndex);
	proto external bool CanSwitchToSights(int sightsIndex);
	proto external int FindAvailableSights();
	proto external bool SetSights(int sightsIndex);
	proto external bool SwitchNextSights();
	proto external bool SwitchPrevSights();
	proto external bool CanSwitchSights();
	proto external IEntity GetOwner();
	proto external float GetInitialProjectileSpeed();
	/*!
	Returns true if weapon has a bipod available.
	*/
	proto external bool HasBipod();
	/*!
	Returns true if weapon has bipod deployed.
	*/
	proto external bool GetBipod();
	/*!
	Changes state of bipod to deployed/undeployed depending on parameter.
	*/
	proto external void SetBipod(bool deploy);
	//! Set deployment gamepad-trigger-effect enable/disable.
	proto external void SetDeploymentTriggerEffectEnabled(bool val);

	// callbacks

	event void OnWeaponActive();
	event void OnWeaponInactive();
}

/*!
\}
*/
