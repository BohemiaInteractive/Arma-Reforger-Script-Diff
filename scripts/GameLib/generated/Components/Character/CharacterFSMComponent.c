/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components_Character
\{
*/

/*!
FSM component class
*/
class CharacterFSMComponent: GenericComponent
{
	/*!
	Returns character owner of this component
	*/
	proto external CharacterEntity GetOwner();
	/*!
	Returns an instance for current fullbody state
	*/
	proto external CharacterFSMState GetCurrentFullbodyState();
	/*!
	Activates new fullbody state
	*/
	proto external CharacterFSMState ActivateFullbodyState(int stateIndex, CharacterFSMStateActivateParams params = null);
	/*!
	Activates or deactivates additive state
	*/
	proto external CharacterFSMState ActivateAdditiveState(int stateIndex, bool state, CharacterFSMStateActivateParams params = null);

	// callbacks

	/*!
	Event called when component is initialized - a place for custom implementation
	*/
	event protected void OnSetUp(CharacterEntity character);
	/*!
	Event called when owning character is possessed by a player
	*/
	event protected void OnPossession(CharacterEntity character);
	/*!
	Event called when owning character is dispossessed
	*/
	event protected void OnDispossession(CharacterEntity character);
	/*!
	Script implementation of FSM transitions
	*/
	event protected void OnProcessTransitions(CharacterEntity character, float timeSlice);
}

/*!
\}
*/
