/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class CommunicationSoundComponent: SoundComponent
{
	//! Add a soundevent with priority to the priority queue which will be played in order of priority
	proto external void SoundEventPriority(string eventName, int priority, bool ignoreQueue = false);

	// callbacks

	event void OnSoundEventFinished(string eventName, AudioHandle handle, int priority, bool terminated);
	event void OnSoundEventStarted(string eventName, AudioHandle handle, int priority);
}

/*!
\}
*/
