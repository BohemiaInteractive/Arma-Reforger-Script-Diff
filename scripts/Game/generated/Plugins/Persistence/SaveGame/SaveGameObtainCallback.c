/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Persistence_SaveGame
\{
*/

void SaveGameObtainDelegate(bool success, array<SaveGame> saves, Managed context = null); typedef func SaveGameObtainDelegate;
class SaveGameObtainCallback: SaveGameCallback
{
	//! Callbacks can be inherited to implement OnResult or constructed by passing a function to handle it with the same parameters.
	proto void SaveGameObtainCallback(SaveGameObtainDelegate delegate = null, Managed context = null);
	/*!
	Will use the delegate passed to constructor if not implemented
	\param statusCode The result status. OK as expected result, anything else needs handling / logging / throwing errors.
	\param context The optional context instance passed to constructor.
	*/
	event protected void OnCompleted(bool success, array<SaveGame> saves, Managed context = null);

	//! Invoke the delegate assigned to this callback. Returns false if there is none. To be used from OnResult.
	proto external bool InvokeDelegate(bool success, array<SaveGame> saves);
}

/*!
\}
*/
