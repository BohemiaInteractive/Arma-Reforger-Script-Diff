[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_ManualCameraZoomHintCondition : SCR_BaseHintCondition
{
	//------------------------------------------------------------------------------------------------
	protected SCR_ZoomManualCameraComponent GetComponent(Managed owner)
	{
		SCR_ManualCamera camera = SCR_ManualCamera.Cast(owner);
		if (camera)
			return SCR_ZoomManualCameraComponent.Cast(camera.FindCameraComponent(SCR_ZoomManualCameraComponent));
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnInitCondition(Managed owner)
	{
		SCR_ZoomManualCameraComponent component = GetComponent(owner);
		if (component)
			component.GetOnZoomChange().Insert(Activate);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitCondition(Managed owner)
	{
		SCR_ZoomManualCameraComponent component = GetComponent(owner);
		if (component)
			component.GetOnZoomChange().Remove(Activate);
	}
}
