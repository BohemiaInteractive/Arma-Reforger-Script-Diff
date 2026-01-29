/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! JsonApiStruct passed to BanList backend requests
sealed class BanListPageParams: JsonApiStruct
{
	// how many bans will be on one page
	int limit;
	// offset for ban list - to show different page offset by: limit * pageNumber
	int offset;
	// name of player to be used as filter for ban list (optional)
	string playerName;
	// how should player name be searched/matched in database
	EStringMatchType playerNameMatchType;

}

/*!
\}
*/
