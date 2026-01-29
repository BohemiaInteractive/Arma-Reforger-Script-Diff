[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionPlayCinematic : SCR_ScenarioFrameworkActionBase
{	
	[Attribute(desc: "Cinematic to play.")]
	string m_sCinematicTimeline;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;
		
		CinematicEntity cineToPlay = CinematicEntity.Cast(world.FindEntityByName(m_sCinematicTimeline));
		if (cineToPlay)
			cineToPlay.Play();
	}
}