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
Base class for all ban issuer types. Call GetType() to determine the concrete subtype
and cast accordingly to access issuer-specific fields.
*/
sealed class BanIssuerBase: Managed
{
	private void BanIssuerBase();
	private void ~BanIssuerBase();

	/*!
	Returns the issuer type.
	Cast the instance to the matching subclass to access its fields.
	\note type NONE cannot be casted and represents the BanIssuerBase.
	*/
	proto external EBanIssuerType GetType();
}

/*!
\}
*/
