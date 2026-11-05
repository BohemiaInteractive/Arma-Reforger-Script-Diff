/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

//! Result for a single ban in a RequestRemoveBan call.
sealed class BanRemoveResultItem: Managed
{
	private void BanRemoveResultItem();
	private void ~BanRemoveResultItem();

	/*!
	Result code for this ban.
	- EApiCode.EACODE_ERROR_OK — removed successfully.
	- EApiCode.EACODE_ERROR_NOT_FOUND — no ban with this id exists.
	- EApiCode.EACODE_ERROR_UNKNOWN — the ban exists but could not be removed (missing write permission, or server error).
	*/
	proto external EApiCode GetResult();
	//! Returns entry to which this result belongs to.
	proto external BanEntry GetEntry();
}

/*!
\}
*/
