[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_FLIGHT_01Class: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_FLIGHT_01 : SCR_BaseTutorialStage
{
	protected SCR_MapMarkerBase m_Marker;

	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_HELICOURSE_FLIGHT1");
		m_Marker = CreateMarkerCustom("WP_HELICOURSE_FLIGHT1", SCR_EScenarioFrameworkMarkerCustom.OBJECTIVE_MARKER, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);

		ChimeraWorld world = GetGame().GetWorld();
			
		if (world)
			world.GetMusicManager().Play("SOUND_HELICOPTER_FLYING_COURSE");
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_Heli_FLIGHT_01()
	{
		DeleteMarker(m_Marker);
	}
};