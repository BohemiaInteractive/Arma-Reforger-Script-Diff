[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Intro_MoveClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Intro_Move : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_VoiceoverSystem.GetInstance().RegisterActor(GetGame().GetWorld().FindEntityByName("Captain_Outside"));
		SCR_VoiceoverSystem.GetInstance().RegisterActor(GetGame().GetWorld().FindEntityByName("WCDrill"));
		SCR_VoiceoverSystem.GetInstance().RegisterActor(GetGame().GetWorld().FindEntityByName("WCMaggot"));
		SCR_VoiceoverSystem.GetInstance().RegisterActor(GetGame().GetWorld().FindEntityByName("Briefing_Officer"));
		SetPermanentNarrativeStage("Soldier_DoorGuard", 2);
		RegisterWaypoint("Captain_Outside");
	}
}