/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

/*!
Shared object with constant data for enf::EntitySource from prefab. For example:
- material remapping for visual objects
- physics geometries for physics controller
- etc.
*/
class EntityPrefabData
{
	proto external ResourceName GetPrefabName();
	proto external BaseContainer GetPrefab();
	//!Find component data container of given type
	proto external EntityComponentPrefabData FindComponentData(typename typeName);
}

/*!
\}
*/
