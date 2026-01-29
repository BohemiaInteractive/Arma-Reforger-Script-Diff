/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup DamageEffects
\{
*/

class PersistentDamageEffect: SCR_DamageEffect
{
	/*!
	Terminates this DamageEffect. It will be removed from the list of persistent effects on it´s containing damage manager.
	Only works on server.
	*/
	proto external void Terminate();
	/*!
	When active, the damage effect will be updated on frame
	\param bool value: When true, EOnFrame will get called when this damage effect is contained on a damage manager.
	*/
	proto external void SetActive(bool value);
	/*!
	Checks if this effect is active or not.
	\return Returns true if the DamageEffect is being updated on frame.
	*/
	proto external bool IsActive();

	// callbacks

	//On frame logic for the persistent damage effect. Will only be called if active
	event void EOnFrame(float timeSlice, SCR_ExtendedDamageManagerComponent dmgManager);
	/*!
	* It is indented to work togeter with SCR_OnEffectApplied to send replicated data to clients.
	*/
	event bool SaveApplyEffect(ScriptBitWriter w) { return true; };
	/*!
	* It is indented to work togeter with SCR_OnEffectApplied to load replicated data to clients.
	*/
	event bool LoadApplyEffect(ScriptBitReader r) { return true; };
}

/*!
\}
*/
