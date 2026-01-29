/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Resources
\{
*/

/*!
\brief Object holding reference to resource. In destructor release the resource
@code
// load
Resource resource = Resource.Load(prefabResourceName);
if (resource.IsValid())
{
	IEntity rootBall = GetGame().SpawnEntityPrefab(resource);
}
// At the end of scope is resource released.
@endcode
*/
class Resource: Managed
{
	private void Resource();

	proto external BaseResourceObject GetResource();
	proto external bool IsValid();
	//!Loads object from data, or gets it from cache.
	static proto ref Resource Load(ResourceName name);
}

/*!
\}
*/
