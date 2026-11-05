/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_bans
\{
*/

/*!
Contains the per-ban results of a RequestRemoveBan call — one entry per requested ban id.
Retrieve this object after the callback fires via ServerBanManagerApi.GetLastRemoveResult().
Use GetErrorsCount() to quickly check whether any removal failed, then iterate
GetItems() for per-ban details.
*/
sealed class BanRemoveResult: Managed
{
	private void BanRemoveResult();
	private void ~BanRemoveResult();

	//! Total number of items in the result (one per requested ban id).
	proto external int GetCount();
	//! Number of items whose result is not EACODE_ERROR_OK.
	proto external int GetErrorsCount();
	//! Fills the output array with all per-item results.
	proto external void GetItems(out notnull array<BanRemoveResultItem> itemsOut);
}

/*!
\}
*/
