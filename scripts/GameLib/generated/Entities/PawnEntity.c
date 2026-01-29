/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class PawnEntity: GenericEntity
{
	proto external void EnableSimulation(bool state);
	proto external bool IsSimulated();
	proto external bool IsPossessed();
	proto external RplIdentity GetOwnerRplIdentity();
	proto external PawnEntityController GetController();
	proto external PawnMovementComponent GetPawnMovementComponent();
	proto external BaseRplComponent GetRplComponent();
	proto external bool IsLocal();
	proto external bool IsAuthority();
	proto external bool IsControlledAuthority();
	proto external bool IsControlledProxy();
	proto external bool IsControlledMaster();
	proto external bool IsSimulatedProxy();
	proto external bool IsSimulatedAuthority();
	proto external bool IsOwner();

	// callbacks

	event protected void OnFramePhaseMainLogic(float timeSlice);
	event protected void OnFramePhasePreAnim(float timeSlice);
	event protected void OnFramePhasePostAnim(float timeSlice);
	event protected void OnPossession(RplIdentity rplIdentity);
	event protected void OnDispossession();
	event protected void OnSimulationStateChanged(bool state);
}

/*!
\}
*/
