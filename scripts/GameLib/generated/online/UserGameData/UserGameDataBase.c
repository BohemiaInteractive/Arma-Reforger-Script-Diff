/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_UserGameData
\{
*/

sealed class UserGameDataBase: Managed
{
	private void UserGameDataBase();
	private void ~UserGameDataBase();

	//! Returns type of the data
	proto external EUserGameDataType GetDataType();
	//! Returns status of the data
	proto external EUserGameDataStatus GetStatus();
	//! Returns ID of the data
	proto external string GetId();
	/*!
	Returns whether the caller has read-write access to this entry.
	\note For GetOwn responses this is always true.
	      For GetBatch responses on a client this is true only for
	      the entry whose ID matches the caller's game identity.
	      For GetBatch responses on a server this is true only for
	      entries whose type matches the server type (OFFICIAL or COMMUNITY).
	*/
	proto external bool IsReadWrite();
}

/*!
\}
*/
