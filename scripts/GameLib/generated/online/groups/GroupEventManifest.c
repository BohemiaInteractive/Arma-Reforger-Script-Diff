/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_groups
\{
*/

//! Data needed to create a new Group event.
sealed class GroupEventManifest: Managed
{
	proto external void SetTitle(string sTitle);
	proto external void SetDescription(string sDescription);
	//! Must fall between 2000-01-01 and 2100-01-01.
	proto external void SetStartTimestamp(Timestamp startTimestamp);
	//! Must fall between 2000-01-01 and 2100-01-01.
	proto external void SetEndTimestamp(Timestamp endTimestamp);
	proto external void SetMaxParticipants(int iMaxParticipants);
	proto external string GetTitle();
	proto external string GetDescription();
	proto external Timestamp GetStartTimestamp();
	proto external Timestamp GetEndTimestamp();
	proto external int GetMaxParticipants();
}

/*!
\}
*/
