/*
===========================================
Do not modify, this script is generated
===========================================
*/

enum EServerCmdResultType
{
	//! Normal execution
	OK,
	//! No command using given keyword was found
	UNKNOWN_COMMAND,
	//! Insufficient permission/role
	MISSING_PERMISSION,
	//! Error among parameters (e.g.: not enough params, wrong name/id)
	PARAMETERS,
	//! Response is not ready yet.
	PENDING,
	//! The command is already running.
	BUSY,
	//! General error not covedred by previous categories
	ERR,
}
