/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Utilities
\{
*/

sealed class MeshObjectVolumeCalculator: ScriptAndConfig
{
	/*!
	* \brief Calculate volume of given mesh of meshObject using colliders with given interaction layer.
	* Colliders should be closed as not closed could return imprecise result.
	* Doesn't support scaled objects.
	* \param ent Entity that is volume calculated for.
	* \param layer Iteraction layer of object to calculate it from.
	* \return Volume of given object, or 0 without object or on error
	*/
	static proto float GetVolumeFromColliders(IEntity ent, int layer);
	/*!
	* \brief Calculate volume of given mesh of meshObject. Mesh should be closed as not closed meshes will return imprecise result.
	* Doesn't support scaled objects.
	* \param vObject Object that is volume calculated for.
	* \param meshId Id of mesh from vObject to calculate.
	* \return Volume of given object, or 0 without object
	*/
	static proto float GetVolumeFromVisualMesh(VObject vObject, int meshId);
}

/*!
\}
*/
