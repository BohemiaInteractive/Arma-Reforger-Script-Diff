/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class GenericWorldEntityClass: GenericEntityClass
{
}

class GenericWorldEntity: GenericEntity
{
	/*!
	Get terrain
	*/
	proto external GenericTerrainEntity GetTerrain(float x, float z);
	proto external ref Material GetSkyMaterial();
}

/*!
\}
*/
