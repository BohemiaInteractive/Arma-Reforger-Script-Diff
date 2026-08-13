/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Type of value stored in a value filter.
sealed enum EBackendValueFilterType
{
	NONE,
	BOOL,
	INT,
	FLOAT,
	STRING,
	//! Enum backed by a C++-side static string table referenced in path info.
	//! Stores both int (for comparison) and string (for JSON serialization).
	ENUM_STATIC,
}

/*!
\}
*/
