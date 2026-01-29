/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup World
\{
*/

enum EQueryEntitiesFlags
{
	//!Include entities with enf::EntityFlags.TFL_STATIC
	STATIC,
	//!Include entities without enf::EntityFlags.TFL_STATIC
	DYNAMIC,
	//!Include entities with enf::EntityFlags.TFL_FEATURE (usually large landscape parts as lakes, fake horizons etc)
	FEATURES,
	//!Include only entities that has some VObjectComponent (Mesh, Particle etc)
	WITH_OBJECT,
	//!Do not include children entities with enf::EntityFlags.TFL_PROXY
	NO_PROXIES,
	ALL,
}

/*!
\}
*/
