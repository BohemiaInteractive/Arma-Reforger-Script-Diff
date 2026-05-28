/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Plugins_Persistence_SaveGame
\{
*/

void SaveGameOperationDelegate(bool success, Managed context = null); typedef func SaveGameOperationDelegate;
class SaveGameOperationCallback: SaveGameCallback
{
	//! Callbacks can be inherited to implement OnCompleted or constructed by passing a function to handle it with the same parameters.
	proto void SaveGameOperationCallback(SaveGameOperationDelegate delegate = null, Managed context = null);
	/*!
	Will use the delegate passed to constructor if not implemented
	\param statusCode The result status. OK as expected result, anything else needs handling / logging / throwing errors.
	\param context The optional context instance passed to constructor.
	*/
	event protected void OnCompleted(bool success, Managed context = null);

	//! Invoke the delegate assigned to this callback. Returns false if there is none. To be used from OnCompleted.
	proto external bool InvokeDelegate(bool success);
}

/*!
\}
*/
