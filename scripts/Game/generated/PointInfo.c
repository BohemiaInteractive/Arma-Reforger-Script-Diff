/*
===========================================
Do not modify, this script is generated
===========================================
*/

//! PointInfo - declare object, allows to define position (before named ComponentPositionInfo)
class PointInfo: ScriptAndConfig
{
	void PointInfo();

	proto external void Init(IEntity owner);
	proto external IEntity GetOwner();
	proto external void Set(IEntity owner, string boneName, vector matInOwnerSpace[4]);
	proto external void GetTransform(out vector outMat[4]);
	proto external void GetModelTransform(out vector outMat[4]);
	//! If the owner is not specified, the local transform will be equal to the world transform
	proto external void GetWorldTransform(out vector outMat[4]);
	proto external void GetLocalTransform(out vector outMat[4]);
	proto external vector GetTransformAxis(int axis);
	proto external vector GetWorldTransformAxis(int axis);
	proto external vector GetModelTransformAxis(int axis);
	proto external vector GetLocalTransformAxis(int axis);
	proto external void GetTransformNode(out vector outMat[4]);
	proto external string GetBoneName();
	proto external int GetNodeId();
}
