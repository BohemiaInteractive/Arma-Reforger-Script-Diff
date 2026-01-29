[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetVoiceOverActorEntity : SCR_ScenarioFrameworkGet
{
	[Attribute("0", UIWidgets.ComboBox, "Actor enum in VO system", "", ParamEnumArray.FromEnum(SCR_EVoiceoverActor))]
	protected SCR_EVoiceoverActor m_eActor;

	[Attribute(desc: "Entity playing the voiceover.")]
	protected ref SCR_ScenarioFrameworkGet m_ActorEntity;

	//------------------------------------------------------------------------------------------------
	SCR_EVoiceoverActor GetActor()
	{
		return m_eActor;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ScenarioFrameworkGet GetActorEntity()
	{
		return m_ActorEntity;
	}

}
