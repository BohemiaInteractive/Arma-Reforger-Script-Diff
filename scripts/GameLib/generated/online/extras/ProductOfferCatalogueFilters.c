/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_extras
\{
*/

class ProductOfferCatalogueFilters: Managed
{
	/*!
	Sets product ID filter.
	Only offers for the given product will be returned.
	Pass -1 to clear the filter.
	*/
	proto external void SetProductId(int productId);
	//! Returns current product ID filter, or -1 if not set.
	proto external int GetProductId();
	/*!
	Sets platform filter.
	Only offers available on the given platform will be returned.
	Pass NONE to clear the filter.
	*/
	proto external void SetPlatform(EOfferPlatform platform);
	//! Returns current platform filter, or EOFP_NONE if not set.
	proto external EOfferPlatform GetPlatform();
}

/*!
\}
*/
