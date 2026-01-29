[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionCallAnimationCommand : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity (Entity has to have Character Animation componnent)")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Name of the animation command. E.g CMD_Gestures")]
	string m_sAnimationCommand;

	[Attribute(desc: "ID of animation can be found in animation graph.", params: "0 inf")]
	int m_iAnimId;

	[Attribute(SCR_EAnimationState.START.ToString(), UIWidgets.ComboBox, "Set if the animation should start or stop.", "", enumType: SCR_EAnimationState)]
	SCR_EAnimationState m_eState;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;

		SCR_CharacterAnimationComponent animationComponent = SCR_CharacterAnimationComponent.Cast(entity.FindComponent(SCR_CharacterAnimationComponent));
		if (!animationComponent)
		{
			if (object)
				PrintFormat("ScenarioFramework Action: Animation component not found for Action %1 attached on %2.", this, object.GetName(), level: LogLevel.ERROR);
			else
				PrintFormat("ScenarioFramework Action: Animation component not found for Action %1.", this, level: LogLevel.ERROR);

			return;
		}

		TAnimGraphCommand commandId = animationComponent.BindCommand(m_sAnimationCommand);

		switch (m_eState)
		{
			case SCR_EAnimationState.START:
			{
				animationComponent.CallCommand4I(commandId, 0, m_iAnimId, 0, 0, 0);
				break;
			}
			case SCR_EAnimationState.STOP:
			{
				animationComponent.CallCommand(commandId, -1, m_iAnimId);
				break;
			}
		}
	}
}
