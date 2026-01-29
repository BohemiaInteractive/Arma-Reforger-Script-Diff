[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_FLIGHT_03Class: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_FLIGHT_03 : SCR_BaseTutorialStage
{
	protected SCR_MapMarkerBase m_Marker;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_HELICOURSE_FLIGHT3");
		m_Marker = CreateMarkerCustom("WP_HELICOURSE_FLIGHT3", SCR_EScenarioFrameworkMarkerCustom.OBJECTIVE_MARKER, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
		
		PlayNarrativeCharacterStage("HELICOPTER_Copilot", 12);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_Heli_FLIGHT_03()
	{
		DeleteMarker(m_Marker);
	}
};