/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class BaseGameEntityClass: GenericEntityClass
{
}

class BaseGameEntity: GenericEntity
{
	/*!
	Teleport the entity to a coordinate
	\param mat The matrix used for the teleportation
	*/
	proto external void Teleport(vector mat[4]);
	//! Returns the replication component associated to this entity
	proto external RplComponent GetRplComponent();
}

/*!
\}
*/
