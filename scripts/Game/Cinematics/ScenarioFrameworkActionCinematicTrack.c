[CinematicTrackAttribute(name:"ScenarioFramework action cinematic track", description:"Allows you to activate Scenario Framework actions defined on SCR_ScenarioFrameworkCinematicEntityComponent attached to CinematicEntity")]
class ScenarioFrameworkActionCinematicTrack : CinematicTrackBase
{
	
	[Attribute(defvalue: "", desc: "Name of action sequence defined in SCR_ScenarioFrameworkCinematicEntityComponent")]
	protected string m_sActionSequenceName;
	
	private World actualWorld;
	
	//------------------------------------------------------------------------------------------------
	[CinematicEventAttribute()]
	void Launch()
	{
		if (!GetGame().GetWorld())
			return; 
		
		IEntity entity;
		entity = actualWorld.FindEntityByName(GetSceneName());
		
		if (!entity)
			return;
		
		SCR_ScenarioFrameworkCinematicEntityComponent sfComponent = SCR_ScenarioFrameworkCinematicEntityComponent.Cast(entity.FindComponent(SCR_ScenarioFrameworkCinematicEntityComponent));
		if (sfComponent)
			sfComponent.ActivateAction(m_sActionSequenceName);
	}
	
	override void OnInit(World world)
	{		
		actualWorld = world;
	}
	
}