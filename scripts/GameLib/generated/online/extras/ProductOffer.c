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
	proto external string GetLabel();
	proto external string GetSaleStartStr();
	proto external string GetSaleEndStr();
	proto external EOfferPlatformFlags GetAllowedPlatforms();
	proto external ExtraProduct GetProduct();
}

/*!
\}
*/
