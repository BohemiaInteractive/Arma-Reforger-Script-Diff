/*
===========================================
Do not modify, this script is generated
===========================================
*/

enum DataError
{
	//! Game version used by client does not match versions supported by server.
	VERSION_MISMATCH,
	//! Resource database contents differ in significant way between client and server.
	RDB_MISMATCH,
	//! Scripts differ between client and server.
	SCRIPT_MISMATCH,
	//! World could not be loaded.
	WORLD_LOAD_ERROR,
	//! Replicated parts of world do not match between client and server.
	WORLD_LOAD_INCONSISTENCY,
	//! Addon loading failed
	ADDON_LOAD_ERROR,
	//! Attempting to create connection between developer and non-developer binaries.
	IS_DEV_BINARY_MISMATCH,
}
