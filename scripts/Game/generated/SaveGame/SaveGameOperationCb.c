/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup SaveGame
\{
*/

class SaveGameOperationCb: Managed
{
	//! Callbacks can be inherited to implement OnResult or constructed by passing a function to handle it with the same parameters.
	proto void SaveGameOperationCb(SaveGameOperationDelegate delegate = null, Managed context = null);
	/*!
	Will use the delegate passed to constructor if not implemented
	\param statusCode The result status. OK as expected result, anything else needs handling / logging / throwing errors.
	\param context The optional context instance passed to constructor.
	*/
	event protected void OnCompleted(bool success, Managed context = null);

	//! Invoke the delegate assigned to this callback. Returns false if there is none. To be used from OnResult.
	proto external bool InvokeDelegate(bool success, Managed context = null);
}

/*!
\}
*/
