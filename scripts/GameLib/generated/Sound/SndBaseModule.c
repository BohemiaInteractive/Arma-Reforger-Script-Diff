/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Sound
\{
*/

/*!
To create a module, you just need to create a class which inherits from BaseSndModule.
*/
class SndBaseModule: ScriptAndConfig
{
	private void SndBaseModule();

	proto external protected SndSystem GetSndSystem();
	//! Sound event.
	proto external AudioHandle SoundEvent(string eventName);
	//! Sets signal value by name.
	proto external void SetSignalValueStr(string signal, float value);
	//! Sets signal value by index returned by GetSignalIndex(string name).
	proto external void SetSignalValue(int signal, float value);
	//! Set transformation of all sounds played by this component.
	proto external void SetTransformation(vector transf[]);
	//! Sets transformation of the sound associated with the given handle
	proto external void SetSoundTransformation(AudioHandle handle, vector transf[]);
	//! Update playback of triggered sounds.
	proto external void UpdateTrigger();
	//! Check if sound associated with given handle has finished playing.
	proto external bool IsFinishedPlaying(AudioHandle handle);
	//! Returns the index of the given signal name or -1 if not found.
	proto external int GetSignalIndex(string name);
	//! Terminate the sound associated with the given handle
	proto external void Terminate(AudioHandle handle);

	// callbacks

	//! Called after owner system is initialized.
	event protected void OnInit();
	//! Called before parent system is cleaned up.
	event protected void OnCleanup();
	//! Called after parent system is ticked.
	event protected void OnUpdate(float timeslice);
	//! Diag for the module.
	event protected void OnDiag(float timeslice);
}

/*!
\}
*/
