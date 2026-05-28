/*
===========================================
Do not modify, this script is generated
===========================================
*/

class PawnEntityController: ScriptAndConfig
{
	protected void PawnEntityController(PawnEntity pawnOwner);

	proto external PawnEntity GetPawnOwner();
	proto external void SetFlags(int flags);
	proto external void ClearFlags(int flags);
	proto external bool IsFlagSet(int flag);
	// movement input
	proto external void AddMovementInput(vector movementInput);
	proto external vector GetCurrentMovementInput();
	proto external vector GetLastMovementInput();
	// rotations in radians in world space
	proto external void SetControlRotation(vector ypr);
	proto external vector GetControlRotation();
	proto external void SetAimRotation(vector ypr);
	proto external vector GetAimRotation();
	proto external void SetLookRotation(vector ypr);
	proto external vector GetLookRotation();
	static proto vector GetCompressedAngles(vector rotation);

	// callbacks

	event protected void OnInit(PawnEntity owner);
	event protected void OnPrepare(PawnEntity owner, float timeSlice);
	event protected void OnUpdate(PawnEntity owner, float timeSlice);
	event protected int OnPackControls(PawnEntity owner);
	event protected void OnUnpackControls(PawnEntity owner, int packedControls);
	event protected bool OnPackUserActions(PawnEntity owner, ScriptBitWriter bitWriter);
	event protected bool OnUnpackUserActions(PawnEntity owner, ScriptBitReader bitReader);
}
