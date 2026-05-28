/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_workshop
\{
*/

//! Determines state in which WorkshopJob currently is in.
enum EWorkshopJobState
{
	//! Job is preparing to be active
	PREPARING,
	//! Job is currently actively processing its task
	ACTIVE,
	//! Job is queued and waiting for other ACTIVE task to finish
	QUEUED,
	//! Job is paused and will not continue until resumed
	PAUSED,
	//! Job was successfully finished
	FINISHED,
	//! Job was canceled and will not do any additional actions
	CANCELED,
	//! Job failed during its processing and can be retried
	FAILED,
}

/*!
\}
*/
