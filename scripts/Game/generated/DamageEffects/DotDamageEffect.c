/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup DamageEffects
\{
*/

class DotDamageEffect: SCR_PersistentDamageEffect
{
	/*!
	Applies DOT equivalent to timeSlice seconds.
	Automatically increases the current duration and total damage of this damage effect
	That duration uses the accurate time slice (check GetAccurateTimeSlice for more info)
	Calling this will also terminate the damage effect if the maximum duration is reached
	\param float timeSlice: amount of time that has passed.
	\param SCR_ExtendedDamageManagerComponent dmgManager: damage manager containing this DamageEffect
	*/
	proto external void DealDot(float timeSlice, SCR_ExtendedDamageManagerComponent dmgManager);
	/*
	Applies DOT equivalent to timeSlice seconds, with default logic override options.
	This method automatically increases total damage, but not duration.
	Keep in mind that if this method is used, GetDPS() might not be accurate anymore.
	\param HitZone hitzone: This allows you to have the damage effect added to one hitzone, but deal damage to a different one.
	\param float dotAmount: Amount of damage to be applied
	\param DotDamageEffectTimerToken token: Instance of DotDamageEffectTimerToken, obtained by calling UpdateTimer().
	Unlike DealDot, this method doesn't update the timer. The token is here so people don't forget to update the timer when using this method.
	\param ExtendedDamageManagerComponent dmgManager: DmgManager containing this DamageEffect
	*/
	proto external void DealCustomDot(notnull HitZone hitzone, float dotAmount, notnull DotDamageEffectTimerToken token, SCR_ExtendedDamageManagerComponent dmgManager);
	/*!
	set amount of damage per second. Only works on server.
	\param float dps: damage per second dealt by this DamageEffect.
	*/
	proto external void SetDPS(float dps);
	/*!
	Set max duration for this DOT effect. Value of 0 will make it infinite.  Only works on server.
	\param float newMaxduration: New max duration for this DamageEffect
	*/
	proto external void SetMaxDuration(float newMaxduration);
	/*!
	Increases the max duration of this damage effect.  Only works on server.
	\param float amount: how much the max duration should be increased by
	*/
	proto external void ExtendMaxDuration(float amount);
	/*!
	Updates current duration of this damage effect by timeSlice and returns a token to be used for DealCustomDot.
	It will also mark the effect for termination if the current duration > max duration.
	\param float timeSlice: current duration will be increased by this amount of time.
	\param SCR_ExtendedDamageManagerComponent dmgManager: Damage manager containing this damage effect
	\return DotDamageEffectTimerToken: Empty class that can be used as an input for DealCustomDot
	*/
	proto external DotDamageEffectTimerToken UpdateTimer(float timeSlice, SCR_ExtendedDamageManagerComponent dmgManager);
	/*!
	timeSlice can exceed the remaining time for DOT, this will return the accurate amount of DOT time left that can be applied.
	E.g.: Remaining time: 5s, timeSlice: 7s, this would return 5s
	\param float timeSlice: current timeSlice
	\return actual amount of time this DamageEffect will actually be applied for
	*/
	proto external float GetAccurateTimeSlice(float timeSlice);
	/*!
	Returns amount of Damage Per Second applied by this DamageEffect
	*/
	proto external float GetDPS();
	/*!
	Returns amount of time this effect has been running. Current duration can only be increased through DealDot or UpdateTimer
	*/
	proto external float GetCurrentDuration();
	/*
	Returns total amount of time this DamageEffect will run for. 0 = infinite duration
	*/
	proto external float GetMaxDuration();
}

/*!
\}
*/
