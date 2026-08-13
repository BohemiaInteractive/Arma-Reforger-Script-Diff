/*
===========================================
Do not modify, this script is generated
===========================================
*/

sealed class PawnEntitySystem: WorldSystem
{
	proto external void PossessPawn(notnull PawnEntity pawn, RplIdentity identity);
	proto external void DispossessPawn(notnull PawnEntity pawn);

	// callbacks

	[EventAttribute()]
	event void OnPawnEntitySpawned(notnull PawnEntity pawnEntity);
	[EventAttribute()]
	event void OnPawnEntityDespawn(notnull PawnEntity pawnEntity);
	[EventAttribute()]
	event void OnPawnEntityPossessed(notnull PawnEntity pawnEntity);
	[EventAttribute()]
	event void OnPawnEntityDispossessed(notnull PawnEntity pawnEntity);
}
