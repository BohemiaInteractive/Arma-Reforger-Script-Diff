/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Backend error
enum EBackendError
{
	//! all OK
	EBERR_OK,
	//! unknown error
	EBERR_UNKNOWN,
	//! backend is disabled
	EBERR_DISABLED,
	//! called request from state where it is not possible (ie. reading data before login and such)
	EBERR_INVALID_STATE,
	//! no request can be called - login/auth in process
	EBERR_BUSY,
	//! state already requested once!
	EBERR_ALREADY_REQUESTED,
	//! failed to login
	EBERR_LOGIN_FAILED,
	//! failed to authenticate
	EBERR_AUTH_FAILED,
	//! login successful
	EBERR_LOGIN_SUCCESS,
	//! authenticate successful
	EBERR_AUTH_SUCCESS,
	//! configuration received
	EBERR_CONFIGURATION_GET,
	//! configuration written
	EBERR_CONFIGURATION_WRITE,
	//! character data received
	EBERR_CHARACTER_GET,
	//! character update done
	EBERR_CHARACTER_UPDATE,
	//! save point doesn't exist
	EBERR_FILE_NOT_FOUND,
	EBERR_LINK_FAILED,
	//! non-supported request call performed
	EBERR_UNSUPPORTED_REQUEST,
	//! unable to store data
	EBERR_STORAGE_IS_FULL,
	EBERR_VALIDATION_FAILED,
	EBERR_INVALID_INPUT,
}

/*!
\}
*/
