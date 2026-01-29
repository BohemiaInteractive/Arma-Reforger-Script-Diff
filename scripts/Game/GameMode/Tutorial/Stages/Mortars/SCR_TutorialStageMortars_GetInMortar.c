[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMortars_GetInMortarClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMortars_GetInMortar : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override void Setup()
	{
		RegisterWaypoint("WP_MORTAR_GETIN", "", "GETIN");
		
		CreateMarkerCustom(GetGame().GetWorld().FindEntityByName("FirstTarget"), SCR_EScenarioFrameworkMarkerCustom.CIRCLE, SCR_EScenarioFrameworkMarkerCustomColor.RED);
		
		PlayNarrativeCharacterStage("FIRESUPPORT_Instructor", 1);
		
		CreateMarkerCustom("FIRESUPPORT_Instructor", SCR_EScenarioFrameworkMarkerCustom.POINT_OF_INTEREST, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return CompartmentAccessComponent.GetVehicleIn(SCR_PlayerController.GetLocalControlledEntity()) == GetGame().GetWorld().FindEntityByName("Mortar");
	}
};