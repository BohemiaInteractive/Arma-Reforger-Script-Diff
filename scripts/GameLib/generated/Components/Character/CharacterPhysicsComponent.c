/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components_Character
\{
*/

/*!
CharacterPhysicsComponent isn't a managed instance, don't store references in script, where the lifetime of the reference can be longer than the lifetime of this component.
*/
sealed class CharacterPhysicsComponent: NativeComponent
{
	/*
	* Requests change of ragdoll state. The ragdoll state only changes at the last phase of the pawn simulation.
	*/
	proto external void EnableRagdoll(bool state);
	/*
	* Returns true if the component is actively ragdolling.
	*/
	proto external bool IsRagdollEnabled();
	/*
	* Returns true if at least one bone of the ragdoll has higher velocity than given treshold.
	*/
	proto external bool AreRagdollBonesMoving(float fBoneVelocityTreshold);
	/*
	* Returns true if root bone of the ragdoll has higher velocity than given treshold.
	*/
	proto external bool IsRagdollRootMoving(float fVelocityTreshold);
	proto external void EnableCollisionResponse(bool state);
}

/*!
\}
*/
