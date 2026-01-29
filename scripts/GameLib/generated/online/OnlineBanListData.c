/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Structure for ban data response of BanList requests
sealed class OnlineBanListData: Managed
{
	// Identity ID of player
	string identityId
	// Name of player when he was banned
	string bannedName;
	// Current name of player
	string currentName;
	// Reason of ban
	string reason;
	// Timestamp of when ban was created
	int createdAt;
	// Timestamp of when ban will expire (0 if never - permanent ban)
	int expiresAt;

}

/*!
\}
*/
