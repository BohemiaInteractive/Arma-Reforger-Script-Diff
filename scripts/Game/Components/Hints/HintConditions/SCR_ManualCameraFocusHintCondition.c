[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_ManualCameraFocusHintCondition : SCR_BaseHintCondition
{
	//------------------------------------------------------------------------------------------------
	protected SCR_FocusManualCameraComponent GetComponent(Managed owner)
	{
		SCR_ManualCamera camera = SCR_ManualCamera.Cast(owner);
		if (camera)
			return SCR_FocusManualCameraComponent.Cast(camera.FindCameraComponent(SCR_FocusManualCameraComponent));
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnInitCondition(Managed owner)
	{
		SCR_FocusManualCameraComponent component = GetComponent(owner);
		if (component)
			component.GetOnFocusChange().Insert(Activate);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitCondition(Managed owner)
	{
		SCR_FocusManualCameraComponent component = GetComponent(owner);
		if (component)
			component.GetOnFocusChange().Remove(Activate);
	}
}
