[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkActionSetVehicleCruiseSpeed : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity to set max cruise speed.")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(defvalue: "50", uiwidget: UIWidgets.EditBox, desc: "Max cruise speed of AI if driving a given entity in km/h.", params: "0 inf 0.5", category: "Common")]
	float m_fMaxCruiseSpeed;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		AICarMovementComponent carMovementComp = AICarMovementComponent.Cast(entity.FindComponent(AICarMovementComponent));
		if (!carMovementComp)
			return;

		carMovementComp.SetCruiseSpeed(m_fMaxCruiseSpeed);
	}
}
