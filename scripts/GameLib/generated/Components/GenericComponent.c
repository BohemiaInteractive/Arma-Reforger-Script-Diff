/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class GenericComponent
{
	static bool Preload(IEntityComponentSource src);
	private void GenericComponent(IEntityComponentSource src, IEntity ent, IEntity parent);
	private void ~GenericComponent();

	//! Gets GenericComponentClass descendant instance with the prefab data
	proto external EntityComponentPrefabData GetComponentData(notnull IEntity ent);
	//! Gets config container with the prefab data
	proto external BaseContainer GetComponentSource(notnull IEntity ent);
	/*!
	Attempts to run a remote procedure call (RPC) of this instance with parameters
	specified in method RplRpc attribute.
	\param      method  Member function to be invoked as an RPC.
	*/
	proto protected void Rpc(func method, void p0 = NULL, void p1 = NULL, void p2 = NULL, void p3 = NULL, void p4 = NULL, void p5 = NULL, void p6 = NULL, void p7 = NULL);
	/*!
	* Gets current eventmask of the component.
	* \return Returns bitmask of events the component accepts
	*/
	proto external int GetEventMask();
	/*!
	* Sets eventmask. Component accepts only events which has set bits in eventmask.
	* Bits are or'ed with existing bitmask. See enf::EntityEvents.
	* When this method is called in the constructor of the component, it will not properly set the eventmask to the parent entity. You may consider OnComponentInsert event.
	* \param mask Mask of those bits, which will be set.
	* \return Returns bitmask of events the component accepts (result of mask|GetEventMask())
	*/
	proto external int SetEventMask(notnull IEntity owner, int mask);
	/*!
	* Clears bitmask. Component accepts only events which has set bits in eventmask.
	* Only bits set in the mask are cleared. See enf::EntityEvents
	* \param mask Mask of those bits, which will be cleared.
	* \return returns these bits that were set before and now are cleared.
	*/
	proto external int ClearEventMask(notnull IEntity owner, int mask);
	/*!
	Finds first occurance of the coresponding component.
	\param typeName type of the component
	*/
	proto external GenericComponent FindComponent(typename typeName);
	/*!
	Finds all occurances of the coresponding component.
	\param typeName type of the component
	\param outComponents array to fill with selected components
	*/
	proto external int FindComponents(typename typeName, notnull array<GenericComponent> outComponents);
	/*!
	* Activate component and calls EOnActivate().
	*/
	proto external void Activate(IEntity owner);
	/*!
	* Deactivate component and calls EOnDectivate().
	*/
	proto external void Deactivate(IEntity owner);
	/*!
	* Returns activity state.
	* \return Returns true, if component is active.
	*/
	proto external bool IsActive();
	/*!
	Native implementation of OnTransformReset.

	The default implementation does nothing except calling OnTransformReset on the child components.

	Can be overridden in the native code to alter the default behavior
	(usually the override should also call the base implementation to preserve the recursion).
	There's a script version of OnTransformResetImpl to override too.

	\param params See the TransformResetParams documentation.
	*/
	proto external private void OnTransformResetImplNative(TransformResetParams params);
	proto external protected void ConnectToDiagSystem(IEntity owner);
	proto external protected void DisconnectFromDiagSystem(IEntity owner);
	/*!
	Notifies the component that a transformation of the owner entity has been discontinuously changed.

	Should be called after any transformation discontinuity (right after updating the transform)
	e.g. by teleportation or desync-correction code so the component can react appropriately.
	By default, this is called automatically from GenericEntity::OnTransformReset.

	The default implementation does nothing except calling OnTransformReset on the child components.

	The default behavior may be changed in inherited components by overriding OnTransformResetImpl.

	\param isCorrection [optional] Hint that the transform was reset due to its correction (e.g. by net-code),
											i.e. not a placement/teleport
	\param newVelocity  [optional] Initial velocity of the owner entity after the transform reset
	*/
	proto external void OnTransformReset(bool isCorrection = false, vector newVelocity = vector.Zero);

	// callbacks

	//! Editor changed entity transformation matrix source. Do not call editor API here!
	event void _WB_SetTransform(IEntity owner, inout vector mat[4], IEntitySource src);
	//! Called always after entity creation. It's purpose is to prepare entity for editing. Do not edit anything through editor API here because it's too early for undoable actions! Use plain BaseContainer API for changes through src parameter if needed!
	event void _WB_OnInit(IEntity owner, inout vector mat[4], IEntitySource src);
	//! Editor needs to know if this entity can be deleted. Do not call editor API here!
	event bool _WB_CanDelete(IEntity owner, IEntitySource src) { return true; };
	//! Editor needs to know if this entity can be renamed. Do not call editor API here!
	event bool _WB_CanRename(IEntity owner, IEntitySource src) { return true; };
	//! Editor needs to know if this entity can be copied. Do not call editor API here!
	event bool _WB_CanCopy(IEntity owner, IEntitySource src) { return true; };
	//!	Editor needs to know whether this entity can be selected in scene window or not
	event bool _WB_CanSelect(IEntity owner, IEntitySource src) { return true; };
	//! Editor needs to know a bound box of entity (For ray-casting, visualizers etc.). You can return any custom size you need. Do not call editor API here!
	event void _WB_GetBoundBox(IEntity owner, inout vector min, inout vector max, IEntitySource src);
	//! If entity needs to have a special visualizer instead of default one, here is the place where you can implement it. Do not call editor API here!
	event void _WB_SetExtraVisualiser(IEntity owner, EntityVisualizerType type, IEntitySource src);
	//! An opportunity to append items into editor's "Component" context menu. Do not call editor API here!
	event array<ref WB_UIMenuItem> _WB_GetContextMenuItems(IEntity owner);
	//! Does this entity support editing transformation using physics simulation? Do not call editor API here!
	event bool _WB_OnPhysSimulPlacementBegin(IEntity owner, IEntitySource src) { return true; };
	//! Prepare to edit transformation using physics simulation. Return previous status. Do not call editor API here!
	event bool _WB_EnablePhysics(IEntity owner, IEntitySource src, bool physics);
	//! Any property value has been changed. You can use editor API here and do some additional edit actions which will be part of the same "key changed" action.
	event bool _WB_OnKeyChanged(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent);
	//! Some "coords" or "angles" vector property is being to be edited. Entity can provide a world matrix which represents their local space and which is needed by editor to edit the key by a gizmo
	event bool _WB_GetKeySpaceMatrixWorld(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent, out vector transformSpaceWorld[4]);
	//! Called after updating world in Workbench. The entity must be selected. You can use editor API here and do some edit actions if needed.
	event void _WB_AfterWorldUpdate(IEntity owner, float timeSlice);
	//! Called after _WB_OnInit or also later when editor needs to know whether _WB_AfterWorldUpdate needs to be called and when. Return value can be either 0 (event will not be called at all) or any combination of EEntityFrameUpdateSpecs. Avoid CALL_ALWAYS flag whenever possible to prevent performance issues
	event int _WB_GetAfterWorldUpdateSpecs(IEntity owner, IEntitySource src);
	//! User has chosen any of your menu item from editor's "Component" menu which you have recently provided in WB_GetContextMenuItems(). You can use editor API here and do some edit actions.
	event void _WB_OnContextMenu(IEntity owner, int id);
	//! User pressed a key and this entity is main member of entity selection. You can use editor API here and do some edit actions which will be undoable.
	event void _WB_OnKeyDown(IEntity owner, int keyCode);
	//! Called after entity gets created in map during editing or when deleted entity gets restored after undo action. This event isn't called by loading map!!! If you need an event that is being called after every entity creation then use WB_OnInit instead it. You can use editor API here and do some additional edit actions which will be part of the same "create entity" action.
	event void _WB_OnCreate(IEntity owner, IEntitySource src);
	//! Parent entity has been changed ( it's available through src->GetParent() ). prevParentSrc is a pointer to a previous parent (if any)
	event void _WB_OnParentChange(IEntity owner, IEntitySource src, IEntitySource prevParentSrc);
	//! Entity is going to be deleted. You can use editor API here and do some additional edit actions which will be part of the same "entity delete" action.
	event void _WB_OnDelete(IEntity owner, IEntitySource src);
	//! Entity has been renamed. You can use editor API here and do some additional edit actions which will be part of the same "entity rename" action.
	event void _WB_OnRename(IEntity owner, IEntitySource src, string oldName);
	//! Possibility to get variable value choices dynamically
	event array<ref ParamEnum> _WB_GetUserEnums(string varName, IEntity owner, IEntityComponentSource src);
	/*!
	Script-side implementation of OnTransformReset.

	The default implementation does nothing except calling OnTransformReset on the child components.

	Can be overridden to alter the default behavior.
	Usually, you'll want to call the base implementation somewhere in the override to preserve the recursion.

	\param params See the TransformResetParams documentation.
	*/
	event protected void OnTransformResetImpl(TransformResetParams params){ OnTransformResetImplNative(params); };
}

/*!
\}
*/
