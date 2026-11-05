/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

sealed class BanList: Managed
{
	private void BanList();
	private void ~BanList();

	//! Unique name identifying this ban list.
	proto external string GetId();
	//! Returns true when the ban list is publicly visible.
	proto external bool IsPublic();
	//! Returns true when this server has write permission on the ban list and can create bans into it.
	proto external bool IsWritable();
}

/*!
\}
*/
