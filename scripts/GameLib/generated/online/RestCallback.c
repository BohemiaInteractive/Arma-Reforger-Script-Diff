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
Object used as callback structure in which you will receive results of scripted RestAPI calls.

For callback function you can use any function without arguments or with RestCallback as the only argument.
If RestCallback was provided as the argument it will contain RestCallback used for the request that invoked it.
Allowed arguments of the function can be found in typedef of RestCallbackFunc.

\warning If callback is not stored as ref then it will be deleted after its execution finishes.

Examples of callback functions:
\code

	// you can use function without arguments
	void MyOnSuccessFunction()
	{
		// This code will be executed on successful request
	}

	// or add RestCallback argument
	void MyOnSuccessFunction(RestCallback cb)
	{
		// This code will be executed on successful request

		// you can access data from the response
		string data = cb.GetData();

		// you can access HTTP result code as enum
		HttpCode httpResultEnum = cb.GetHttpCode();

		// or it can be stored as int for codes undefined in enum
		int httpResultInt = cb.GetHttpCode();
	}

	// use different function for error event
	void MyOnErrorFunction(RestCallback cb)
	{
		// This code will be executed if request failed

		if (cb.GetRestResult() == ERestResult.EREST_ERROR_TIMEOUT)
		{
			// Handle error caused by request timeout
		}

		// you can access data if any were provided by the RestApi
		string data = cb.GetData()
	}

\endcode

You have to set functions on the RestCallback instance for them to be invoked:
\code

	m_MyCallback = new RestCallback();

	m_MyCallback.SetOnSuccess(MyOnSuccessFunction);
	m_MyCallback.SetOnError(MyOnErrorFunction);

\endcode
*/
class RestCallback: Managed
{
	/*!
	Sets function which will be invoked if request was successful.
	Refer to RestCallbackFunc to see what arguments are required for the function.
	*/
	proto external void SetOnSuccess(RestCallbackFunc onSuccess);
	/*!
	Sets function which will be invoked if request failed due to some error.
	Refer to RestCallbackFunc to see what arguments are required for the function.
	*/
	proto external void SetOnError(RestCallbackFunc onError);
	//! Returns Rest result of the request.
	proto external ERestResult GetRestResult();
	//! Returns HTTP code result of the request.
	proto external HttpCode GetHttpCode();
	//! Returns raw data received via response as string.
	proto external string GetData();

	// callbacks

	//! OBSOLETE - WILL BE REMOVED! Use SetOnSuccess() to set callback function instead!
	[Obsolete("Use RestCallback.SetOnSuccess() instead.")]
	event void OnSuccess(string data, int dataSize);
	//! OBSOLETE - WILL BE REMOVED! Use SetOnError() to set callback function instead!
	[Obsolete("Use RestCallback.SetOnError() instead.")]
	event void OnError(int errorCode);
	//! OBSOLETE - WILL BE REMOVED! Use SetOnError() to set callback function instead and check GetRestResult() == ERestResult.EREST_ERROR_TIMEOUT!
	[Obsolete("Use RestCallback.SetOnError() instead.")]
	event void OnTimeout();
}

/*!
\}
*/
