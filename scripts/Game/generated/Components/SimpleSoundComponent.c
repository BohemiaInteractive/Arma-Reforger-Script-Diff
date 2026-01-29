/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class SimpleSoundComponentClass: SndComponentClass
{
}

class SimpleSoundComponent: SndComponent
{
	//! Enables the dynamic simulation.
	proto external void EnableDynamicSimulation(bool value);
	//! Set flag for script callbacks.
	proto external void SetScriptedMethodsCall(bool state);
	//! TRUE when flag for script callbacks is set.
	proto external bool IsScriptedMethodsCallEnabled();

	// callbacks

	event protected void OnFrame(IEntity owner, float timeSlice);
	//! Call when component is in range
	event protected void UpdateSoundJob(IEntity owner, float timeSlice);
	// //! Called when dynamic simulation is started.
	event protected void OnUpdateSoundJobBegin(IEntity owner);
	// //! Called when dynamic simulation is stopped.
	event protected void OnUpdateSoundJobEnd(IEntity owner);
}

/*!
\}
*/
