[BaseContainerProps(), SCR_BaseContainerHintCondition()]
class SCR_ManualCameraLightHintCondition : SCR_BaseHintCondition
{
	//------------------------------------------------------------------------------------------------
	protected SCR_LightManualCameraComponent GetComponent(Managed owner)
	{
		SCR_ManualCamera camera = SCR_ManualCamera.Cast(owner);
		if (camera)
			return SCR_LightManualCameraComponent.Cast(camera.FindCameraComponent(SCR_LightManualCameraComponent));
		else
			return null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnInitCondition(Managed owner)
	{
		SCR_LightManualCameraComponent component = GetComponent(owner);
		if (component)
			component.GetOnLightChanged().Insert(Activate);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnExitCondition(Managed owner)
	{
		SCR_LightManualCameraComponent component = GetComponent(owner);
		if (component)
			component.GetOnLightChanged().Remove(Activate);
	}
}
