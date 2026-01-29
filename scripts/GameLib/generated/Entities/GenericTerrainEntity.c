/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class GenericTerrainEntityClass: GenericEntityClass
{
}

class GenericTerrainEntity: GenericEntity
{
	/*!
	Submit an ellipse obstacle in this frame
	\param position				World coordinates of the obstacles bottom center
	\param velocity				Obstacle velocity, world units / s
	\param height					Obstacle height
	\param sizeX					Size in the X direction
	\param sizeZ					Size in the Z direction
	\param maskOffset			Offset of the obstacle in the Z direction, [-1, 1]
	\param angleRAD				Obstacle rotation around the Y axis, radians, clockwise
	\param strength				Strength of the obstacle, [0, 1]
	\param falloffOffset	Used to offset obstacle strength falloff, [0, 1]
	\param flatten				Should flatten?
	\param sway						Should sway?
	*/
	proto external void AffectFoliageEllipse(vector position, vector velocity, float height, float sizeX, float sizeZ, float maskOffset, float angleRAD, float strength = 1.0, float falloffOffset = 0.5, bool flatten = true, bool sway = true);
	/*!
	Submit a circle obstacle in this frame
	\param position				World coordinates of the obstacles bottom center
	\param velocity				Obstacle velocity, world units / s
	\param height					Obstacle height
	\param diameter				Circle diameter
	\param strength				Strength of the obstacle, [0, 1]
	\param falloffOffset	Used to offset obstacle strength falloff, [0, 1]
	\param flatten				Should flatten?
	\param sway						Should sway?
	*/
	proto external void AffectFoliageCircle(vector position, vector velocity, float height, float diameter, float strength = 1.0, float falloffOffset = 0.5, bool flatten = true, bool sway = true);
	/*!
	Submit a rectangle obstacle in this frame
	\param position				World coordinates of the obstacles bottom center
	\param velocity				Obstacle velocity, world units / s
	\param height					Obstacle height
	\param sizeX					Size in the X direction
	\param sizeZ					Size in the Z direction
	\param maskOffset			Offset of the obstacle in the Z direction, [-1, 1]
	\param angleRAD				Obstacle rotation around the Y axis, radians, clockwise
	\param strength				Strength of the obstacle, [0, 1]
	\param falloffOffset	Used to offset obstacle strength falloff, [0, 1]
	\param flatten				Should flatten?
	\param sway						Should sway?
	*/
	proto external void AffectFoliageRect(vector position, vector velocity, float height, float sizeX, float sizeZ, float maskOffset, float angleRAD, float strength = 1.0, float falloffOffset = 0.5, bool flatten = true, bool sway = true);
	/*!
	Submit a square obstacle in this frame
	\param position				World coordinates of the obstacles bottom center
	\param velocity				Obstacle velocity, world units / s
	\param height					Obstacle height
	\param size						Size of the box
	\param angleRAD				Obstacle rotation around the Y axis, radians, clockwise
	\param strength				Strength of the obstacle, [0, 1]
	\param falloffOffset	Used to offset obstacle strength falloff, [0, 1]
	\param flatten				Should flatten?
	\param sway						Should sway?
	*/
	proto external void AffectFoliageSquare(vector position, vector velocity, float height, float size, float angleRAD, float strength = 1.0, float falloffOffset = 0.5, bool flatten = true, bool sway = true);
	proto bool GetTileTextureResName(int tile, out ResourceName textureResName);
	/*!
	get linear tile number from texcoords
	\param terrx	x terrain coords
	\param terry	y terrain coords
	\return tile number in linear pos
	*/
	proto external int GetTileNumber(int terrx, int terry);
	/*!
	convert point from world to terr coords
	\param worldpos		worldpos
	\return terrpos
	*/
	proto external vector WorldToTerrCoord(vector worldpos);
	/*!
	convert point from terr to worldpos coords
	\param terrpos	terrpos
	\return worldpos
	*/
	proto external vector TerrToWorldCoord(vector terrpos);
	proto bool TryGetHeightTC(float x, float z, out float y);
	/*!
	number of vertices in tile
	*/
	proto external int GetTileVerticesCount();
}

/*!
\}
*/
