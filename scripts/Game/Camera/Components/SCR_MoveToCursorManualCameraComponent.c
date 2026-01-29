//! @ingroup ManualCamera

//! Moving the camera to and away from the cursor position
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_MoveToCursorManualCameraComponent : SCR_BaseManualCameraComponent
{	
	[Attribute("2", UIWidgets.Auto, "")]	
	private float m_fSpeed;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isManualInputEnabled || param.isManualInput || !param.isCursorEnabled)
			return;
		
		float inputValue = GetInputManager().GetActionValue("ManualCameraMoveToCursor");
		if (inputValue == 0)
			return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();			
		if (!workspace)
			return;
		
		// Get mouse position
		int mouseX, mouseY;
		WidgetManager.GetMousePos(mouseX, mouseY);
		
		// Get move direction
		vector outDir;
		workspace.ProjScreenToWorld(workspace.DPIUnscale(mouseX), workspace.DPIUnscale(mouseY), outDir, GetGame().GetWorld());
		
		param.transform[3] = param.transform[3] + outDir * inputValue * m_fSpeed * Math.Sqrt(param.multiplier.Length());
		param.isDirty = true;
		param.isManualInput = true;
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		return true;
	}
}
