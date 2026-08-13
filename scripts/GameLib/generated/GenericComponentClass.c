/*
===========================================
Do not modify, this script is generated
===========================================
*/

class GenericComponentClass: EntityComponentPrefabData
{
	void GenericComponentClass(IEntityComponentSource componentSource, IEntitySource parentSource, IEntitySource prefabSource) {}

	//! Editor needs to have entity visible or not. Do not call editor API here!
	static event void _WB_MakeVisible(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc, bool visible);
	//! Editor changed transformation matrix source. This is the place to apply it on entity. Do not call editor API here!
	static event void _WB_SetTransform(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc, inout vector mat[4]);
	//! Called always after entity creation. It's purpose is to prepare entity for editing. Do not edit anything through editor API here because it's too early for undoable actions! Use plain BaseContainer API for changes through src parameter if needed!
	static event void _WB_OnInit(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc, inout vector mat[4]);
	//! Editor needs to know if this entity can to be deleted. Do not call editor API here!
	static event bool _WB_CanDelete(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc) { return true; };
	//! Editor needs to know if this entity can to be renamed. Do not call editor API here!
	static event bool _WB_CanRename(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc) { return true; };
	//! Editor needs to know if this entity can be copied. Do not call editor API here!
	static event bool _WB_CanCopy(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc) { return true; };
	//! Editor needs to know if this entity can change its translation. Do not call editor API here!
	static event bool _WB_CanTranslate(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc) { return true; };
	//! Editor needs to know if this entity can change its rotation. Do not call editor API here!
	static event bool _WB_CanRotate(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc) { return true; };
	//! Editor needs to know if this entity can change its scale. Do not call editor API here!
	static event bool _WB_CanScale(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc) { return true; };
	//!	Editor needs to know whether this entity can be selected in scene window or not
	static event bool _WB_CanSelect(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc) { return true; };
	//!	Editor needs to know whether this entity is traceable at selecting entities in scene
	static event bool _WB_IsTraceable(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc) { return true; };
	//! Returns true if bound box should be visible when entity is hovered/selected in WB
	static event bool _WB_ShouldShowBoundBox(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc) { return true; };
	//! If entity needs to have a special visualizer instead of default one, here is the place where you can implement it. Do not call editor API here!
	static event void _WB_SetExtraVisualiser(WorldEditorAPI api, IEntitySource src, EntityVisualizerType type);
	//! An opportunity to append items into editor's "Entity" context menu. Do not call editor API here!
	static event array<ref WB_UIMenuItem> _WB_GetContextMenuItems(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc);
	//! Does this entity support editing transformation using physics simulation? Do not call editor API here!
	static event bool _WB_OnPhysSimulPlacementBegin(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc) { return true; };
	//! Prepare to edit transformation using physics simulation. Return previous status. Do not call editor API here!
	static event bool _WB_EnablePhysics(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc, bool physics);
	//! Possibility to get variable value choices dynamically. Do not call editor API here!
	static event array<ref ParamEnum> _WB_GetUserEnums(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc, string varName);
	//! Any property value has been changed. You can use editor API here and do some additional edit actions which will be part of the same "key changed" action.
	static event bool _WB_OnKeyChanged(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc, string key, BaseContainerList ownerContainers, IEntitySource parent);
	//! Some "coords" or "angles" vector property is being to be edited. Entity can provide a world matrix which represents their local space and which is needed by editor to edit the key by a gizmo
	static event bool _WB_GetKeySpaceMatrixWorld(IEntity owner, BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent, out vector transformSpaceWorld[4]) { return false; };
	//! Called after updating world in Workbench. The entity must be visible in frustum, selected or named. You can use editor API here and do some edit actions if needed.
	static event void _WB_AfterWorldUpdate(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc, float timeSlice);
	//! Called after _WB_OnInit or also later when editor needs to know whether _WB_AfterWorldUpdate needs to be called and when. Return value can be either 0 (event will not be called at all) or any combination of EEntityFrameUpdateSpecs. Avoid CALL_ALWAYS flag whenever possible to prevent performance issues
	static event int _WB_GetAfterWorldUpdateSpecs(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc);
	//! User has chosen any of your menu item from editor's "Entity" menu which you have recently provided in WB_GetContextMenuItems(). You can use editor API here and do some edit actions.
	static event void _WB_OnContextMenu(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc, int id);
	//! User pressed a key and this entity is main member of entity selection. You can use editor API here and do some edit actions which will be undoable.
	static event void _WB_OnKeyDown(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc, int keyCode);
	//! Called after entity gets created in map during editing or when deleted entity gets restored after undo action. This event isn't called by loading map!!! If you need an event that is being called after every entity creation then use WB_OnInit instead it. You can use editor API here and do some additional edit actions which will be part of the same "create entity" action.
	static event void _WB_OnCreate(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc);
	//! Parent entity has been changed ( it's available through src->GetParent() ). prevParentSrc is a pointer to a previous parent (if any)
	static event void _WB_OnParentChange(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc, IEntitySource prevParentSrc);
	//! This entity is going to be deleted. You can use editor API here and do some additional edit actions which will be part of the same "entity delete" action.
	static event void _WB_OnDelete(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc);
	//! This entity has been renamed. You can use editor API here and do some additional edit actions which will be part of the same "entity rename" action.
	static event void _WB_OnRename(WorldEditorAPI api, IEntityComponentSource compSrc, IEntitySource entSrc, string oldName);
}
