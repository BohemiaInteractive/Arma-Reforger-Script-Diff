[BaseContainerProps()]
class SCR_TutorialLogic_Mortars : SCR_BaseTutorialCourseLogic
{	
	//------------------------------------------------------------------------------------------------
	override void OnCourseStart()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		
		if (!gameMode)
			return;
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.Cast(gameMode.FindComponent(SCR_TutorialGamemodeComponent));
		
		if (!tutorial)
			return;
		
		tutorial.SpawnAsset("Mortar", "{8094D99689ABE241}Prefabs/Weapons/Mortars/M252/Mortar_M252.et");
	}
}