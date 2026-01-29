/*
===========================================
Do not modify, this script is generated
===========================================
*/

sealed class AttachPoint: ScriptAndConfig
{
	private void AttachPoint();

	proto external void Set(string boneName, vector offsetInBoneSpace[4]);
	proto external void GetLocalTransform(notnull IEntity owner, out vector outMat[4]);
	//! Axis should be [0..3], vector.Zero will be returned otherwise
	proto external vector GetLocalTransformAxis(notnull IEntity owner, int axis);
	proto external void GetWorldTransform(notnull IEntity owner, out vector outMat[4]);
	//! Axis should be [0..3], vector.Zero will be returned otherwise
	proto external vector GetWorldTransformAxis(notnull IEntity owner, int axis);
}
