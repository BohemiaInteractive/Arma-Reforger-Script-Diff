/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online_workshop
\{
*/

/*!
Specifies possible states of locally available revision of this asset.
*/
sealed enum EWorkshopAssetState
{
	//! No revision is locally available/downloaded and not subscribed (cannot download until subscribed).
	NONE,
	//! User has this asset blocked so it should not be possible to interact with it until unblocked.
	BLOCKED,
	//! Asset is subscribed but no revision is downloaded (waiting for request to begin download).
	SUBSCRIBED,
	//! Asset is disabled and should not load any data until until enabled.
	DISABLED,
	//! Asset is marked as enabled and its data should be loaded when possible.
	ENABLED,
	//! Asset is loaded and some operations should not be possible in this state.
	LOADED,
	//! This asset has currently pending job and cannot be used until finished.
	PENDING_JOB,
	//! Current revision is missing some dependencies and will not be allowed to be used (ENABLED and LOADED) until fixed.
	MISSING_DEPENDENCIES,
	//! Current revision is corrupted and cannot be used (ENABLED and LOADED) until fixed.
	CORRUPTED,
	//! Asset has some data downloaded but unknown to which revision - cannot be repaired.
	CORRUPTED_UNKNOWN,
}

/*!
\}
*/
