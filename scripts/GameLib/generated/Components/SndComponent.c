/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class SndComponent: GenericComponent
{
	/*! Enables scripted component callbacks. */
	proto external protected void SetCallbackMask(SndComponentCallbacks mask);
	/*! When set to TRUE, Update is called regardless of audibility and other conditions.*/
	proto external protected void SetForceUpdate(bool forceUpdate);
	/*! When set to TRUE, Update is not called, but component stays registered in SndSystem. */
	proto external protected void SetDisableUpdate(bool forceUpdate);
	/* Component owner. */
	proto external IEntity GetOwner();
	/* Plays sound for given name. */
	proto external AudioHandle PlayStr(string name);
	/* Obsolete Returns the index of the given event name or -1 if not found */
	proto external int GetEventIndex(string name);
	/* Beginning of sound event scope. */
	[Obsolete("Not supported, will be removed!")]
	proto external int SoundEventBegin(string eventName);
	/* Sets scope parameters and plays the event. */
	[Obsolete("Not supported, will be removed!")]
	proto external int SoundEventEnd(int eventBeginId);
	/*! Returns the index of the given signal name or -1 if not found.*/
	proto external int GetSignalIndex(string name);
	/*! Sets signal value by index returned by GetSignalIndex(string name).*/
	proto external void SetSignalValue(int signal, float value);
	/* Sets signal value by name. */
	proto external void SetSignalValueStr(string signal, float value);
	/* Set transformation of all sounds played by this component */
	proto external void SetTransformation(vector transf[]);
	/* Set transformation of the sound associated with the given handle */
	proto external void SetSoundTransformation(AudioHandle handle, vector transf[]);
	/* Update playback of triggered sounds */
	proto external void UpdateTrigger();
	/* Terminate the sound associated with the given handle */
	proto external void Terminate(AudioHandle handle);
	/* Terminate all playing sounds associated with this component */
	proto external void TerminateAll();
	/* Check if any sound playing on this component */
	proto external bool IsPlaying();
	/* Check if sound associated with given handle has finished playing */
	proto external bool IsFinishedPlaying(AudioHandle handle);
	/* Returns true if handle is valid. */
	proto external bool IsHandleValid(AudioHandle handle);
	/* Fills the given array with the names of all signals found in this component*/
	proto external void GetSignalNames(out array<string> signalNames);

	// callbacks

	/*! Called when the component is audible. Audibility is determined as max from all events in acps. */
	event protected void Update(float dt);
	/*! Called when the component enters an audible range. */
	event protected void OnUpdateBegin();
	/*! Called when the component gets out of audible range. */
	event protected void OnUpdateEnd();
	/*! Requires SndComponentCallbacks::OnInit to be set. */
	event protected void OnInit(IEntity owner);
	/*! Requires SndComponentCallbacks::OnPostInit to be set. */
	event protected void OnPostInit(IEntity owner);
	/*! Requires SndComponentCallbacks::OnDelete to be set. */
	event protected void OnDelete(IEntity owner);
}

/*!
\}
*/
