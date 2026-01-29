/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class ActionsPerformerComponentClass: GameComponentClass
{
}

class ActionsPerformerComponent: GameComponent
{
	proto external void StartAction(BaseUserAction action);
	proto external void PerformAction(BaseUserAction action);
	proto external void PerformContinuousAction(BaseUserAction action, float timeSlice);
	proto external void CancelAction(BaseUserAction action);
}

/*!
\}
*/
