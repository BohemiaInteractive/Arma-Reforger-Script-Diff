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
A single ban record returned by the backend. Contains the target of the ban, who issued
it, its scope (which system and action it restricts), the reason, and timing information.

Obtain instances from ServerBanManagerApi (ban listing) or ServerJoinManagerApi
(join refused due to ban).
*/
sealed class BanEntry: Managed
{
	private void BanEntry();
	private void ~BanEntry();

	//! Returns the scope of this ban (the system and action it restricts).
	proto external BanScope GetScope();
	//! Returns the entity targeted by this ban.
	proto external BanTargetBase GetTarget();
	//! Returns the entity which issued this ban.
	proto external BanIssuerBase GetIssuer();
	//! Returns the reason category for this ban.
	proto external EBanReasonType GetReasonType();
	//! Returns the custom human-readable reason message provided by the issuer when the ban was created.
	proto external string GetReasonMessage();
	//! Returns true when this ban has no expiry date.
	proto external bool IsPermanent();
	//! Returns the time this ban was created.
	proto external Timestamp GetCreatedAt();
	/*!
	Returns the time this ban expires.
	\note Only valid when IsPermanent() returns false.
	*/
	proto external Timestamp GetExpiresAt();
}

/*!
\}
*/
