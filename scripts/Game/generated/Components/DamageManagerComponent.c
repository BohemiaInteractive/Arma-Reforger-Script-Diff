/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class DamageManagerComponent: HitZoneContainerComponent
{
	/*!
	Clears and fills struckHitZones array with all HZs that should get damaged by this BaseDamageContext.
	This function uses BaseDamageContext.colliderID to find the struck hitzones.
	If no hitzones are attached to the colliderID, it will return default hitzone.
	Use this to call HandleDamage individually on each of the affected hitzones.
	Dont call this if:
	-The DamageManager has been destroyed, struckHitZones will be empty.
	-DamageContext.struckHitZone is known. It will only return that hitzone.
	\param struckHitZones: array of hitzones that were struck
	\param damageContext: Information about the damage. Only BaseDamageContext.struckHitZone and BaseDamageContext.colliderID are relevant.
	\return Returns the count of elements that were filled into the array.
	*/
	proto external int ComputeStruckHitZones(out notnull array<HitZone> struckHitZones, notnull BaseDamageContext damageContext);
	//Enables damage handling. It will only be done if called from server.
	proto external void EnableDamageHandling(bool enable);
	//Returns true if this damage manager and its hitzones can take damage
	proto external bool IsDamageHandlingEnabled();
	//! Call to enable damage system's on frame
	proto external void EnableDamageSystemOnFrame();
	//! Call to disable damage system's on frame
	proto external void DisableDamageSystemOnFrame();
	//! Sets the movement damage modifier
	//! \warning Only usable by ChimeraCharacters and Vehicles.
	proto external void SetMovementDamage(float damage);
	//! Returns the movement damage modifier
	//! \warning Only usable by ChimeraCharacters and Vehicles.
	proto external float GetMovementDamage();
	//! Sets the aiming damage modifier.
	//! \warning Only usable by ChimeraCharacters.
	proto external void SetAimingDamage(float damage);
	//! Returns aiming damage modifier.
	//! \warning Only usable by ChimeraCharacters.
	proto external float GetAimingDamage();
	//Get default hitzone's health [0...1]
	proto external float GetHealthScaled();
	//sets the health scaled to the default HitZone [0, ..., 1]
	proto external bool SetHealthScaled(float health);
	//Returns current amount of health on the default hitzone
	proto external float GetHealth();
	//Returns default hitzone's max health
	proto external float GetMaxHealth();
	// Returns default hitzone's DamageState
	proto external EDamageState GetState();
	//returns true if the default hitzone is in Destroyed damage state
	proto external bool IsDestroyed();
	//Sets the given entity as the last instigator of damage.
	proto external void SetInstigatorEntity(IEntity instigator);
	proto external void SetInstigator(notnull Instigator instigator);
	//! Sets the instigator and broadcasts the change to clients
	//! DO NOT USE THIS METHOD UNLESS YOU KNOW EXACTLY WHEN TO USE IT
	//! IF YOU ARE USING THIS YOU PROBABLY SHOULD BE DEALING DAMAGE TO THE PLAYER TO KILL THEM.
	//! This method is currently only used to set the instigator of people neutralized through GM,
	//! because GM uses SetHealth(0) instead of dealing damage. In this REALLY SPECIFIC case it makes sense,
	//! because the player might have damage handling disabled and GM SHOULD be able to kill invulnerable people.
	//! This will completely bypass ShouldOverrideInstigator, method won't even be called.
	proto external void SetAndReplicateInstigator(notnull Instigator instigator);
	//Returns last instigator
	proto external notnull Instigator GetInstigator();
	//! Fills colliderIDs with all the colliders attached to hitzones this dmg manager owns
	proto external int GetAttachedColliderIDs(out notnull array<int> outAttachedColliderIDs);
	proto external void HandleDamage(notnull BaseDamageContext damageContext);
	/*!
	\return true if there is active DOT of specified type.
	*/
	proto external bool IsDamagedOverTime(EDamageType dType);
	/*!
	\return total DPS inflicted by all DoTs of specified type.
	*/
	proto external float GetDamageOverTime(EDamageType dType);
	/*!
	Removes DOT of provided damageType from all currently affected HitZones.
	*/
	proto external void RemoveDamageOverTime(EDamageType dType);

	// callbacks

	/*!
	Use this to run diag for the character who is currently possessed by player controller
	\param owner: owner entity
	*/
	event protected void DiagOnlyIfPossessedByPlayerController(notnull IEntity owner);
	/*! Called when fall related damage is relevant */
	event protected void OnHandleFallDamage(EFallDamageType fallDamageType, vector velocityVector);
	//! Called when SetHealth() is used on default hitzone
	event protected void OnHealthSet();
	//! Called when SetMaxHealth() is used on default hitzone
	event protected void OnMaxHealthChanged();
	/*!
	Called whenever an instigator is going to be set.
	\param currentInstigator: This damage manager's last instigator
	\param newInstigator: The new instigator for this damage manager
	\return If it returns true, newInstigator will become the new current instigator for the damage manager and it will receive kill credit.
	*/
	event bool ShouldOverrideInstigator(notnull Instigator currentInstigator, notnull Instigator newInstigator)  { return true; };
	//! Not all armors are physical so the surface that gets struck by projectiles will not be the one the armor, but the hitzone.
	//! This is a workaround to solve that issue. It gets called every time a projectile strikes a hitzone
	//! We can override material of the hit with the corresponding one from the armor (if needed)
	event GameMaterial OverrideHitMaterial(HitZone struckHitzone);
	/*!
	Invoked every time the DoT is added to certain hitzone.
	*/
	event protected void OnDamageOverTimeAdded(EDamageType dType, float dps, HitZone hz);
	/*!
	Invoked when provided damage type is removed from certain hitzone.
	*/
	event protected void OnDamageOverTimeRemoved(EDamageType dType, HitZone hz);
	/*!
	Called when the damagestate changes.
	*/
	event protected void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP);
	/*!
	Called after all components are initialized.
	\param owner Entity this component is attached to.
	*/
	event protected void OnPostInit(IEntity owner);
	event protected void OnDelete(IEntity owner);
	/*!
	Called during EOnFrame.
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event protected void OnFrame(IEntity owner, float timeSlice);
	/*!
	Event when physics engine registered contact with other RigidBody
	\param owner The owner entity
	\param other Other Entity who contacted us
	\param contact Structure describing the contact
	\return If it returns false, contact gets discarded and DamageManagerComponent::OnFilteredContact will not be called
	*/
	event bool FilterContact(IEntity owner, IEntity other, Contact contact);
	/*!
	Only called if FilterContact returned true
	\param owner The owner entity
	\param other Other Entity who contacted us
	\param contact Structure describing the contact
	*/
	event protected void OnFilteredContact(IEntity owner, IEntity other, Contact contact);
	/*!
	Called during EOnDiag.
	\param owner Entity this component is attached to.
	\param timeSlice Delta time since last update.
	*/
	event protected void OnDiag(IEntity owner, float timeSlice);
	event protected void OnDamage(notnull BaseDamageContext damageContext);
	/*!
	Called when this DamageManager is about to handle damage.
	Any modifications done to the damageContext will persist for the rest of the damage handling process
	return false if damage handling should proceed with the changes done to the DamageContext.
	return true if damage should be discarded / was fully hijacked and should no longer be applied on this damage manager
	(e.g.: damage was passed to another dmg manager, so we dont handle damage on this manager ).
	*/
	event bool HijackDamageHandling(notnull BaseDamageContext damageContext) { return false; };
	/*!
	Called after HijackDamageHandling.
	If it returns true, damage will be dealt.
	If it returns false, damage will not be handled.
	Use this to introduce randomness on hit chances (e.g.: moving helicopter rotors)
	*/
	event bool ShouldCountAsHit(notnull BaseDamageContext damageContext) { return true; };
}

/*!
\}
*/
