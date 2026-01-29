/*
===========================================
Do not modify, this script is generated
===========================================
*/

class ScriptedBaseZeroingGenerator: BaseZeroingGenerator
{
	/*!
	Called on initialization.
	\param owner Parent entity.
	\param sights Parent sights component.
	*/
	event protected void OnInit(IEntity owner, BaseSightsComponent sights);
	/*!
	Called every frame when active to update current state.
	\param owner Parent entity.
	\param sights Parent sights component.
	\param timeSlice Delta of time since last update.
	*/
	event protected void OnUpdate(IEntity owner, BaseSightsComponent sights, float timeSlice);
	/*!
	Called to obtain zeroing data for a given weapon angle.
	\param weaponAngle Angle how the weapon should be held
	\param distance The distance this zeroing data is calculated for
	\param offset output Computed offset of the weapon to the eye position
	\param angles output Computed angles for holding the weapon
	\return true if the value could be computed, false otherwise. Returning false usually implies that something catastrophic is wrong (missing sight component, missing owner, etc)
	*/
	event bool WB_GetZeroingData(float weaponAngle, float distance, out vector offset, out vector angles);
	/*!
	Return the animation value for the sights animation. This usually outputs a value that makes the sight line up with the eye
	as close as possible.
	\param pitch The angle at which the weapon is held.
	\param distance The distance this zeroing data is calculated for
	\param offset output Computed offset of the weapon to the eye position
	\param angles output Computed angles for holding the weapon
	\return a float value, usually between 0 and 1, representing the percentage to forward into the sights animation
	*/
	event float WB_CalculateWeaponZeroingAnimationValue(float pitch, float distance, out vector offset, out vector angles);
	/*!
	Returns the pivot point of the sights. Usually defined in the sights component, but might be missing if for example
	the front sight pivot is not on the owner but rather on the entity it is attached to (like the M203)
	\param owner Parent entity
	\param sights Parent Sights component
	\return pivot point
	*/
	event vector WB_GetPivotPoint();
	/*!
	Initialize the generator
	\return False if the initialization failed. This can be a misconfiguration of the prefab or any other reason
	*/
	event bool WB_InitGenerator();
	/*!
	Clean up. This method is only called automatically if WB_InitGenerator has return true. Otherwise,
	cleanup either needs to take place within WB_InitGenerator itself, or WB_InitGenerator must call this function
	*/
	event void WB_CleanupGenerator();
}
