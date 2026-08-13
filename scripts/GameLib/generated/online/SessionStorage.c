/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Save & Load handler
class SessionStorage
{
	private void SessionStorage();
	private void ~SessionStorage();

	/*!
	\brief Return true if storage is initialized - ready to load/ store data
	*/
	[Obsolete()]
	proto external bool Initialized();
	/*!
	\brief Clear all scheduled operations
	*/
	[Obsolete()]
	proto external void ClearScheduler();
	/*!
	\brief Request scheduler On/ Off without being removed from queue
	\param fileName - name of file handle
	\param bEnable - if should run or not (Note: by default it run so you typically pause it first)
	*/
	[Obsolete()]
	proto external void EnableScheduler( string fileName, bool bEnable );
	/*!
	\brief Request player save
	\param iPlayerId Is Player Id used on player identity
	*/
	[Obsolete()]
	proto external void RequestPlayerSave( int iPlayerId );;
	/*!
	\brief Request periodical processing save of session content
	\param fileName - name of file handle
	\param sec - time in seconds
	*/
	[Obsolete()]
	proto external void RequestScheduledSave( string fileName, float sec );
	/*!
	\brief Request server to process save of session content (can be invoked from script or game)
	\param fileName - name of file handle
	*/
	[Obsolete()]
	proto external void RequestSave( string fileName );
	/*!
	\brief Request server to process load of session content (can be invoked from script or game)
	\param fileName - name of file handle
	*/
	[Obsolete()]
	proto external void RequestLoad( string fileName );
	/*!
	\brief Request local save of session content (can be invoked from script or game)
	\param fileName - name of file handle
	*/
	[Obsolete()]
	proto external void LocalSave( string fileName );
	/*!
	\brief Request local delete of session content (can be invoked from script or game)
	\param fileName - name of file handle
	*/
	[Obsolete()]
	proto external void LocalLoad( string fileName );
	/*!
	\brief Request local delete of session content (can be invoked from script or game)
	\param fileName - name of file handle
	*/
	[Obsolete()]
	proto external void LocalDelete( string fileName );
	/*!
	\brief Check if file/ handle with thist name exist - local if you're local - online if you're online
	\param fileName - name of file handle
	*/
	[Obsolete()]
	proto external bool CheckFileID( string fileName );
	/*!
	\brief Assign callback for handling Save & Load events for specific file handle under session
	\param fileName - name of file handle
	\param sessionCallback - session callback
	*/
	[Obsolete()]
	proto external void AssignFileIDCallback( string fileName, DSSessionCallback sessionCallback );
	/*!
	\brief Get a list of all save files that are ready to load
	*/
	[Obsolete()]
	proto external int AvailableSaves( out notnull array<string> aSaves );
	/*!
	\brief Process session load - You call this method from OnLoad() event of callback.
	\param pDataObject Represents "master" object as targer for incoming data
	*/
	[Obsolete()]
	proto external void ProcessLoad( JsonApiStruct pDataObject, string fileName );
	/*!
	\brief Process session save - You call this method from OnSave() event of callback.
	\param pDataObject Represents "master" object as source of outcoming data
	*/
	[Obsolete()]
	proto external void ProcessSave( JsonApiStruct pDataObject, string fileName );
	/*!
	\brief Check if online storage privileges are granted (if not - all is stored locally with session)
	*/
	[Obsolete()]
	proto external bool GetOnlineWritePrivilege();
}

/*!
\}
*/
