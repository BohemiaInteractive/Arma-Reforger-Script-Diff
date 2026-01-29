/*
===========================================
Do not modify, this script is generated
===========================================
*/

#ifdef WORKBENCH

/*!
\addtogroup WorkbenchAPI_Plugins
\{
*/

/*!
Script definition class for World Editor Tool
*/
class WorldEditorTool: Managed
{
	private void WorldEditorTool();
	private void ~WorldEditorTool();
	WorldEditorAPI m_API;

	//! key is of type `ModifierKey`
	static proto bool GetModifierKeyState(ModifierKey key);
	static proto void UpdatePropertyPanel();

	// callbacks

	event void OnKeyPressEvent(KeyCode key, bool isAutoRepeat);
	event void OnKeyReleaseEvent(KeyCode key, bool isAutoRepeat);
	event void OnEnterEvent();
	event void OnLeaveEvent();
	event void OnMouseMoveEvent(float x, float y);
	event void OnMouseDoubleClickEvent(float x, float y, WETMouseButtonFlag buttons);
	event void OnMousePressEvent(float x, float y, WETMouseButtonFlag buttons);
	event void OnMouseReleaseEvent(float x, float y, WETMouseButtonFlag buttons);
	event void OnWheelEvent(int delta);
	event void OnActivate();
	event void OnDeActivate();
	event void OnAfterLoadWorld();
	event void OnBeforeUnloadWorld();
}

/*!
\}
*/

#endif // WORKBENCH
