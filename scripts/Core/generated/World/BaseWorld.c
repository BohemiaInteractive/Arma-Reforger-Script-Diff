/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup World
\{
*/

sealed class BaseWorld: global_pointer
{
	/*!
	Returns number of loaded entities in certain sub scene.
	*/
	proto external int GetNumEntities(int subScene);
	/*!
	returns loaded entity ID
	*/
	proto external EntityID GetEntity(int subScene, int entityIndex);
	proto external float GetSurfaceY(float x, float z);
	proto external void GetActiveEntities(notnull out array<IEntity> entities);
	/*!
	Query for entities on the line. Entities are roughly sorted by distance from start to end of line
	\param from Start of line
	\param to End of line
	\param addEntity
	Callback method. called when another Entity is intersected during Query methods.
	\return Return true, when query should continue, False when you want to quit query call.
	\param filterEntity
	[optional] Callback method. Called when another Entity is encountered during Query methods.
	Some light-weight filtering could be implemented here, prior to	intersection test \return True when entity has to be further tested, False otherwise
	\param queryFlags Various flags See EQueryEntitiesFlags
	\return False when query was canceled.
	*/
	proto external bool QueryEntitiesByLine(vector from, vector to, QueryEntitiesCallback addEntity, QueryEntitiesCallback filterEntity = null, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL);
	/*!
	Query for entities touched by axis aligned box.
	\param mins
	Mins of axis aligned box
	\param maxs
	Maxs of axis aligned box
	\param addEntity
	Callback method. called when another Entity is intersected during Query methods.
	\return Return true, when query should continue, False when you want to quit query call.
	\param filterEntity
	[optional] Callback method. Called when another Entity is encountered during Query methods.
	Some light-weight filtering could be implemented here, prior to	intersection test \return True when entity has to be further tested, False otherwise
	\param queryFlags Various flags See EQueryEntitiesFlags
	\return
	False when query was canceled.
	*/
	proto external bool QueryEntitiesByAABB(vector mins, vector maxs, QueryEntitiesCallback addEntity, QueryEntitiesCallback filterEntity = null, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL);
	/*!
	Query for entities touched by sphere.
	\param center
	Center of sphere
	\param radius
	Radius of sphere
	\param addEntity
	Callback method. called when another Entity is intersected during Query methods.
	\return Return true, when query should continue, False when you want to quit query call.
	\param filterEntity
	[optional] Callback method. Called when another Entity is encountered during Query methods.
	Some light-weight filtering could be implemented here, prior to	intersection test \return True when entity has to be further tested, False otherwise
	\param queryFlags Various flags See EQueryEntitiesFlags
	\return
	False when query was canceled.
	*/
	proto external bool QueryEntitiesBySphere(vector center, float radius, QueryEntitiesCallback addEntity, QueryEntitiesCallback filterEntity = null, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL);
	/*!
	Query for entities on the beveled line (sweeping test by AABB)
	Entities are roughly sorted by distance from start to end of line
	\param from
	Start of line
	\param to
	End of line
	\param bevelMins
	Mins of bevel axis aligned box
	\param bevelMaxs
	Maxs of bevel axis aligned box
	\param addEntity
	Callback method. called when another Entity is intersected during Query methods.
	\return Return true, when query should continue, False when you want to quit query call.
	\param filterEntity
	[optional] Callback method. Called when another Entity is encountered during Query methods.
	Some light-weight filtering could be implemented here, prior to	intersection test \return True when entity has to be further tested, False otherwise
	\param queryFlags Various flags See EQueryEntitiesFlags
	\return
	False when query was canceled.
	*/
	proto external bool QueryEntitiesByBeveledLine(vector from, vector to, vector bevelMins, vector bevelMaxs, QueryEntitiesCallback addEntity, QueryEntitiesCallback filterEntity = null, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL);
	/*!
	Query for entities touched by oriented box
	\param mins
	Mins of axis aligned box
	\param maxs
	Maxs of axis aligned box
	\param matrix
	Orientation of box
	\param addEntity
	Callback method. called when another Entity is intersected during Query methods.
	\return Return true, when query should continue, False when you want to quit query call.
	\param filterEntity
	[optional] Callback method. Called when another Entity is encountered during Query methods.
	Some light-weight filtering could be implemented here, prior to	intersection test \return True when entity has to be further tested, False otherwise
	\param queryFlags Various flags See EQueryEntitiesFlags
	\return
	False when query was canceled.
	*/
	proto external bool QueryEntitiesByOBB(vector mins, vector maxs, vector matrix[4], QueryEntitiesCallback addEntity, QueryEntitiesCallback filterEntity = null, EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.ALL);
	proto external void SchedulePreload(vector pos, float radius);
	proto external void GetBoundBox(out vector mins, out vector maxs);
	/*!
	Is ocean available
	*/
	proto external bool IsOcean();
	/*!
	Get water ocean height at given point
	\param worldX		world x position
	\param worldZ		world z position
	*/
	proto external float GetOceanHeight(float worldX, float worldZ);
	/*!
	Get water ocean height and displacement at given point, returns vector(displaceX, height, displaceZ)
	\param worldX		world x position
	\param worldZ		world z position
	*/
	proto external vector GetOceanHeightAndDisplace(float worldX, float worldZ);
	/*!
	return base ocean level
	*/
	proto external float GetOceanBaseHeight();
	/*!
	is in camera ocean enabled?
	\param camera		camera
	*/
	proto external bool IsOceanEnabledInCamera(int camera);
	/*!
	enable ocean render in some camera
	\param camera		camera
	\param enable		enabled = true
	*/
	proto external void EnabledOceanInCamera(int camera, bool enable);
	/*
	water body
	*/
	proto external void RegisterWaterBody(notnull IEntity ent, ResourceName matName);
	/*
	remove water body
	*/
	proto external void RemoveWaterBody(notnull IEntity ent);
	/*!
	remove decal
	\param decal 	item to be removed
	*/
	proto external void RemoveDecal(Decal decal);
	/*!
	Creates continous visual mark, e.g. from wheel when a car is moving on the ground
	\param entity		entity where the Track should be created (only terrain is supported ATM)
	\param origin		first point of the decal, nothing is done now
	\param normal		normal of surface
	\param edgeSize	Edge size of decal
	\param lifeTime	Lifetime in seconds
	\param materialName Material used for decal
	\param prevDecal	Previous decal, we are connecting to
	\param alpha		translucency of point
	\return TrackDecal pointer or null
	*/
	proto external TrackDecal CreateTrackDecal(IEntity entity, vector origin, vector normal, float edgeSize, float lifeTime, string materialName, Decal prevDecal, float alpha);
	proto external int GetCurrentCameraId();
	/*!
	Changes camera position
	\param cam Index of camera
	\param origin	position
	\param angle	orientation (angles <Yaw, Pitch, Roll> in degrees)
	*/
	proto external void SetCamera(int cam, vector origin, vector angle);
	proto external void GetCamera(int cam, out vector mat[4]);
	proto external void GetCurrentCamera(out vector mat[4]);
	//! Changes camera matrix
	proto external void SetCameraEx(int cam, const vector mat[4]);
	proto external void SetCameraVerticalFOV(int cam, float fovy);
	//! Default 5 units
	proto external void SetCameraNearPlane(int cam, float nearplane);
	//! Default 160000 units
	proto external void SetCameraFarPlane(int cam, float farplane);
	//! set HDR camera exposure (if hdrBrightness > 0, camera is set to manual mode with this exposure, set -1 to enable again auto mode)
	proto external void SetCameraHDRBrightness(int cam, float hdrBrightness);
	//! adjust camera EV (light stops), 0 = no adjustment, both positive/negative values are allowed
	proto external void AdjustCameraEV(int cam, float EV);
	//! Returns actual camera HDR exposure
	proto external float GetCameraHDRBrightness(int cam);
	//! Returns actual camera normalized scene middle brightness
	proto external float GetCameraSceneMiddleBrightness(int cam);
	/*!
	set postprocess effect to camera
	To disable effect in some prioroty ppEffect, just set effectName or name to NULL
	\param cam 		number of camera
	\param priority		priority of effect, currently we support max 20 effects thus max priority is 19 (priority means also order of rendering, less = earlier)
	\param type	type of effect
	\param materialPath 		material
	*/
	proto external void SetCameraPostProcessEffect(int cam, int priority, PostProcessEffectType type, string materialPath);
	proto external void SetCameraType(int cam, CameraType type);
	/*!
	Set lens flare set for given camera.
	\param cam number of camera
	\param lensFlareSetType	type of the lens flare set
	\param userLensFlareSetName name of the lens flare set (is used then type is set to User)
	*/
	proto external void SetCameraLensFlareSet(int cam, CameraLensFlareSetType lensFlareSetType, string userLensFlareSetName);
	/*!
	Project a position from world to viewport of given resolution.
	\param pos Position in world space
	\param cam Camera index to use
	\param width Viewport width
	\param height Viewport height
	*/
	proto external vector ProjectWorldToViewport(vector pos, int cam, int width, int height);
	/*!
	Project a position from viewport of given resolution to world.
	\param x Coord in viewport
	\param y Coord in viewport
	\param cam Camera index to use
	\param width Viewport width
	\param height Viewport height
	\param outDir Returned direction vector
	*/
	proto external vector ProjectViewportToWorld(float x, float y, int cam, int width, int height, out vector outDir);
	[Obsolete("use TraceMove instead")]
	proto external bool TraceLineToEntity(notnull IEntity ent, vector start, vector end, Class placeHolder = null);
	/*!
	Traces shape collision at start position, return <0 if we hit something. Results are filled in param object @see TraceParam
	\param param object used for both input parameters and output parameters @see TraceParam
	\param filterCallback callback called for each entity in trace path used to filter them
	\returns value 0 or negative number as penetration depth
	*/
	proto external float TracePosition(inout TraceParam param, TraceFilterCallback filterCallback = null);
	/*!
	Traces shape movement along start->end, return 0..1 if trace was sucessfull. Results are filled in param object @see TraceParam
	\param param object used for both input parameters and output parameters @see TraceParam
	\param filterCallback callback called for each entity in trace path used to filter them
	\returns	value 0...1, percentage of a path traveled
	*/
	proto external float TraceMove(inout TraceParam param, TraceFilterCallback filterCallback = null);
	/*!
	Traces shape movement along start->end asynchronously, callbeck is called when finished. Framework is hodling reference to param until callback is called. Results are same as @see BaseWorld.TraceMove.
	\param param object used for both input parameters and output parameters @see TraceParam
	\param finishedCallback callback called when trace is finished
	*/
	proto external void AsyncTraceMove(inout TraceParam param, AsyncTraceFinishedCallback finishedCallback);
	//! Returns current lifetime of the World in milliseconds
	proto external float GetWorldTime();
	//! Returns current timestamp of the World.
	proto external WorldTimestamp GetTimestamp();
	//! Returns current frame of the World
	proto external int GetFrameNumber();
	//! Returns actual time scale of world, can be different from engine time scale
	proto external float GetTimeScale();
	//! Returns time slice of the world.
	proto external float GetTimeSlice();
	//! Returns fixed time slice of the world.
	proto external float GetFixedTimeSlice();
	//! Returns physics time slice of the world.
	proto external float GetPhysicsTimeSlice();
	proto external IEntity FindEntityByName(string name);
	proto external IEntity FindEntityByID(EntityID ID);
	//! Returns true during edit mode in Workbench and in Ingame editor
	proto external bool IsEditMode();
	//! Update active entities in the world, do not call from inside another world update (e.g. from IEntity::EOnXXX)
	proto external void UpdateEntities();
	//! Create new empty BaseWorld
	static proto SharedItemRef CreateWorld(string type, string name);
}

/*!
\}
*/
