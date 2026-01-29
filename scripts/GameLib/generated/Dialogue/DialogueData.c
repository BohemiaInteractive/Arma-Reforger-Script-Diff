/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Dialogue
\{
*/

class DialogueData: DialogueBaseElement
{
	private void DialogueData();
	private void ~DialogueData();

	proto external bool IsTimeSensitive();
	// Initial value = true
	proto external bool CanBeSkipped();
	// Initial value = false
	proto external bool ReadyForTransition();
	proto external void SetCanBeSkipped(bool skipable);
	proto external void SetReadyForTransition(bool isReady);
	/*
	Sets the value of the Dialogue variable which's name matches the provided one.
	The use of this variable must have been previously declared in GetVariableNames().
	If no matching variable is found or the incorrect type is used in the 'value' variable, nothing will be set.
	*/
	proto void SetVariable(string name, void value);

	// callbacks

	// Invoked by DialogueGlobalData counterpart during initialization of data.
	event void InitGlobalData(notnull DialogueGlobalData globalData);
	event void OnStart(IEntity pSpeaker);
	event void OnStartAt(IEntity pSpeaker, float timeInNode) { OnStart(pSpeaker); };
	event void OnEnd(IEntity pSpeaker);
	event void OnUpdate(IEntity pSpeaker, float timeSlice, float timeInNode);
	event void OnInterrupt(IEntity pSpeaker) { OnEnd(pSpeaker); };
	event void OnSkip(IEntity pSpeaker) { OnEnd(pSpeaker); };
}

/*!
\}
*/
