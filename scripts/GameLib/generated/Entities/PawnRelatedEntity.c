/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class PawnRelatedEntity: GenericEntity
{
	event protected void OnAddedToPawn(PawnEntity pawnOwner);
	event protected void OnRemovedFromPawn(PawnEntity pawnOwner);
	event protected void OnFramePhaseMainLogic(PawnEntity pawnOwner, float timeSlice);
	event protected void OnFramePhasePreAnim(PawnEntity pawnOwner, float timeSlice);
	event protected void OnFramePhasePostAnim(PawnEntity pawnOwner, float timeSlice);
}

/*!
\}
*/
