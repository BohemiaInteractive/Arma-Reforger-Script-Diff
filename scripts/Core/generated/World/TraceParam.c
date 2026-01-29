/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup World
\{
*/

/*!
Input&Output structure for collision tests.
*/
class TraceParam: Managed
{
	//! Start position from where we trace
	vector Start;
	//! End position to where we trace
	vector End;
	//! Bitmask of layer(s) representing the traced object. The interaction matrix is used to determine whether an object can be traced.
	int LayerMask = 0xffffffff;
	//! Bitmask of layers to trace. Not used if 0, otherwise it overrides LayerMask.
	int TargetLayers = 0;
	TraceFlags Flags;
	//! Use either Exclude or ExcludeArray. Never both, it has some performance penalty
	IEntity Exclude;
	array<IEntity> ExcludeArray;
	//! Use either Include or IncludeArray. Never both, it has some performance penalty
	IEntity Include;
	//! Include only these entities
	array<IEntity> IncludeArray;
	IEntity TraceEnt; ///<[out] traced entity
	vector TraceNorm; ///<[out] traced polygon normal (X,Y,Z)
	float TraceDist; ///<[out] traced polygon plane distace
	int NodeIndex; ///<[out] bone associated with traced collider
	int ColliderIndex; ///<[out] trace collider index
	SurfaceProperties SurfaceProps; ///<[out] traced surface properties
	owned string TraceMaterial; ///<[out] traced surface material (usualy only on terrain)
	owned string ColliderName; ///<[out] traced collider name

	proto external bool IsRunning();
}

/*!
\}
*/
