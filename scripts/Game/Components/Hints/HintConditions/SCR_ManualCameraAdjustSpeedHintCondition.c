[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_ManualCameraAdjustSpeedHintCondition : SCR_BaseHintCondition
{
	//------------------------------------------------------------------------------------------------
	protected SCR_AdjustSpeedManualCameraComponent GetComponent(Managed owner)
	{
		SCR_ManualCamera camera = SCR_ManualCamera.Cast(owner);
		if (camera)
			return SCR_AdjustSpeedManualCameraComponent.Cast(camera.FindCameraComponent(SCR_AdjustSpeedManualCameraComponent));
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSpeedChange(float speed, bool isManualChange)
	{
		if (isManualChange)
			Activate();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnInitCondition(Managed owner)
	{
		SCR_AdjustSpeedManualCameraComponent component = GetComponent(owner);
		if (component)
			component.GetOnSpeedChange().Insert(OnSpeedChange);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitCondition(Managed owner)
	{
		SCR_AdjustSpeedManualCameraComponent component = GetComponent(owner);
		if (component)
			component.GetOnSpeedChange().Remove(OnSpeedChange);
	}
}
