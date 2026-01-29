/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

/*!
* Base entity class.
*/
class GenericEntity: IEntity
{
	/*!
	Called when Item is initialized from replication stream. Carries the data from Master.
	*/
	event protected bool RplLoad(ScriptBitReader reader);
	/*!
	Called when Item is getting replicated from Master to Slave connection. The data will be
	delivered to Slave using RplInit method.
	*/
	event protected bool RplSave(ScriptBitWriter writer);
	static event bool Preload(IEntitySource src);

	/*!
	* Activate entity events.
	* Will also activate all components of the entity.
	*/
	proto external void Activate();
	/*!
	* Deactivate entity events.
	* Will also deactivate all components of the entity.
	*/
	proto external void Deactivate();
	proto external void Show(bool show);
	/*!
	Attempts to run a remote procedure call (RPC) of this instance with parameters
	specified in method RplRpc attribute.
	\param      method  Member function to be invoked as an RPC.
	*/
	proto protected void Rpc(func method, void p0 = NULL, void p1 = NULL, void p2 = NULL, void p3 = NULL, void p4 = NULL, void p5 = NULL, void p6 = NULL, void p7 = NULL);
	//! This returns world editor API, which is safe to use from editor events bellow
	proto external WorldEditorAPI _WB_GetEditorAPI();
	static proto void GetMatrixFromSource(BaseWorld world, IEntitySource src, bool withScale, out vector mat[4]);
	/*!
	Native implementation of OnTransformReset.

	The default implementation handles particles (if present on the entity)
	and calls OnTransformReset on all of the entity's GenericComponent instances and child
	GenericEntity instances with auto-transform enabled
	(i.e. it handles the whole entity and component hierarchy by recursion).

	Can be overridden in the native code to alter the default behavior
	(usually the override should also call the base implementation to preserve the recursion).
	There's a script version of OnTransformResetImpl to override.

	\param params See the TransformResetParams documentation.
	*/
	proto external private void OnTransformResetImplNative(TransformResetParams params);
	proto external protected void ConnectToDiagSystem();
	proto external protected void DisconnectFromDiagSystem();
	/*!
	Notifies the entity that its transformation has been discontinuously changed.

	Should be called after any transformation discontinuity (right after updating the transform)
	e.g. by teleportation or desync-correction code so the entity can react appropriately.

	The default implementation handles Particles (if present on the entity)
	and calls OnTransformReset on all of the entity's GenericComponent instances and child
	GenericEntity instances with auto-transform enabled
	(i.e. it handles the whole entity and component hierarchy by recursion).

	The default behavior may be changed in inherited entities by overriding OnTransformResetImpl.

	\param isCorrection [optional] Hint that the transform was reset due to its correction (e.g. by net-code),
	                    i.e. not a placement/teleport
	\param newVelocity  [optional] Initial velocity of the entity after the transform reset
	*/
	proto external void OnTransformReset(bool isCorrection = false, vector newVelocity = vector.Zero);

	// callbacks

