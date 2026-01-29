/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup HitZone
\{
*/

class HitZone: ScriptAndConfig
{
	//All non true damage taken will be multiplied by this amount
	proto external float GetBaseDamageMultiplier();
	//returns true if this hitzone has collider nodes.
	proto external bool HasColliderNodes();
	//returns index of collider descriptor attached to collider ID
	proto external int GetColliderDescriptorIndex(int colliderID);
	//Get damage multipler for a type of damage in this hitzone
	proto external float GetDamageMultiplier(EDamageType dmgType);
	//return damage reduction
	proto external float GetDamageReduction();
	//return damage threshold
	proto external float GetDamageThreshold();
	/*!
	\param colliderNames array which is filled with collider names of this hitzone
	*/
	proto external int GetAllColliderNames(out notnull array<string> colliderNames);
	/*!
	Sets damage over time for this particular hitzone. And invoke particular
	event on DamageManagerComponent. It has no effect if there is
	already a DOT of same type and DPS. Cannot be called on proxy.
	\param dmgType Type of damage
	\param dps Damage per second applied to this HitZone
	*/
	proto external void SetDamageOverTime(EDamageType dmgType, float dps);
	//return damage per second of specified type
	proto external float GetDamageOverTime(EDamageType dmgType);
	//Returns number of collider decriptors
	proto external int GetNumColliderDescriptors();
	//script method wrappers
	proto external HitZoneContainerComponent GetHitZoneContainer();
	//Sets the health of this hitzone. Only works when called from server.
	proto external void SetHealth(float health);
	//Sets the scaled health of this hitzone [0, 1]. Only works when called from server.
	proto external void SetHealthScaled(float health);
	//Sets the max health of this hitzone. Only works when called from server.
	proto external void SetMaxHealth(float maxHealth, ESetMaxHealthFlags flag = ESetMaxHealthFlags.NONE);
	//Gets the current health of this hitzone. Avoid tying game logic to GetHealth, there is no guarantee of synchronization! Tie game logic to hitzone damage state.
	proto external float GetHealth();
	//Gets the current scaled health of this hitzone. Avoid tying game logic to this function, there is no guarantee of synchronization! Tie game logic to hitzone damage state.
	proto external float GetHealthScaled();
	//Returns health % needed to trigger the requested damage state.
	proto external float GetDamageStateThreshold(EDamageState damageState);
	//Returns max health of this hitzone
	proto external float GetMaxHealth();
	//Returns hitzone name
	proto external string GetName();
	//Gets current damage state of the hitzone
	proto external EDamageState GetDamageState();
	//Returns the amount of damage to be received at once needed to be considered critical
	proto external float GetCriticalDamageThreshold();
	//Gets previous damage state of the hitzone
	proto external EDamageState GetPreviousDamageState();
	//Hitzone will handle this amount of damage. Damage will only be applied if called from server. Use DamageManager.HandleDamage when possible, using this skips DamageManager.OnDamage
	proto external void HandleDamage(float damage, int damageType, IEntity instigator);
	//Returns true if its a proxy
	proto external bool IsProxy();
	//! Returns all collider IDs attached to this hitzone
	proto external int GetColliderIDs(out notnull array<int> outIDs);
	//gets collider description
	proto bool TryGetColliderDescription(IEntity owner, int descIndex, out vector transformLS[4], out int boneIndex, out int nodeID);
	//Gets collider description from name
	proto bool TryGetColliderDescriptionFromName(IEntity owner, string colliderName, out vector transformLS[4], out int boneIndex, out int nodeID);

	// callbacks

	/*!
	Called to save information related to the hitzone
	EnableScriptDataStreaming has to return true for this method to be called
	\param ScriptBitWriter writer: Buffer where you will write the data necessary.
	Remember: Don't waste bandwidth, only write information if it has changed.
	This means that if it has no changes, you should only send a bit to indicate that.
	*/
	event protected bool Save(notnull ScriptBitWriter writer);
	/*!
	Load the buffer written on Save
	EnableScriptDataStreaming has to return true for this method to be called
	\param ScriptBitReader reader: Buffer to read the data from.
	*/
	event protected bool Load(notnull ScriptBitReader reader);
	//! Use it to save bandwidth in some default conditions. Use it if Save and Load are overriden.
	event protected bool HasDataToReplicate();
	//! Call OnInit method from script
	event void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent);
	//! Called when damage changes
	event protected void OnHealthSet();
	//! Called when max damage changes
	event protected void OnMaxHealthChanged();
	//! Called when damage state changes
	event protected void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP);
	//! Called when the damage has been dealt by the server, doesn't get called for DOT.
	event void OnDamage(notnull BaseDamageContext damageContext);
	/*!
	Calculates the amount of damage a hitzone will receive.

	\param BaseDamageConext DamageContext for this computation
	\param bool isDOT true if this computation is fot DOT
	*/
	event float ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT);
}

/*!
\}
*/
