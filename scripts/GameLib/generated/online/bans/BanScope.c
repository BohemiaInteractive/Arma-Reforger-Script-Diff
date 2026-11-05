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
Defines the reach of a ban — which system it applies to (e.g. server, workshop) and
which action it restricts (e.g. join, publish). Use GetSystem() and GetAction() to
determine what a player is prevented from doing.
*/
sealed class BanScope: Managed
{
	private void BanScope();
	private void ~BanScope();

	//! Returns true if this scope applies globally across all projects; false if it is limited to this game project.
	proto external bool IsProjectGlobal();
	//! Returns the system this ban scope applies to.
	proto external EBanScopeSystem GetSystem();
	//! Returns the action this ban scope restricts.
	proto external EBanScopeAction GetAction();
}

/*!
\}
*/
