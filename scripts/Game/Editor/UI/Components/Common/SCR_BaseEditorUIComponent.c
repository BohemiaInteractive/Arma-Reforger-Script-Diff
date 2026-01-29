/** @ingroup Editor_UI Editor_UI_Components
*/
class SCR_BaseEditorUIComponent: MenuRootSubComponent
{
	const int CANCEL_CLICK_MAX_DIS = 5; //--- Maximum distance in reference resolution pixels for RMB cancel action to be accepted
	const int CANCEL_CLICK_MAX_TIME = 500; //--- Maximum duration in miliseconds for RMB cancel action to be accepted
	
	protected vector m_vClickPosCancel;
	
	protected vector OnCancelDown()
	{
		/*
		SCR_ManualCamera camera = SCR_CameraEditorComponent.GetCameraInstance();
		if (camera)
		{
			SCR_ManualCameraParam param = camera.GetCameraParam();
			if (param && param.isManualInput)
				return vector.Zero;
		}
		*/
				
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return vector.Zero;
		
		ScriptCallQueue queue = GetGame().GetCallqueue();
		if (queue) queue.CallLater(OnCancelExpired, CANCEL_CLICK_MAX_TIME);
		
		int mouseX, mouseY;
		WidgetManager.GetMousePos(mouseX, mouseY);
		m_vClickPosCancel = Vector(workspace.DPIUnscale(mouseX), workspace.DPIUnscale(mouseY), 0);
		return m_vClickPosCancel;
	}
	protected bool OnCancelUp()
	{
		/*
		SCR_ManualCamera camera = SCR_CameraEditorComponent.GetCameraInstance();
		if (camera)
		{
			SCR_ManualCameraParam param = camera.GetCameraParam();
			if (param && param.isManualInput)
				return false;
		}
		*/
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return false;
		
		int mouseX, mouseY;
		WidgetManager.GetMousePos(mouseX, mouseY);
		vector mousePos = Vector(workspace.DPIUnscale(mouseX), workspace.DPIUnscale(mouseY), 0);
		return vector.Distance(m_vClickPosCancel, mousePos) < CANCEL_CLICK_MAX_DIS;
	}
	protected void OnCancelExpired()
	{
		m_vClickPosCancel = Vector(int.MAX, int.MAX, 0);
	}
};