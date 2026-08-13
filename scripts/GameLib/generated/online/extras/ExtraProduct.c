/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_extras
\{
*/

class ExtraProduct: Managed
{
	private void ExtraProduct();
	private void ~ExtraProduct();

	proto external EProductCategory GetCategory();
	//! Returns platforms this product can be found on
	proto external EOfferPlatformFlags GetPlatforms();
	//! Return Server renting details, or null if the category is not SERVER_RENTING.
	proto external ProductServerRentingDetails GetServerRentingDetails();
	proto external string GetCreatedAtStr();
	proto external string GetUpdatedAtStr();
}

/*!
\}
*/
