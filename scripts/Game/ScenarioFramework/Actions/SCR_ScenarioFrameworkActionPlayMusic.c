[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionPlayMusic : SCR_ScenarioFrameworkActionBase
{	
	[Attribute(desc: "Music to play. Music manager has to be present in the world and music needs to be configured in acp file there.")]
	string m_sMusic;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;
		
		MusicManager musicManager = world.GetMusicManager();
		if (!musicManager)
			return;

		musicManager.Play(m_sMusic);	
	}
}