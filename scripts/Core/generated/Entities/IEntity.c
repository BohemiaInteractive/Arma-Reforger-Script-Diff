/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class IEntity: Managed
{
	/*!
	Event when touched by other entity.
	\note You need to have TouchComponent in entity to receive this event.

	\param owner The owner entity
	\param other Entity who touched us
	\param touchTypesMask Bitmask of touch types TODO
	*/
	event protected void EOnTouch(IEntity owner, IEntity other, int touchTypesMask);
	/*!
	Event after entity is allocated and initialized.
	\param owner The owner entity
	*/
	event protected void EOnInit(IEntity owner);
	/*!
	Event when we are visible.
	\param owner The owner entity
	\param frameNumber Frame number
	*/
	event protected void EOnVisible(IEntity owner, int frameNumber);
	/*!
	Event every frame.
	\param owner The owner entity
	\param timeSlice Time passed since last frame
	*/
	event protected void EOnFrame(IEntity owner, float timeSlice);
	/*!
	Event after physics update.
	\param owner The owner entity
	\param timeSlice Time passed since last frame
	*/
	event protected void EOnPostFrame(IEntity owner, float timeSlice);
	/*!
	Event from animation system.
	\param owner The owner entity
	\param type
	\param slot
	*/
	event protected void EOnAnimEvent(IEntity owner, int type, int slot);
	/*!
	Event before every physics fixed step (can be multiple calls per engine update)
	\param owner The owner entity
	\param timeSlice Time slice of physics fixed step
	*/
	event protected void EOnSimulate(IEntity owner, float timeSlice);
	/*!
	Event after every physics fixed step (can be multiple calls per engine update)
	\param owner The owner entity
	\param timeSlice Time slice of physics fixed step
	*/
	event protected void EOnPostSimulate(IEntity owner, float timeSlice);
	/*!
	Event when joint attached to RigidBody of this entity is broken.
	\param owner The owner entity
	\param other Other Entity attached to the joint
	*/
	event protected void EOnJointBreak(IEntity owner, IEntity other);
	/*!
	Event when physics engine has moved with this Entity.
	\param owner The owner entity
	*/
	event protected void EOnPhysicsMove(IEntity owner);
	/*!
	Event when physics engine registered contact with other RigidBody.
	\param owner The owner entity
	\param other Other Entity who contacted us
	\param contact Structure describing the contact
	*/
	event protected void EOnContact(IEntity owner, IEntity other, Contact contact);
	/*!
	Event when a RigidBody active state is changed between consecutive fixed steps
	\param owner The owner entity
	*/
	event protected void EOnPhysicsActive(IEntity owner, bool activeState);
	/*!
	Event every fixed frame.
	\param owner The owner entity
	\param timeSlice Fixed time step
	*/
	event protected void EOnFixedFrame(IEntity owner, float timeSlice);
	/*!
	Event after physics update every fixed frame.
	\param owner The owner entity
	\param timeSlice Fixed time step
	*/
	event protected void EOnFixedPostFrame(IEntity owner, float timeSlice);
	//! EntityEvent.EV_USER+0
	protected void EOnUser0(IEntity other, int extra);
	//! EntityEvent.EV_USER+1
	protected void EOnUser1(IEntity other, int extra);
	//! EntityEvent.EV_USER+2
	protected void EOnUser2(IEntity other, int extra);
	//! EntityEvent.EV_USER+3
	protected void EOnUser3(IEntity other, int extra);
	//! EntityEvent.EV_USER+4
	protected void EOnUser4(IEntity other, int extra);
	//! Placeholder
	private void EOnDummy018(IEntity other, int extra);
	//! Placeholder
	private void EOnDummy019(IEntity other, int extra);
	//! Placeholder
	private void EOnDummy020(IEntity other, int extra);
	//! Placeholder
	private void EOnDummy021(IEntity other, int extra);
	//! Placeholder
	private void EOnDummy022(IEntity other, int extra);
	//! Placeholder
	private void EOnDummy023(IEntity other, int extra);
	//! Placeholder
	private void EOnDummy024(IEntity other, int extra);
	//! Placeholder
	private void EOnDummy025(IEntity other, int extra);
	//! Placeholder
	private void EOnDummy026(IEntity other, int extra);
	//! Placeholder
	private void EOnDummy027(IEntity other, int extra);
	//! Placeholder
	private void EOnDummy028(IEntity other, int extra);
	//! Placeholder
	private void EOnDummy029(IEntity other, int extra);
	//! Placeholder
	private void EOnDummy030(IEntity other, int extra);
	//! Placeholder
	private void EOnDummy031(IEntity other, int extra);
	//-----------------------------------------------------------------------
	//! protected script Constructor
	protected void IEntity(IEntitySource src, IEntity parent);

	/*!
	Return unique entity ID.
	\code
		ItemBase apple = g_Game.CreateObject( "FruitApple", String2Vector("0 10 0"), false );
		Print( apple.GetID() );

		>> 0
	\endcode
	*/
	proto external EntityID GetID();
	/*!
	Returns parent of this entity (entity on upper level
	of hierarchy).
	\return
	Pointer to parent entity, or nullptr, when we are not child.
	*/
	proto external IEntity GetParent();
	/*!
	Returns top-parent of this entity.
	\return
	Pointer to root entity, or self, when we are not child.
	*/
	proto external IEntity GetRootParent();
	/*!
	Returns first child from this level of hierarchy
	if there is any.
	\return
	Pointer to child entity, or nullptr
	*/
	proto external IEntity GetChildren();
	/*!
	Returns next sibling of this entity (next entity on this level
	of hierarchy).
	\return
	Pointer to sibling entity, or nullptr
	*/
	proto external IEntity GetSibling();
	//! Returns visual object set to this Entity. No reference is added.
	proto external VObject GetVObject();
	proto external EntityPrefabData GetPrefabData();
	proto external EntityComponentPrefabData FindComponentData(typename typeName);
	proto external BaseWorld GetWorld();
	//! Set fixed LOD. Use -1 for non-fixed LOD.
	proto external void SetFixedLOD(int lod);
	/*!
	Returns world transformation of Entity. It returns only as many vectors as is array length.
	\code
		Man player = g_Game.GetPlayer();

		vector mat[4];
		player.GetTransform(mat);
		Print( mat );

		>> <0.989879,-0,0.141916>,<0,1,0>,<-0.141916,0,0.989879>,<2545.08,15.6754,2867.49>
	\endcode
	\param mat `vector[1...4]` matrix to be get
	*/
	proto external void GetTransform(out vector mat[]);
	//! See IEntity#GetTransform
	proto external void GetWorldTransform(out vector mat[]);
	//! See IEntity#GetTransform
	proto external void GetLocalTransform(out vector mat[]);
	/*!
	Returns combined local transformation of parent's pivot attachment and local transform
	*/
	proto external void ComputeLocalTransform(out vector mat[]);
	/*!
	Transforms local vector to parent(world) space.
	\code
		Man player = g_Game.GetPlayer();
		Print( player.VectorToParent("1 2 3") );

		>> <2.89791,2,1.26575>
	\endcode
	\param vec local space vector to transform
	\return parent space vector
	*/
	proto external vector VectorToParent(vector vec);
	/*!
	Transforms local position to parent(world) space.
	\code
		Man player = g_Game.GetPlayer();
		Print( player.CoordToParent("1 2 3") );

		>> <2549,17.6478,2857>
	\endcode
	\param coord local position to transform
	\return position in parent space
	*/
	proto external vector CoordToParent(vector coord);
	/*!
	Transforms world space vector to local space.
	\code
		Man player = g_Game.GetPlayer();
		Print( player.VectorToLocal("2 1 5") );

		>> <-0.166849,1,5.38258>
	\endcode
	\param vec world space vector to transform
	\return local space vector
	*/
	proto external vector VectorToLocal(vector vec);
	/*!
	Transforms world space position to local space.
	\code
		Man player = g_Game.GetPlayer();
		Print( player.CoordToLocal("500 10 155") );

		>> <15254,-54.2004,8745.53>
	\endcode
	\param coord world space position to transform
	\return position in local space
	*/
	proto external vector CoordToLocal(vector coord);
	/*!
	Returns orientation of Entity in world space (Yaw, Pitch, Roll).
	\code
		Man player = g_Game.GetPlayer();
		Print( player.GetYawPitchRoll() );

		>> <180,-76.5987,180>
	\endcode
	\return entity orientation
	*/
	proto external vector GetYawPitchRoll();
	/*!
	Sets angles for entity (Yaw, Pitch, Roll).
	\code
		Man player = g_Game.GetPlayer();
		player.SetYawPitchRoll("180 50 180" );
		Print( player.GetYawPitchRoll() );

		>> <-180,50,-180>
	\endcode
	\param angles angles to be set
	*/
	proto external void SetYawPitchRoll(vector angles);
	//! See IEntity#GetTransformAxis
	proto external vector GetWorldTransformAxis(int axis);
	//! See IEntity#GetTransformAxis
	proto external vector GetTransformAxis(int axis);
	//! See IEntity#GetTransformAxis
	proto external vector GetLocalTransformAxis(int axis);
	//! See IEntity#SetTransform. Returns `false`, if there is no change in transformation.
	proto external bool SetLocalTransform(vector mat[4]);
	//! See IEntity#SetTransform. Returns `false`, if there is no change in transformation.
	proto external bool SetWorldTransform(vector mat[4]);
	//! Returns pivot ID from hierarchy component.
	proto external TNodeId GetPivot();
	/*!
	Sets entity world transformation.
	\code
		vector mat[4];
		Math3D.MatrixIdentity( mat )

		Man player = g_Game.GetPlayer();
		player.SetTransform( mat );

		vector outmat[4];
		player.GetTransform(outmat );
		Print( outmat );

		>> <1,0,0>,<0,1,0>,<0,0,1>,<0,0,0>
	\endcode
	\param mat matrix to be set
	*/
	proto external bool SetTransform(vector mat[4]);
	//! Same as GetLocalYawPitchRoll(), but returns rotation vector around X, Y and Z axis.
	proto external vector GetLocalAngles();
	/*!
	Returns local orientation when it's in hierarchy (Yaw, Pitch, Roll).
	\code
		Man player = g_Game.GetPlayer();
		Print( player.GetLocalYawPitchRoll() );

		>> <180,-57.2585,180>
	\endcode
	\return local orientation
	*/
	proto external vector GetLocalYawPitchRoll();
	//! Same as GetYawPitchRoll(), but returns rotation vector around X, Y and Z axis.
	proto external vector GetAngles();
	//! Same as SetYawPitchRoll(), but sets rotation around X, Y and Z axis.
	proto external void SetAngles(vector angles);
	/*!
	Returns origin of Entity.
	\code
		Man player = g_Game.GetPlayer();
		Print( player.GetOrigin() );

		>> <2577.02,15.6837,2924.27>
	\endcode
	\return entity origin
	*/
	proto external vector GetOrigin();
	/*!
	Sets origin for entity.
	\code
		Man player = g_Game.GetPlayer();
		player.SetOrigin("2550 10 2900" );
		Print( player.GetOrigin() );

		>> <2550,10,2900>
	\endcode
	\param orig origin to be set
	*/
	proto external void SetOrigin(vector orig);
	proto external void SetScale(float scale);
	proto external float GetScale();
	/*!
	Returns local bounding box of model on Entity.
	\code
		Man player = g_Game.GetPlayer();

		vector mins, maxs;
		player.GetBounds(mins, maxs );

		Print( mins );
		Print( maxs );

		>> <0,0,0>
		>> <0,0,0>
	\endcode
	\param[out] mins minimum point of bounding box
	\param[out] maxs maximum point of bounding box
	*/
	proto external void GetBounds(out vector mins, out vector maxs);
	/*!
	Returns quantized world-bound-box of Entity.
	\code
		Man player = g_Game.GetPlayer();

		vector mins, maxs;
		player.GetWorldBounds( mins, maxs );

		Print( mins );
		Print( maxs );

		>> <2547.2,15.5478,2852.85>
		>> <2548.8,17.5478,2855.05>
	\endcode
	\param[out] mins minimum point of bounding box
	\param[out] maxs maximum point of bounding box
	*/
	proto external void GetWorldBounds(out vector mins, out vector maxs);
	//! Dynamic event invocation. Parameters are the same as in `IEntity::EOnXXXX()` methods.
	proto external volatile void SendEvent(notnull IEntity actor, EntityEvent e, void extra);
	proto external string GetName();
	//! Sets component flags.
	proto external int SetVComponentFlags(VCFlags flags);
	/*!
	Sets the visual object to this entity. Reference is added and released
	upon entity destruction.
	\param object Handle to object got by GetObject().
	\param options String, dependent on object type. The only one supported for
	XOB objects:
	```"$remap 'original material name' 'new material'; [$remap 'another original material name' 'another new material']"```
	*/
	proto external void SetObject(VObject object, string options);
	/*!
	Sets Entity flags. It's OR operation, not rewrite. Returns previous flags.
	\code
		Man player = g_Game.GetPlayer();
		player.SetFlags(EntityFlags.VISIBLE|EntityFlags.TRACEABLE );
		Print( player.GetFlags() );

		>> 1610612747
	\endcode
	\param flags Flags to be set
	\param recursively Flags will be recursively applied to children of hierarchy too
	\return previous flags
	*/
	proto external EntityFlags SetFlags(EntityFlags flags, bool recursively = false);
	/*!
	Clear Entity flags. Returns cleared flags.
	\code
		Man player = g_Game.GetPlayer();
		player.ClearFlags(EntityFlags.VISIBLE|EntityFlags.TRACEABLE );
		Print( player.GetFlags() );

		>> 1610612744
	\endcode
	\param flags Flags to be set
	\param recursively Flags will be recursively applied to children of hierarchy too
	\return cleared flags
	*/
	proto external EntityFlags ClearFlags(EntityFlags flags, bool recursively = false);
	/*!
	Returns Entity flags.
	\code
		Man player = g_Game.GetPlayer();
		Print( player.GetFlags() );

		>> 1610612745
	\endcode
	\return Entity flags
	*/
	proto external EntityFlags GetFlags();
	/*!
	Sets event mask.
	\code
		Man player = g_Game.GetPlayer();
		Print( player.GetEventMask() );
		player.SetEventMask( EntityEvent.VISIBLE );
		Print( player.GetEventMask() );

		>> 0
		>> 128
	\endcode
	\param e Combined mask of one or more members of EntityEvent enum
	*/
	proto external EntityEvent SetEventMask(EntityEvent e );
	/*!
	Clears event mask.
	\code
		Man player = g_Game.GetPlayer();
		player.SetEventMask(EntityEvent.VISIBLE );
		Print( player.GetEventMask() );
		player.ClearEventMask(EntityEvent.ALL );
		Print( player.GetEventMask() );

		>> 128
		>> 0
	\endcode
	\param e event mask
	\return event mask
	*/
	proto external EntityEvent ClearEventMask(EntityEvent e);
	/*!
	Returns current event mask.
	\code
		Man player = g_Game.GetPlayer();
		Print( player.GetEventMask() );

		>> 0
	\endcode
	\return current event mask
	*/
	proto external EntityEvent GetEventMask();
	//! \returns `true` when entity is loaded from map, `false` when dynamically spawned.
	proto external bool IsLoaded();
	//! \returns `true` if entity was deleted (entity pointer valid until the end of the frame).
	proto external bool IsDeleted();
	/*!
	Updates entity state/position. Should be called when you want to manually commit position changes etc
	before trace methods etc. Entity is updated automatically at the end and the beginning of simulation step,
	when it has EntityFlags.TFL_ACTIVE flag set.
	\returns Mask with flags:
	- EntityFlags.UPDATE - hierarchy has been updated
	- EntityFlags.UPDATE_MDL - model hierarchy has been updated
	*/
	proto external int Update();
	//! Add Entity to hierarchy. Pivot is pivot index, or -1 for center of parent.
	proto external int AddChild(notnull IEntity child, TNodeId pivot, EAddChildFlags flags = EAddChildFlags.AUTO_TRANSFORM);
	//! Remove Entity from hierarchy.
	proto external void RemoveChild(notnull IEntity child, bool keepTransform = false);
	proto external void SetName(string name);
	//! Sets visibility mask for cameras, where Entity will be rendered.
	proto external int SetCameraMask(int mask);
	proto external Physics GetPhysics();
	proto external Particles GetParticles();
	proto external Animation GetAnimation();
	//! Updates animation (either xob, or particle, whatever).
	proto external int Animate(float speed, int loop);
	//! Updates animation (either xob, or particle, whatever).
	proto external int AnimateEx(float speed, int loop, out vector lin, out vector ang);
	[Obsolete("Use GetAnimation().SetBone() instead")]
	proto external void	SetBone(TNodeId bone, vector angles, vector trans, float scale);
	[Obsolete("Use GetAnimation().SetBoneMatrix() instead")]
	proto external bool	SetBoneMatrix(TNodeId bone, vector mat[4]);
	[Obsolete("Use GetAnimation().GetBoneMatrix() instead")]
	proto external bool	GetBoneMatrix(TNodeId bone, out vector mat[4]);
	[Obsolete("Use GetAnimation().GetBoneIndex() instead")]
	proto external TNodeId	GetBoneIndex(string boneName);
	[Obsolete("Use GetAnimation().GetBoneNames() instead")]
	proto external void	GetBoneNames(out notnull array<string> boneNames);
	[Obsolete("Use GetAnimation().GetBoneLocalMatrix() instead")]
	proto external bool	GetBoneLocalMatrix(TNodeId bone, out vector mat[4]);
	/*!
	Finds first occurance of the coresponding component.
	\param typeName type of the component
	*/
	proto external Managed FindComponent(typename typeName);
	/*!
	Finds all occurances of the coresponding component.
	\param typeName type of the component
	\param outComponents array to fill with selected components
	*/
	proto external int FindComponents(typename typeName, notnull array<Managed> outComponents);
}

/*!
\}
*/
