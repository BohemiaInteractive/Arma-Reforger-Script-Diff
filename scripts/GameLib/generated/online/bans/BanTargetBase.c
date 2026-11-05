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
Base class for all ban target types. Call GetType() to determine the concrete subtype
and cast accordingly to access target-specific fields.
*/
sealed class BanTargetBase: Managed
{
	private void BanTargetBase();
	private void ~BanTargetBase();

	//! Returns the target type; cast the instance to the matching subclass to access its fields.
	proto external EBanTargetType GetType();
}

/*!
\}
*/
