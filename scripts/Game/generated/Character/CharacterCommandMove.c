/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

//! CharacterCommandMove
class CharacterCommandMove: CharacterCommand
{
	proto external float GetAdjustedStanceAmount();
	/*!
	direction held on keyboard/controller
	-180 -90 0 90 180 angles of input movement (no filtering, no postprocesses)
	return false in idle states
	*/
	proto bool GetCurrentInputAngle(out float pAngle);
	//! -180 -90 0 90 180 angles of current movement
	proto external float GetCurrentMovementAngle();
	//! 0,1,2..3 idle, walk, run, sprint
	proto external float GetCurrentMovementSpeed();
	proto external float GetMovementSlopeAngle();
	proto external float GetLateralSlopeAngle();
	proto external bool IsRolling();
	proto external bool IsBlendingOutRoll();
}

/*!
\}
*/
