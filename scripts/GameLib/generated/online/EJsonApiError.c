/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//!-----------------------------------------------------------------------------
//! error codes for handle processing
//! defined in C++
enum EJsonApiError
{
	//! invalid code
	ETJSON_UNKNOWN,
	//! all fine
	ETJSON_OK,
	//! error during send
	ETJSON_COMMSEND,
	//! error during parsing
	ETJSON_PARSERERROR,
	//! failed to send/ store handle due to timeout
	ETJSON_TIMEOUT,
	//! not enough buffers available
	ETJSON_NOBUFFERS,
	//! failed to load file
	ETJSON_FAILFILELOAD,
	//! failed to save file
	ETJSON_FAILFILESAVE,
	//! object is not array (ie. attempt to provide different or none object as array)
	ETJSON_NOTARRAY,
}

/*!
\}
*/