	event protected void EOnDiag(IEntity owner, float timeSlice);
	/*!
	* Event when entity is activated.
	*/
	event protected void EOnActivate(IEntity owner);
	/*!
	* Event when entity is deactivated.
	*/
	event protected void EOnDeactivate(IEntity owner);
	//! Editor needs to know if there is any entity which manages this one and is editable
	event IEntity _WB_GetEditableOwner();
	//! Editor needs to have entity visible or not. Do not call editor API here!
	event void _WB_MakeVisible(IEntitySource src, bool visible);
	//! Editor changed transformation matrix source. This is the place to apply it on entity. Do not call editor API here!
	event void _WB_SetTransform(inout vector mat[4], IEntitySource src);
	//! Called always after entity creation. It's purpose is to prepare entity for editing. Do not edit anything through editor API here because it's too early for undoable actions! Use plain BaseContainer API for changes through src parameter if needed!
	event void _WB_OnInit(inout vector mat[4], IEntitySource src);
	//! Editor needs to know if this entity can to be deleted. Do not call editor API here!
	event bool _WB_CanDelete(IEntitySource src) { return true; };
	//! Editor needs to know if this entity can to be renamed. Do not call editor API here!
	event bool _WB_CanRename(IEntitySource src) { return true; };
	//! Editor needs to know if this entity can be copied. Do not call editor API here!
	event bool _WB_CanCopy(IEntitySource src) { return true; };
	//!	Editor needs to know whether this entity can be selected in scene window or not
	event bool _WB_CanSelect(IEntitySource src) { return true; };
	//! Returns how many anchor points does this entity have at this time
	event int _WB_GetAnchorCount(IEntitySource src);
	//! Fills position of anchor at index `index` to `anchorPosition`
	event void _WB_GetAnchor(inout vector position, IEntitySource src, int index);
	/*!
	Called when two entities are snapped together
	\param isReceiver True if another entity snapped to this one, false otherwise.
	*/
	event void _WB_OnAnchorSnapped(IEntitySource thisSrc, int thisAnchor, IEntitySource otherSrc, int otherAnchor, bool isReceiver);
	/*!
	\see WB_OnAnchorSnapped
	\return True if this entity agrees with the snap operation
	*/
	event bool _WB_CanAnchorSnap(IEntitySource thisSrc, int thisAnchor, IEntitySource otherSrc, int otherAnchor, bool isReceiver) { return true; };
	//! Editor needs to know a bound box of entity (For ray-casting, visualizers etc.). You can return any custom size you need. Do not call editor API here!
	event void _WB_GetBoundBox(inout vector min, inout vector max, IEntitySource src);
	//! Returns true if bound box should be visible when entity is hovered/selected in WB
	event bool _WB_ShouldShowBoundBox(IEntitySource src) { return true; };
	//! If entity needs to have a special visualizer instead of default one, here is the place where you can implement it. Do not call editor API here!
	event void _WB_SetExtraVisualiser(EntityVisualizerType type, IEntitySource src);
	//! An opportunity to append items into editor's "Entity" context menu. Do not call editor API here!
	event array<ref WB_UIMenuItem> _WB_GetContextMenuItems();
	//! Does this entity support editing transformation using physics simulation? Do not call editor API here!
	event bool _WB_OnPhysSimulPlacementBegin(IEntitySource src) { return true; };
	//! Prepare to edit transformation using physics simulation. Return previous status. Do not call editor API here!
	event bool _WB_EnablePhysics(IEntitySource src, bool physics);
	//! Possibility to get variable value choices dynamically. Do not call editor API here!
	event array<ref ParamEnum> _WB_GetUserEnums(string varName, IEntitySource src);
	//! Any property value has been changed. You can use editor API here and do some additional edit actions which will be part of the same "key changed" action.
	event bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent);
	//! Some "coords" or "angles" vector property is being to be edited. Entity can provide a world matrix which represents their local space and which is needed by editor to edit the key by a gizmo
	event bool _WB_GetKeySpaceMatrixWorld(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent, out vector transformSpaceWorld[4]);
	//! Called after updating world in Workbench. The entity must be visible in frustum, selected or named. You can use editor API here and do some edit actions if needed.
	event void _WB_AfterWorldUpdate(float timeSlice);
	//! Called after _WB_OnInit or also later when editor needs to know whether _WB_AfterWorldUpdate needs to be called and when. Return value can be either 0 (event will not be called at all) or any combination of EEntityFrameUpdateSpecs. Avoid CALL_ALWAYS flag whenever possible to prevent performance issues
	event int _WB_GetAfterWorldUpdateSpecs(IEntitySource src);
	//! User has chosen any of your menu item from editor's "Entity" menu which you have recently provided in WB_GetContextMenuItems(). You can use editor API here and do some edit actions.
	event void _WB_OnContextMenu(int id);
	//! User pressed a key and this entity is main member of entity selection. You can use editor API here and do some edit actions which will be undoable.
	event void _WB_OnKeyDown(int keyCode);
	//! Called after entity gets created in map during editing or when deleted entity gets restored after undo action. This event isn't called by loading map!!! If you need an event that is being called after every entity creation then use WB_OnInit instead it. You can use editor API here and do some additional edit actions which will be part of the same "create entity" action.
	event void _WB_OnCreate(IEntitySource src);
	//! Parent entity has been changed ( it's available through src->GetParent() ). prevParentSrc is a pointer to a previous parent (if any)
	event void _WB_OnParentChange(IEntitySource src, IEntitySource prevParentSrc);
	//! This entity is going to be deleted. You can use editor API here and do some additional edit actions which will be part of the same "entity delete" action.
	event void _WB_OnDelete(IEntitySource src);
	//! This entity has been renamed. You can use editor API here and do some additional edit actions which will be part of the same "entity rename" action.
	event void _WB_OnRename(IEntitySource src, string oldName);
	/*!
	Script-side implementation of OnTransformReset.

	The default implementation handles Particles (if present on the entity)
	and calls OnTransformReset on all of the entity's GenericComponent instances and child
	GenericEntity instances (i.e. it handles the whole entity and component hierarchy by recursion).

	Can be overridden to alter the default behavior.
	Usually, you'll want to call the base implementation somewhere in the override to preserve the recursion.

	\param params See the TransformResetParams documentation.
	*/
	event protected void OnTransformResetImpl(TransformResetParams params){ OnTransformResetImplNative(params); };
}

/*!
\}
*/
