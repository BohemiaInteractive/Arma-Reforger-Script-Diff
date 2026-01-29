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
derived class connected to gamelib to implement game related features
*/
sealed class World: BaseWorld
{
	/*!
	Creates single visual mark, e.g. from shots, version with projection vector and angle.
	When lifetime=0, pointer to decal is returned, that can be removed by RemoveDecal then.
	\param entity       entity where the Track should be created
	\param origin       first point of the decal, nothing is done now
	\param project      projection direction (length is far clipping distance)
	\param nearclip     near clipping distance
	\param farclip      far clipping distance
	\param materialName Material used for decal
	\param size         size of decal
	\param stretch      stretch of decal
	\param lifetime     Lifetime in seconds, if created with zero or negative value it's used as static and the pointer to decal is returned
	\param color        color of decal
	\param atlasNum     which image num to take in case of atlas
	\return Decal pointer for static decals or null if the decal is dynamic or wasn't created for some reason (invalid material, NoDecal flag, ...)
	*/
	proto external Decal CreateDecal(notnull IEntity entity, vector origin, vector project, float nearclip, float farclip, float angle, float size, float stretch, string materialName, float lifetime, int color, int atlasNum = 0);
	/*!
	Creates single visual mark, e.g. from shots, version with exact matrix.
	When lifetime=0, pointer to decal is returned, that can be removed by RemoveDecal then.
	\param entity       entity where the Track should be created
	\param matrix       projection matrix of decal
	\param nearclip     near clipping distance
	\param farclip      far clipping distance
	\param materialName Material used for decal
	\param size         size of decal
	\param stretch      stretch of decal
	\param lifetime     Lifetime in seconds, if created with zero or negative value it's used as static and the pointer to decal is returned
	\param color        color of decal
	\param atlasNum     which image num to take in case of atlas
	\return Decal pointer for static decals or null if the decal is dynamic or wasn't created for some reason (invalid material, NoDecal flag, ...)
	*/
	proto external Decal CreateDecal2(notnull IEntity entity, vector matrix[4], float nearclip, float farclip, float size, float stretch, string materialName, float lifetime, int color, int atlasNum = 0);
	/*!
	Outlines an entity in the current frame.
	To be called every frame.
	\param ent          The entity to outline
	\param color        Color of the outline in the linear color space (and also of a fill - if used)
	\param occlOpacity  Alpha of an occluded part of the outline
	\param fillOpacity  Alpha of the fill (of occluded parts of the entity), 0 to turn the fill off
	\param inclChildren true to outline the entity together with its child entities
	*/
	proto external void OutlineEntity(notnull IEntity ent, notnull Color color, float occlOpacity = 1.0, float fillOpacity = 0.0, bool inclChildren = false);
	/*!
	Outlines a group of entities in the current frame.
	The entities will be outlined together (without any intersections). To be called every frame.
	\param entities     The group of entities to outline
	\param color        Color of the outline in the linear color space (and also of a fill - if used)
	\param occlOpacity  Alpha of an occluded part of the outline
	\param fillOpacity  Alpha of the fill (of occluded parts of the entity), 0 to turn the fill off
	*/
	proto external void OutlineEntityGroup(notnull array<IEntity> entities, notnull Color color, float occlOpacity = 1.0, float fillOpacity = 0.0);
	//! Reload all systems, by deleting all current systems, and create them again
	proto external void ReloadSystems();
	//! Find a world system based on its type.
	proto external WorldSystem FindSystem(typename type);
	//! Access world systems associated with this world.
	proto external WorldSystems GetSystems();
}

/*!
\}
*/
