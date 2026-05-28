/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Objects_Character
\{
*/

class CharacterSimStrategyFSM: CharacterSimStrategy
{
	/*!
	Returns the ID of a state
	*/
	proto external int GetStateId(string stateName);
	/*!
	Returns an instance for current fullbody state
	*/
	proto external CharacterFSMState GetCurrentFullbodyState();
	/*!
	Returns true if a state with passed id is active
	*/
	proto external bool IsStateActive(int stateId);
	/*!
	Activates new fullbody state - use GetStateId for getting the id of a state you want to activate
	*/
	proto external CharacterFSMState ActivateFullbodyState(int stateId);
	/*!
	Activates or deactivates additive state - use GetStateId for getting the id of a state you want to activate
	*/
	proto external CharacterFSMState ActivateAdditiveState(int stateId, bool enabled);
	/*!
	If state is active, set it to finished
	*/
	proto external void CancelState(int stateId);
	/*!
	Activates a callback on a state - when additiveStateId is zero, callback is activated for current fullbody state
	*/
	proto external CharacterFSMStateCb StartCallback(typename callbackClassType, int additiveStateId = 0);
	/*!
	Stops a callback on a state - when additiveStateId is zero, callback is deactivated for current fullbody state
	*/
	proto external void StopCallback(int additiveStateId = 0);
}

/*!
\}
*/
