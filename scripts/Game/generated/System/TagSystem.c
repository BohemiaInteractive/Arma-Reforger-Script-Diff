/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

class TagSystem: GameSystem
{
	//! Returns registered entities in given categories within a given range.
	proto external int GetTagsInRange(out notnull array<IEntity> entities, vector origin, float range, ETagCategory categories);
	proto external void RegisterEntity(IEntity entity, bool isDynamic, ETagCategory categories);
	proto external void UnregisterEntity(IEntity entity);
}

/*!
\}
*/
