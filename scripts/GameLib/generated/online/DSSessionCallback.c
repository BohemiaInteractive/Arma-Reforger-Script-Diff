/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Dedicated Server Session callback class for script
class DSSessionCallback: Managed
{
	/*!
	\brief Session connect event
	*/
	event void OnConnect();
	/*!
	\brief Session disconnect event
	*/
	event void OnDisconnect();
	/*!
	\brief Save event handling
	*/
	event void OnSaving( string fileName );
	/*!
	\brief Load event handling
	*/
	event void OnLoaded( string fileName );
	/*!
	\brief Setup event handling
	*/
	event void OnSetup( string fileName );
	/*!
	\brief Event when timed player saving is about to happen
	\param iPlayerId - Id of Player which will be saved
	*/
	event void OnPlayerSaveEvent( int iPlayerId );
	/*!
	\brief Load Fail event handling
	*/
	event void OnLoadFailed( string fileName );
	/*!
	\brief Save Fail event handling
	*/
	event void OnSaveFailed( string fileName );
	/*!
	\brief Save Success event handling
	*/
	event void OnSaveSuccess( string fileName );
	/*!
	\brief Delete Fail event handling
	*/
	event void OnDeleteFailed( string fileName );
	/*!
	\brief Delete Success event handling
	*/
	event void OnDeleteSuccess( string fileName );
	/*!
	\brief Initialize event - here specify what you want to load before game starts
	*/
	event void OnInitialize();
	/*!
	\brief Initializing new session
	*/
	event void OnNew();
	/*!
	\brief Ready event handling - point where session goes to game
	*/
	event void OnReady();
}

/*!
\}
*/
