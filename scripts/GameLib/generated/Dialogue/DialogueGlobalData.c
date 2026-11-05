/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Dialogue
\{
*/

class DialogueGlobalData: ScriptAndConfig
{
	//! Retrieves data components of given type and returns the first one. Optionally you can get them all as an array.
	proto external DialogueData GetCurrentNodeData(typename type, out array<DialogueData> outData = null);
	//! Returns speaker index of the current node.
	proto external int GetCurrentNodeSpeakerIndex();

	// callbacks

	// --- Event handling
	event void OnPreStart(IEntity pSpeaker);
	event void OnStart(IEntity pSpeaker);
	event void OnStartAt(IEntity pSpeaker, float timeInNode) { OnStart(pSpeaker); };
	event void OnEnd(IEntity pSpeaker);
	event void OnUpdate(IEntity pSpeaker, float timeSlice, float timeInNode);
	event void OnFixedUpdate(IEntity pSpeaker, float timeSlice, float timeInNode);
	event void OnInterrupt(IEntity pSpeaker);
	event void OnSkip(IEntity pSpeaker);
	//! Declares which DialogueData type will be initialized by this.
	event typename GetDataType();
	//! Called when this type is set. Before doing global initialization.
	event void OnSetGlobalData();
}

/*!
\}
*/
