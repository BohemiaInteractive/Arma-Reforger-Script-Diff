/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence
\{
*/

class PersistenceResultCallback: PersistenceCallback
{
	//! Callbacks can be inherited to implement OnResult or constructed by passing a function to handle it with the same parameters.
	proto void PersistenceResultCallback(PersistenceResultDelegate delegate = null, Managed context = null);
	/*!
	Will use the delegate passed to constructor if not implemented
	\param statusCode The result status. OK as expected result, anything else needs handling / logging / throwing errors.
	\param result The result instance which must be casted into your expected type. Usually IEntity or custom script class.
				  Special case: If a specific id should have been fetched, but the status code is not OK, the result is the persistence id it failed to fetch.
	\param isLast If the load request resulted in multiple instance the callback wil be invoked as many times with the last invoke being isLast=true.
				  You can store all results in a member array and pass them somewhere else after recieving the last one.
	\param context The optional context instance passed to constructor.
	*/
	event protected void OnResult(EPersistenceStatusCode statusCode, Managed result, bool isLast, Managed context = null);

	//! Invoke the delegate assigned to this callback. Returns false if there is none. To be used from OnResult.
	proto external bool InvokeDelegate(EPersistenceStatusCode statusCode, Managed result, bool isLast, Managed context = null);
}

/*!
\}
*/
