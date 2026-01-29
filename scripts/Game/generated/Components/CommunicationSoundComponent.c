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
	//! Get list of metadata. Last item in array is 'textFormat'.
	proto external void GetMetadata(AudioHandle handle, out notnull array<string> metadata);
	//! Add a soundevent with priority to the priority queue which will be played in order of priority
	proto external void SoundEventPriority(string eventName, int priority, bool ignoreQueue = false);

	// callbacks

	event void OnSoundEventFinished(string eventName, AudioHandle handle, int priority, bool terminated);
	event void OnSoundEventStarted(string eventName, AudioHandle handle, int priority);
	event protected void HandleMetadata(array<string> metadata, int priority, float distance);
}

/*!
\}
*/
