//! @ingroup ManualCamera

//! Editor control of camera GUI visibility.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_EditorToggleUIManualCameraComponent : SCR_BaseManualCameraComponent
{
	//------------------------------------------------------------------------------------------------
	protected void OnVisibilityChange(bool isVisible)
	{
		Widget widget = GetCameraEntity().GetWidget();
		if (widget)
			widget.SetVisible(isVisible);
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		SCR_MenuEditorComponent menuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		if (!menuManager)
		{
			SetEnabled(false);
			return false;
		}
		
		menuManager.GetOnVisibilityChange().Insert(OnVisibilityChange);
		OnVisibilityChange(menuManager.IsVisible());
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		SCR_MenuEditorComponent menuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		if (!menuManager)
			return;
		
		menuManager.GetOnVisibilityChange().Remove(OnVisibilityChange);
	}
}
