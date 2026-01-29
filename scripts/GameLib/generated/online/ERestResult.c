/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! States and result + error code produced by RestApi.
enum ERestResult
{
	//! not initialized
	EREST_EMPTY,
	//! result and/or data are ready (success)
	EREST_SUCCESS,
	//! (state >= EREST_ERROR) == error happened
	EREST_ERROR,
	EREST_ERROR_SERVERERROR,
	EREST_ERROR_APPERROR,
	EREST_ERROR_TIMEOUT,
	EREST_ERROR_NOTIMPLEMENTED,
	//! packing request to JSON failed (too much data?)
	EREST_ERROR_PACKINGREQUEST,
	//! failed to even create REST request (send failed)
	EREST_ERROR_CREATE,
	//! failed to receive response
	EREST_ERROR_RECV,
	//! failed to send request
	EREST_ERROR_SEND,
	//! unable to store received data (not enough space?)
	EREST_ERROR_STORE,
	EREST_ERROR_UNKNOWN,
}

/*!
\}
*/
