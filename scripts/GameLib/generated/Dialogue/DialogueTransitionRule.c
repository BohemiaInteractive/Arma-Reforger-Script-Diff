/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Dialogue
\{
*/

//! Wrapper for testing if a condition is met. Used by DialogueNodeTransition.
class DialogueTransitionRule: DialogueBaseElement
{
	private void DialogueTransitionRule();
	private void ~DialogueTransitionRule();

	event protected void OnInit();
	//! Function that performs a scripted condition test. Returns true if the scripted condition test passes.
	event protected bool TestFunction(IEntity currentSpeaker, IEntity nextSpeaker);
}

/*!
\}
*/
