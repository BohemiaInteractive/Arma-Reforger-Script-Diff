/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Data structure for block list response
sealed class BlockListItem: Managed
{
	private void BlockListItem();
	private void ~BlockListItem();

	//! Get identity UUID of blocked user
	proto external string GetId();
	//! Get name of blocked user
	proto external string GetName();
	//! Get platform of blocked user. Will return NONE if platform could not be resolved.
	proto external PlatformKind GetPlatform();
	/*
	Delete user from block list.
	\param pCallback Is BackendCallback where you will receive result/error when request finishes
	*/
	proto external void DeleteBlock(BackendCallback pCallback);
}

/*!
\}
*/
