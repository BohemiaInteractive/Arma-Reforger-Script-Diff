/*
===========================================
Do not modify, this script is generated
===========================================
*/

#ifdef WORKBENCH

/*!
\addtogroup WorkbenchAPI_Modules
\{
*/

sealed class DialogueEditor: WBModuleDef
{
	proto external BaseContainer GetCurrentDialogue();
	proto external BaseContainer GetCurrentNode();
	proto external void BeginEditAction(string name = string.Empty);
	proto external void EndEditAction(bool fullRefreshNeeded = false);
}

/*!
\}
*/

#endif // WORKBENCH
