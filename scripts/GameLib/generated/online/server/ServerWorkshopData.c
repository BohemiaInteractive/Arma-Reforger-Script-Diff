/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_server
\{
*/

/*!
Data structure for integrating workshop with servers.
It is created for servers which are running with any assets from workshop.

If server is not running with any assets (is Vanilla) then this object will not be created for such server.
*/
sealed class ServerWorkshopData
{
	private void ServerWorkshopData();
	private void ~ServerWorkshopData();

	//! Returns owner server of these workshop data.
	proto external ServerInfo GetOwner();
	//! Returns count of how many workshop assets is server using.
	proto external int GetAssetCount();
	/*!
	Will request missing partial details about assets and revisions in collection.
	\warning This must be requested after full server details.
	         It must be also requested to fully generate collection which will otherwise return null.
	\throw VME - If full server details were not yet requested and loaded.
	           - If request is already being processed.
	*/
	proto external void RequestAssetData(notnull BackendCallback callback);
	/*!
	Returns structure of workshop collection containing all data about individual assets.
	\warning If this returns null then full details about server or its assets and revisions were not yet fetched.
	*/
	proto external WorkshopCollection GetCollection();
}

/*!
\}
*/
