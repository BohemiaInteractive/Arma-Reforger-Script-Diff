/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
Read-only wrapper for GenericEntity::OnTransformResetImpl
and GenericComponent::OnTransformReset parameters.
*/
sealed class TransformResetParams: pointer
{
	/*!
	Gets initial velocity of the owner entity after the transform reset.

	Useful e.g. when velocity is computed using previous position which is invalid after transform reset.
	*/
	proto external vector GetNewVelocity();
	/*!
	Gets a hint whether the transform was reset due to its correction (e.g. by net-code),
	i.e. not a placement / teleport.
	*/
	proto external bool IsCorrection();
}
