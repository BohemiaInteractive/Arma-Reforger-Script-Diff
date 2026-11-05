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
Target the backend could not map to a known kind. Carries the raw identifier and
name if any, so the UI can still show what was banned.
*/
sealed class BanTargetUnknown: BanTargetBase
{
	//! Returns the raw identifier of this target.
	proto external string GetId();
	//! Returns name of this target at the time when ban was created.
	proto external string GetName();
	//! Returns current name of this target.
	proto external string GetCurrentName();
}

/*!
\}
*/
