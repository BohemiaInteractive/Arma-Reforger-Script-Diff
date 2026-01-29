/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

//! Various entity flags.
enum EntityFlags
{
	/*!
	Entity is visible.\ Is rendered if there is some attached object
	and event EV_VISIBLE is invoked, when event mask is set.
	*/
	VISIBLE,
	//! Entity can be detected by various trace methods
	TRACEABLE,
	//! Never link this entity into the scene
	NO_LINK,
	//! Scene rendering hint for dominant objects that are not culled by standard way
	FEATURE,
	//! You can check whether the entity has been rendered by enf::Entity::IsRendered()
	SET_RENDERED,
	STREAMABLE,
	//! Tells that this entity has to be actively updated by engine
	ACTIVE,
	/*!
	Tells that this entity will represent mostly static object, so
	we can use more precise but slower methods for scene-tree linking.\
	Also it tells to scene tree that he can count with this entity as
	tree-split hint.
	*/
	STATIC,
	//! Flags for custom usage and filterings.
	USER1,
	USER2,
	USER3,
	USER4,
	USER5,
	//! Do not include this entity into BSP tree building, because it's too huge, or off scene (like env-cube probes)
	NO_TREE,
	//! The Y coordinate stored in
	RELATIVE_Y,
	//! Do not spawn this entity in game app
	EDITOR_ONLY,
	//! Do not spawn this entity anywhere
	DISABLED,
	//! Slave child entity, that is linked into scene tree just by its parent. Usually some small props/parts of more complex entity
	PROXY,
	/*!
	Set when entity has changed position, orientation or just
	have to relink its position in scene tree.
	*/
	UPDATE_LINK,
}

/*!
\}
*/
