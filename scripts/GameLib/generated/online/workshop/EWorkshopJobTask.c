/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_workshop
\{
*/

//! Determines what task is WorkshopJob supposed to do.
enum EWorkshopJobTask
{
	//! Job is downloading revision without any other revision of the asset present - fresh download
	DOWNLOAD,
	//! Some revision is already present and job is downloading newer one.
	UPDATE,
	//! Some revision is already present and job is downloading older one.
	DOWNGRADE,
	//! Deep validation against the local manifest header - detects corrupted/missing fragments without a connection.
	VALIDATE_OFFLINE,
	//! Deep validation against backend-authoritative manifest data - re-fetches manifest metadata.
	VALIDATE_ONLINE,
	//! Job is fixing corrupted fragments
	REPAIR,
}

/*!
\}
*/
