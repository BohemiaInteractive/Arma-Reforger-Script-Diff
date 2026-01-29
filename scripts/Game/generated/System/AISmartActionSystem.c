/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

class AISmartActionSystem: GameSystem
{
	proto external int FindSmartActions(out notnull array<AISmartActionComponent> outSmartActions, vector pos, float fRange, array<string> aTags, EAIFindSmartAction_TagTest eTagTest);
	proto external void UpdateSmartAction(notnull AISmartActionComponent smartAction);
}

/*!
\}
*/
