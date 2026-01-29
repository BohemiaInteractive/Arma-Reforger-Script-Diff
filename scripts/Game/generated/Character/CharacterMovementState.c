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
HumanMovementState - movement state, command, stance, movement, human.GetMovementState(movementState)
*/
class CharacterMovementState
{
	int m_CommandTypeId; //! current command's id
	int m_iStanceIdx; //! current stance, only if the command has a stance
	float m_fMovement; //! current movement (0 idle, 1 walk, 2-run, 3-sprint), only if the command has a movement 
	float m_fLeaning; //! leaning state (not all commands need to have all movements)
	float m_fDynamicStance;	//! current adjusted stance 0.f - 1.f, only if the command has a adjusted stance
	ECharacterDataState	m_DataState; //! simulation state flags

}

/*!
\}
*/
