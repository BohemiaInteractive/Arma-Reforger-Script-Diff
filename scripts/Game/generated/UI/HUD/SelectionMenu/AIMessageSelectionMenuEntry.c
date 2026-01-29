/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI_HUD_SelectionMenu
\{
*/

class AIMessageSelectionMenuEntry: ScriptedSelectionMenuEntry
{
	//! Create new AIMessageSelectionMenuEntry instance with provided message
	void AIMessageSelectionMenuEntry(AIMessage pAIMessage)
	{
		this.SetAIMessage(pAIMessage);
	}

	//! Set the message for this menu entry
	proto external void SetAIMessage(AIMessage pAIMessage);
	//! Retrieve the message from this menu entry. Can be null.
	proto external AIMessage GetAIMessage();
}

/*!
\}
*/
