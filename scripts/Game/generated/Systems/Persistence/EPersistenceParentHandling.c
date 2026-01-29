/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence
\{
*/

sealed enum EPersistenceParentHandling
{
	//! No root record if any parent is present.
	ACCEPT,
	//! Ignore all parents that are not tracked by persistence.
	IGNORE_UNTRACKED,
	//! Ignore if any parent is a loaded world entity (even if they are tracked).
	IGNORE_LOADED,
	//! Ignore any parent and always create root record.
	IGNORE,
}

/*!
\}
*/
