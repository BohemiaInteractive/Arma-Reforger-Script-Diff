[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_ManualCameraAttachHintCondition : SCR_BaseHintCondition
{
	//------------------------------------------------------------------------------------------------
	protected SCR_AttachManualCameraComponent GetComponent(Managed owner)
	{
		SCR_ManualCamera camera = SCR_ManualCamera.Cast(owner);
		if (camera)
			return SCR_AttachManualCameraComponent.Cast(camera.FindCameraComponent(SCR_AttachManualCameraComponent));
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnInitCondition(Managed owner)
	{
		SCR_AttachManualCameraComponent component = GetComponent(owner);
		if (component)
			component.GetOnAttachChange().Insert(Activate);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitCondition(Managed owner)
	{
		SCR_AttachManualCameraComponent component = GetComponent(owner);
		if (component)
			component.GetOnAttachChange().Remove(Activate);
	}
}
