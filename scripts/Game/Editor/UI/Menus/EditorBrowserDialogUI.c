/** @ingroup Editor_UI Editor_UI_Menus
*/
class EditorBrowserDialogUI: EditorMenuBase
{
	const string WIDGET_BUTTON_CLOSE = "CloseButton";
	
	protected void CloseWithoutPlacing()
	{
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true, true));
		if (placingManager)
		{
			placingManager.SetPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER, false);
			placingManager.SetInstantPlacing(null);
		}
		
		CloseSelf();
	}
	
	override void OnMenuOpen()
	{
		Widget rootWidget = GetRootWidget();
		if (!rootWidget) return;
		
		ScriptInvoker onClose = ButtonActionComponent.GetOnAction(rootWidget, WIDGET_BUTTON_CLOSE);
		if (onClose) onClose.Insert(CloseWithoutPlacing);
		
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true, true));
		if (placingManager)
		{
			SCR_EditorManagerEntity editorManager = placingManager.GetManager();
			if (editorManager)
				editorManager.GetOnDeactivate().Insert(CloseWithoutPlacing);
		}
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
			inputManager.AddActionListener("EditorContentBrowserClose", EActionTrigger.DOWN, CloseWithoutPlacing);
	}
	override void OnMenuClose()
	{
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true, true));
		if (placingManager)
		{			
			SCR_EditorManagerEntity editorManager = placingManager.GetManager();
			if (editorManager)
				editorManager.GetOnDeactivate().Remove(CloseWithoutPlacing);
		}
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
			inputManager.RemoveActionListener("EditorContentBrowserClose", EActionTrigger.DOWN, CloseWithoutPlacing);
	}
};