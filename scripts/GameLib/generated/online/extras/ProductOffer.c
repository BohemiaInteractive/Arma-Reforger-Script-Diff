/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_extras
\{
*/

class ProductOffer: Managed
{
	private void ProductOffer();
	private void ~ProductOffer();

	proto external bool IsActive();
	proto external int GetSortOrder();
	proto external void GetLabels(out notnull array<OfferLabel> outLabels);
	proto external string GetSaleStartStr();
	proto external string GetSaleEndStr();
	//! Returns the time when this offer becomes valid
	proto external Timestamp GetValidFrom();
	//! Returns the time when this offer stops being valid
	proto external Timestamp GetValidTo();
	proto external EOfferPlatformFlags GetAllowedPlatforms();
	proto external ExtraProduct GetProduct();
}

/*!
\}
*/
