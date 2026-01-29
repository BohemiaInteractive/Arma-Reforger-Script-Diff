/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

class CharacterCommandScripted: ScriptedCommand
{
	event bool ShouldForceFreeLook();
	event bool IsRootMotionControlled();
	event void UpdateTransformNoRootMotion(out vector transform[4], float dt);
	event bool TerminateCommand();
	event void OnPossess();
}

/*!
\}
*/
