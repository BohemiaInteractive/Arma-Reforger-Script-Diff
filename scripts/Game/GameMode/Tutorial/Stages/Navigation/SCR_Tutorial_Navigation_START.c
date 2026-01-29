[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_STARTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_START : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_TutorialComponent.SpawnAsset("SmallJeep2", "{6B24D5AFD884D64C}Prefabs/Vehicles/Wheeled/M998/M998_MERDC.et");
		
		PlayNarrativeCharacterStage("NAVIGATION_Instructor", 2);

		CreateMarkerCustom("NAVIGATION_Instructor", SCR_EScenarioFrameworkMarkerCustom.POINT_OF_INTEREST, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
	}
};