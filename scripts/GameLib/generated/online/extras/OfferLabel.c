/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_extras
\{
*/

//! A single label assigned to a ProductOffer, valid within a given time window.
class OfferLabel: Managed
{
	private void OfferLabel();
	private void ~OfferLabel();

	//! Returns the display name of this label
	proto external string GetName();
	//! Returns the date-time string from which this label applies.
	proto external string GetStartStr();
	//! Returns the date-time string until which this label applies, or empty string if open-ended.
	proto external string GetEndStr();
}

/*!
\}
*/
