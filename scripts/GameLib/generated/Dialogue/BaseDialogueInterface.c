/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Dialogue
\{
*/

class BaseDialogueInterface: ScriptAndConfig
{
	//Returns true if the provided entity is involved in a Dialogue as well as the ID of said dialogue. Returns false otherwise
	proto external bool IsEntityInDialogue(notnull IEntity speaker, out DialogueID id);
	//Requests the dialogue of the provided resource to be played with the provided speaker entities and input variables.
	proto external bool RequestDialogue(ResourceName resourceName, notnull array<IEntity> speakers, array<ref DialogueVariableMappingObject> inputVariableMapObjects = null);
	//Retrieves an array with the entities involved in a dialogue, some of them may be null. Returns -1 if the provided Id doesn't represent any dialogue. Otherwise, returns number of elements in array.
	proto external int GetEntitiesInvolvedInDialogue(DialogueID id, out notnull array<IEntity> outEntities);
	//Retrieves all DataComponents of the dialogue's current node that are of the provided type. Returns number of found components.
	proto external int GetDataOfCurrentNode(DialogueID id, typename type, notnull array<DialogueData> outData);
	//Returns true if the dialogue's current node is waiting to be signaled to continue to the next one. This signal is done by requesting a Skip.
	proto external bool IsCurrentNodeWaitingForContinue(DialogueID id);
	//Returns true if the dialogue's current node can be Skipped.
	proto external bool CanCurrentNodeBeSkipped(DialogueID id);
	//Returns true if the dialogue's current node has completed it's expected duration
	proto external bool NodeHasCompletedDuration(DialogueID id);
	//Retrieves the indexes of the currently available responses.
	proto external int GetAvailableResponseIndexes(DialogueID id, notnull array<int> responseIndexes);
	//Retrieves all DataComponents of the node at a specific response index that are of the provided type. The response node doesn't have to be an available response. Returns number of found components.
	proto external int GetDataOfResponse(DialogueID id, int responseIndex, typename type, notnull array<DialogueData> outData);
	//Interaction methods
	proto external void RequestResponse(DialogueID id, int responseId);
	proto external void RequestSkip(DialogueID id);
	proto external void RequestInterrupt(DialogueID id);
	//Enables ticking of the Interface
	proto external void EnableTicking(bool enable);

	// callbacks

	//User Callbacks
	event void OnDialogueStarted(DialogueID id);
	event void OnDialogueEnded(DialogueID id);
	event void OnNodeDurationCompleted(DialogueID id);
	event void OnEventApplied(DialogueID id);
	event void OnTick(float deltaTime);
}

/*!
\}
*/
