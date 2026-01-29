/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Resources
\{
*/

sealed class BaseResourceObject: global_pointer
{
	proto external BaseContainer ToBaseContainer();
	proto external IEntitySource ToEntitySource();
	proto external MeshObject ToMeshObject();
	proto external AnimationSource ToAnimation();
	proto external VObject ToVObject();
	/*!
	Release object. When there are not any other references, object is stored into cache and ready to be victed if necessary.
	\param flush If force is true, the object is flushed from cache after releasing
	*/
	proto external void Release(bool flush = false);
	//! Returns name of visual object
	proto external ResourceName GetResourceName();
}

/*!
\}
*/
