/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_UserGameData
\{
*/

sealed class UserGameDataApi
{
	private void UserGameDataApi();
	private void ~UserGameDataApi();

	/*!
	Returns own user game data of given type.
	\note This can return nullptr if no data have been requested yet.
	*/
	static proto UserGameDataBase GetOwnUserGameData(EUserGameDataType type);
	/*!
	Returns cached user game data matching both id and type from either container.
	\note Can return nullptr if no instance was found
	*/
	static proto UserGameDataBase FindUserGameData(UUID id, EUserGameDataType type);
	/*!
	Returns batch of user game data that was requested.
	\note This should be called after UserGameDataApi.RequestBatchUserGameData()
	*/
	static proto void GetUserGameDataBatch(out notnull array<UserGameDataBase> outData);
	/*!
	Returns server's user game data that was created when user joined the server.
	*/
	static proto void GetServerUserGameData(out notnull array<UserGameDataBase> outData);
	/*!
	Returns all cached user game data, combining both server and batch containers.
	Each unique instance is included only once even if present in both containers.
	*/
	static proto void GetAllUserGameData(out notnull array<UserGameDataBase> outData);
	/*!
	Request own user game data of given type and specified fields.
	\param callback Script callback where you will receive the result.
	\param type Type of user game data to request.
	\param fields Optional list of top-level data field names to retrieve (e.g. "randomNumber", "gamecode").
	       Each string must match a key used in the UserGameData JSON structure.
	       When empty, all fields are returned.
	*/
	static proto void RequestOwnUserGameData(notnull BackendCallback callback, EUserGameDataType type, array<string> fields);
	/*!
	Update own user game data.
	\param callback Script callback where you will receive the result
	\warning This is meant to be called only from single player game, therefore data of type SINGLE_PLAYER is updated.
	*/
	static proto void UpdateOwnUserGameData(notnull BackendCallback callback);
	/*!
	Patch own user game data. Only values that are different compared to the ones stored in the backend will be updated.
	\warning This is meant to be called only from single player game, therefore data of type SINGLE_PLAYER is updated.
	*/
	static proto void PatchOwnUserGameData(notnull BackendCallback callback);
	/*!
	Request batch of given users game data of specific type and fields.
	\param callback Script callback where you will receive the result.
	\param type Type of user game data to request.
	\param fields Optional list of top-level data field names to retrieve (e.g. "randomNumber", "gamecode").
				 Each string must match a key used in the UserGameData JSON structure.
				 When empty, all fields are returned.
	\param userIds List of users to return the data of
	\note If there are no data associated with any given user, the corresponding UserGameData.Status will be set to NOT_FOUND.

	*/
	static proto void RequestBatchUserGameData(notnull BackendCallback callback, EUserGameDataType type, array<string> fields, notnull array<UUID> userIds);
	/*!
	Update batch of users game data.
	\param callback Script callback where you will receive the result
	\param updateData Array of objects that will be used to update the data.
	\warning This is meant to be called only by game servers.
	*/
	static proto void UpdateBatchGameData(notnull BackendCallback callback, notnull array<UserGameDataBase> updateData);
	/*!
	Patch given batch of users game data.
	\param callback Script callback where you will receive the result
	\param patchData Array of objects to patch
	\warning This is meant to be called only by game servers.
	*/
	static proto void PatchGameData(notnull BackendCallback callback, notnull array<UserGameDataBase> patchData);
}

/*!
\}
*/
