/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class BaseSoundComponentClass: GenericComponentClass
{
}

class BaseSoundComponent: GenericComponent
{
	/* Fills the given array with the names of all sound events found in this component */
	proto external int GetEvents(out array<string> events);
	/* Returns the index of the given event name or -1 if not found */
	proto external int GetEventIndex(string name);
	/* Fills the given array with the names of all signals found in this component*/
	proto external void GetSignalNames(out array<string> signalNames);
	/* Returns the index of the given signal name or -1 if not found. */
	proto external int GetSignalIndex(string name);
	/* Set signal value by index. See: GetSignalIndex method */
	proto external void SetSignalValue(int index, float value);
	/* Set signal value by name */
	proto external void SetSignalValueStr(string signal, float value);
	/* Play sound event by name */
	proto external AudioHandle PlayStr(string name);
	/* Play sound event by index. See: GetEventIndex method */
	proto external AudioHandle Play(int name);
	/* Update playback of triggered sounds */
	proto external void UpdateTrigger();
	/* Terminate all playing sounds associated with this component */
	proto external void TerminateAll();
	/* Terminate the sound associated with the given handle */
	proto external void Terminate(AudioHandle handle);
	/* Check if sound associated with given handle has finished playing */
	proto external bool IsFinishedPlaying(AudioHandle handle);
	/* Check if any sound playing on this component */
	proto external bool IsPlaying();
	/* Check if handle is valid */
	proto external bool IsHandleValid(AudioHandle handle);
	/* Set transformation of all sounds played by this component */
	proto external void SetTransformation(vector transf[]);
	/* Set transformation of the sound associated with the given handle */
	proto external void SetSoundTransformation(AudioHandle handle, vector transf[]);
	/* Enable debug mode. */
	proto external void SetDebug(bool value);
	/* Set mute mode. */
	proto external void SetMute(bool value);
	/* Returns the overall gain of the sound associated with the given handle */
	proto external float GetGain(AudioHandle handle);
}

/*!
\}
*/
