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
Base class for FSM state instance
*/
class CharacterFSMState: ScriptAndConfig
{
	/*!
	private contructor - it's forbidden to create instances using operator new
	*/
	private void CharacterFSMState();

	//! sets FINISHED internal flag - should be called when this states decides to be finished and FSM component should react on this event
	proto external void SetFinished();

	// callbacks

	/*!
	script event called when state is created - animation controls binding should happen here
	*/
	event protected void OnBindAnimControls(CharacterAnimGraphComponent animGraphComponent);
	/*!
	script event called when state is activated
	*/
	event protected void OnActivate(CharacterEntity character, CharacterFSMState prevState, CharacterFSMStateActivateParams params);
	/*!
	script event called when state is deactivated
	*/
	event protected void OnDeactivate(CharacterEntity character);
	/*!
	script event called during MainLogic phase
	*/
	event protected void OnMainLogicUpdate(CharacterEntity character, float timeSlice);
	/*!
	script event called during PreAnim phase
	*/
	event protected void OnPreAnimUpdate(CharacterEntity character, float timeSlice, bool isSimulatedProxy);
	/*!
	script event called during PostAnim phase
	*/
	event protected void OnPostAnimUpdate(CharacterEntity character, float timeSlice, bool isSimulatedProxy);
}

/*!
\}
*/
