/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Audio
\{
*/

sealed class AudioSystem
{
	private void AudioSystem();
	private void ~AudioSystem();

	// Mastering voices categories.
	static const int SFX = 0;
	static const int Music = 1;
	static const int VoiceChat = 2;
	static const int Dialog = 3;
	static const int UI = 4;

	static const int BV_None = 0;
	static const int BV_Sphere = 1;
	static const int BV_Box = 2;
	static const int BV_Cylinder = 3;

	//! Play *.wav file.
	static proto AudioHandle PlaySound(string resourceName);
	//! Preload audio file(*.acp).
	static proto bool PlayEventInitialize(string resourceName);
	static proto AudioHandle PlayEvent(string resourceName, string eventName, vector transf[], array<string> names = null, array<float> values = null);
	static proto bool IsSoundPlayed(AudioHandle handle);
	static proto void TerminateSound(AudioHandle handle);
	//! Terminates a sound with specific a handle, where fade-out can be applied for a specific amount of time.
	static proto void TerminateSoundFadeOut(AudioHandle handle, bool fade, float fadeTime);
	//! Sets transformation for given audio handle, return FALSE if audio handle is not valid.
	static proto bool SetSoundTransformation(AudioHandle handle, vector transf[]);
	//! Returns -1.0 for the inaudible event, otherwise, it returns the distance from the passed position to the listener.
	static proto float IsAudible(string resourceName, string eventName, vector position);
	//! Returns distance to the listener.
	static proto float GetDistance(vector position);
	/*
	Allow update bounding volume for existing audio source.
		BV_Sphere	: param0 = radius
		BV_Box		: param0 = x, param1 = y, param2 = z
		BV_Cylinde	: param0 = radius, param1 = height
	*/
	static proto bool SetBoundingVolumeParams(AudioHandle handle, int volumeType, float params0, float param1, float param2);
	//! Pauses currently playing sounds and rejects insert new sounds. Use: AudioSystem.Pause((1 << AudioSystem.SFX) | (1 << AudioSystem.VoiceChat))
	static proto void Pause(int mastersMask);
	//! Resumes paused sources and allow insertion of new ones.
	static proto void Resume(int mastersMask);
	//------------------------------------------------------------------------------------------------
	static proto float OutputVolume();
	//------------------------------------------------------------------------------------------------
	static proto bool SetMasterVolume(int id, float volume);
	static proto float GetMasterVolume(int id);
	//! Returns index of OutputState instance, in case of error, returns -1. There is only one instance of OutputState for the given name.
	static proto bool CreateOutputState(string resourceName);
	//! Returns index of OutputState for given resource name. -1 in case no instance is found for the given name.
	static proto int GetOutputStateIdx(string resourceName);
	//! Returns a total number of instances.
	static proto int GetOutputStateCount();
	//! Returns the index of the given signal, -1 if the signal is not present.
	static proto int GetOutpuStateSignalIdx(int outStateIdx, string signal);
	//! Sets signal to the given value in specified OutputState.
	static proto void SetOutputStateSignal(int outStateIdx, int idx, float value);
	//! Returns ID of a variable by Name.
	static proto int GetVariableIDByName(string varName, string resourceName);
	//! Returns Name of a variable by ID.
	static proto string GetVariableNameByID(int ID, string resourceName);
	//! Returns current value of a variable by Name.
	static proto float GetVariableValue(string varName, string resourceName);
	//! Returns current value of a variable by ID.
	static proto float GetVariableByID(int ID, string resourceName);
	//! Returns false if variable does not exist.
	static proto bool SetVariableByName(string varName, float value, string resourceName);
	//! Returns false if variable does not exist.
	static proto bool SetVariableByID(int ID, float value, string resourceName);
	//! Resets all applicable values to 0 or default value
	static proto void ResetVariables();
}

/*!
\}
*/
