/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Visual
\{
*/

sealed class Animation: pointer
{
	proto external void	SetBone(IEntity ent, TNodeId bone, vector angles, vector trans, float scale);
	proto external bool	SetBoneMatrix(IEntity ent, TNodeId bone, vector mat[4]);
	proto external bool	GetBoneMatrix(TNodeId bone, out vector mat[4]);
	proto external TNodeId	GetBoneIndex(string boneName);
	proto external void	GetBoneNames(out notnull array<string> boneNames);
	proto external bool	GetBoneLocalMatrix(TNodeId bone, out vector mat[4]);
	proto external bool	SetMorphState(string name, float value);
	proto external TMeshIndex	GetNamedMeshIndex(string meshName);
	proto external bool	ShowMesh(TMeshIndex namedMeshIndex, bool show);
}

/*!
\}
*/
