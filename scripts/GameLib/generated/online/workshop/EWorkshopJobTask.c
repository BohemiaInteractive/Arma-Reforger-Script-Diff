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
	//! Job is doing deep validation where it attempts to find if any fragment is corrupted
	VALIDATE,
	//! Job is fixing corrupted fragments
	REPAIR,
}

/*!
\}
*/
