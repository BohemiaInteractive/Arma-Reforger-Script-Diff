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
	[Obsolete()]
	event void OnSetup( string fileName );
	[Obsolete()]
	event void OnNew();

	/*!
	\brief Session connect event
	*/
	[Obsolete()]
	event void OnConnect();
	/*!
	\brief Session disconnect event
	*/
	[Obsolete()]
	event void OnDisconnect();
	/*!
	\brief Save event handling
	*/
	[Obsolete()]
	event void OnSaving( string fileName );
	/*!
	\brief Load event handling
	*/
	[Obsolete()]
	event void OnLoaded( string fileName );
	/*!
	\brief Event when timed player saving is about to happen
	\param iPlayerId - Id of Player which will be saved
	*/
	[Obsolete()]
	event void OnPlayerSaveEvent( int iPlayerId );
	/*!
	\brief Load Fail event handling
	*/
	[Obsolete()]
	event void OnLoadFailed( string fileName );
	/*!
	\brief Save Fail event handling
	*/
	[Obsolete()]
	event void OnSaveFailed( string fileName );
	/*!
	\brief Save Success event handling
	*/
	[Obsolete()]
	event void OnSaveSuccess( string fileName );
	/*!
	\brief Delete Fail event handling
	*/
	[Obsolete()]
	event void OnDeleteFailed( string fileName );
	/*!
	\brief Delete Success event handling
	*/
	[Obsolete()]
	event void OnDeleteSuccess( string fileName );
	/*!
	\brief Initialize event - here specify what you want to load before game starts
	*/
	[Obsolete()]
	event void OnInitialize();
	/*!
	\brief Ready event handling - point where session goes to game
	*/
	[Obsolete()]
	event void OnReady();
}

/*!
\}
*/
