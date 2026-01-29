/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class MusicManagerClass: GenericEntityClass
{
}

class MusicManager: GenericEntity
{
	//! 'Forces' a sound to play without taking into account the automated evaluation process
	proto external void Play(string musicName);
	//! Stops either a specific Music that is playing by name OR stops whatever is currently playing (when the string is empty)
	proto external void Stop(string musicName);
	//! (Un)Mutes a category locally. Does not influence any categories muted from server side
	proto external void MuteCategory(MusicCategory musicCategory, bool mute, bool stopNow);
	//! Requests server (from any client) to mute a category on the server, this muted category is seperated from your own (locally) muted categories
	proto external void RequestServerMuteCategory(MusicCategory musicCategory, bool mute);
	//! Checks whether a given category is muted by the server (game master)
	proto external bool ServerIsCategoryMuted(MusicCategory musicCategory);
	//! Sets default used MusicManager signals
	proto external void SetManagerSignals();
	//! Get index for signal name. Return -1 if signal not found.
	proto external int GetSignalIndex(string name);
	//! Set signal value by 'index'. Index is obtained by GetSignalIndex method.
	proto external void SetSignalValue(int index, float value);
	//! Set signal value by signal name.
	proto external void SetSignalValueStr(string signal, float value);
}

/*!
\}
*/
