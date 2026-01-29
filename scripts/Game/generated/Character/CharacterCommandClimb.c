/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

/*!
command itself
*/
class CharacterCommandClimb: CharacterCommand
{
	//! returns the state of climb (enum value of ClimbStates);
	proto external int GetState();
	//! returns world space position of climbing grab point
	proto external vector GetGrabPointWS();
	//! returns world space position of landspot after climbing over
	proto external vector GetClimbOverStandPointWS();
	/*
	debug draws climb heauristics
	pDebugDrawLevel viz DebugDrawClimb
	*/
	static proto bool DoClimbTest(ChimeraCharacter pHuman, CharacterCommandClimbSettings pSettings, CharacterCommandClimbResult pResult);
}

/*!
\}
*/
