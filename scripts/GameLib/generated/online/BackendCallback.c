/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

/*!
Object used as callback structure in which you will receive results of Backend requests exposed via ScriptAPIs.

For callback function you can use any function without arguments or with BackendCallback as the only argument.
If BackendCallback was provided as the argument it will contain BackendCallback used for the request that invoked it.
Allowed arguments of the function can be found in typedef of BackendCallbackFunc.

\warning Callback has to be preserved until it receives response.
         If it is deleted before it finishes then request will be automatically killed.
         If callback is not stored as ref then it will be deleted after its execution finishes.

\note Timeouts of native Backend requests should be generally considered and handled same as an error.
      But if required it can be differentiated via GetRestResult() == ERestResult.EREST_ERROR_TIMEOUT

Examples of callback functions:
\code

	// you can use function without arguments
	void MyOnSuccessFunction()
	{
		// This code will be executed on successful request
	}

	// or add BackendCallback argument
	void MyOnSuccessFunction(BackendCallback cb)
	{
		// This code will be executed on successful request

		// you can access specific API code for special behavior
		EApiCode apiCode = cb.GetApiCode();

		// you can access HTTP result code as enum
		HttpCode httpResultEnum = cb.GetHttpCode();
	}

	// use different function for error event
	void MyOnErrorFunction(BackendCallback cb)
	{
		// This code will be executed if request failed

		// you can access API code provided by backend in case of error
		EApiCode apiCode = cb.GetApiCode();

		// you can store HTTP code as int for codes undefined in HttpCode enum
		int httpResultInt = cb.GetHttpCode();
	}

\endcode

You have to set functions on the BackendCallback instance for them to be invoked:
\code

	m_MyCallback = new BackendCallback();

	m_MyCallback.SetOnSuccess(MyOnSuccessFunction);
	m_MyCallback.SetOnError(MyOnErrorFunction);

\endcode
*/
class BackendCallback: Managed
{
	//! Sets function which will be invoked if request was successful.
	proto external void SetOnSuccess(BackendCallbackFunc onSuccess);
	//! Sets function which will be invoked if request failed due to some error.
	proto external void SetOnError(BackendCallbackFunc onError);
	//! Returns result code from RestApi.
	proto external ERestResult GetRestResult();
	//! Returns type of request that invoked callback.
	proto external EBackendRequest GetBackendRequest();
	//! Returns backend error of failed request.
	proto external EBackendError GetBackendError();
	/*!
	Returns API code of the request which can represent specific reason why the request failed.
	Some API codes when provided in OnSuccess can also direct what action should be taken next
	which means that not all API Codes represents errors.
	*/
	proto external EApiCode GetApiCode();
	/*!
	Returns HTTP code result of the request.
	\note For backend services we should not generally depend on HTTP code for specific errors or states and instead use API Codes.
	      But is still good to show HTTP codes in case of errors for situations when we cannot depend on API Codes.
	*/
	proto external HttpCode GetHttpCode();
	/*!
	Returns additional message provided by the backend to the request error.
	\warning This message is not localized and is more human readable technical message for unhandled errors.
	         Localized error messages should be primarily based on API Codes.
	*/
	proto external string GetErrorMessage();
	/*!
	Returns UID of the request error.
	\note This UID is technical information necessary to find relevant entry in backend side logs.
	      Every service can have slightly different format so it is not specifically set which format this UID will have.
	*/
	proto external string GetErrorUID();
	/*!
	Returns weak reference to possible result data from request response if supported. Otherwise returns null.
	\warning Request which used this callback must specify in documentation that it provides data through callback.
	         In addition it must also specify of which type these data will be since it is necessary to cast them from Managed into type appropriate type.
	*/
	proto external Managed GetResultData();

	// callbacks

	//! OBSOLETE - WILL BE REMOVED! Use SetOnSuccess() to set callback function instead!
	[Obsolete("Use BackendCallback.SetOnSuccess() instead.")]
	event void OnSuccess(int code);
	//! OBSOLETE - WILL BE REMOVED! Use SetOnError() to set callback function instead!
	[Obsolete("Use BackendCallback.SetOnError() instead.")]
	event void OnError(int code, int restCode, int apiCode);
	//! OBSOLETE - WILL BE REMOVED! Use SetOnError() to set callback function instead and check GetRestResult() == ERestResult.EREST_ERROR_TIMEOUT!
	[Obsolete("Use BackendCallback.SetOnError() instead.")]
	event void OnTimeout();
}

/*!
\}
*/
