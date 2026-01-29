[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionStopAnimation : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to play animation on")]
	ref SCR_ScenarioFrameworkGet m_ActorGetter;
	
	[Attribute("false", UIWidgets.Auto)]
	bool m_bFast;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_ActorGetter, entity))
			return;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (!character)
			return;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return;
		
		controller.StopLoitering(m_bFast);
	}
}