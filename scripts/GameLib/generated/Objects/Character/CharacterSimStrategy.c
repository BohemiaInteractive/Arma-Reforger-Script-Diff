/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Objects_Character
\{
*/

class CharacterSimStrategy: ScriptAndConfig
{
	/*!
	Event called owner entity initializes
	*/
	event protected void OnInit(CharacterEntity character);
	/*!
	Event called when owning character is possessed by a player
	*/
	event protected void OnPossession(CharacterEntity character);
	/*!
	Event called when owning character is dispossessed
	*/
	event protected void OnDispossession(CharacterEntity character);
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
	/*!
	script event for bumping replicated properties
	*/
	event protected void OnReplicate(CharacterEntity character);
}

/*!
\}
*/
