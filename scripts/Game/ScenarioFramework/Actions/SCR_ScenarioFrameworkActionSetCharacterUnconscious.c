[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetCharacterUnconscious : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to play animation on")]
	ref SCR_ScenarioFrameworkGet m_CharacterGetter;

	[Attribute(defvalue: "1", desc: "Set Unconscious")]
	bool m_bSetUnconscious;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_CharacterGetter, entity))
			return;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (!character)
			return;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!controller)
			return;
		
		controller.SetUnconscious(m_bSetUnconscious);
	}
}