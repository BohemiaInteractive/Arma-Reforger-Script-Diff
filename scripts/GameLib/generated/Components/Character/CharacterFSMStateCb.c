/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components_Character
\{
*/

class CharacterFSMStateCb
{
	/*!
	private contructor - it's forbidden to create instances using operator new
	*/
	private void CharacterFSMStateCb();

	proto external bool IsStateFinished();
	proto external bool IsStateRunning();
	proto external bool IsStateIn();
	proto external bool IsStateLoop();
	proto external bool IsStateOut();

	// callbacks

	event protected void OnInit(CharacterFSMState state, CharacterEntity character);
	event protected void OnFinished(CharacterFSMState state, CharacterEntity character);
	event protected void OnCanceled(CharacterFSMState state, CharacterEntity character);
	event protected void OnMainLogicUpdate(CharacterFSMState state, CharacterEntity character, float timeSlice);
	event protected void OnPreAnimUpdate(CharacterFSMState state, CharacterEntity character, float timeSlice, bool isSimulatedProxy);
	event protected void OnPostAnimUpdate(CharacterFSMState state, CharacterEntity character, float timeSlice, bool isSimulatedProxy);
}

/*!
\}
*/
