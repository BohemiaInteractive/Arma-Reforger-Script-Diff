[BaseContainerProps()]
class SCR_TutorialLogic_LRS : SCR_BaseTutorialCourseLogic
{
	protected static const float MINIMUM_VIEW_DISTANCE = 800;
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseStart()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		tutorial.EnableArsenal("Ammobox3", true);
		tutorial.SetupTargets("TARGETS_BEACH", null, ETargetState.TARGET_DOWN, false);
		tutorial.SetupTargets("TARGETS_CLIFF", null, ETargetState.TARGET_DOWN, false);
		tutorial.SetupTargets("TARGETS_LAST", null, ETargetState.TARGET_DOWN, false);
		
		float viewDistance = GetGame().GetViewDistance();

		if (viewDistance < MINIMUM_VIEW_DISTANCE)
		{
			tutorial.SaveViewDistance(viewDistance);
			GetGame().SetViewDistance(MINIMUM_VIEW_DISTANCE);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseEnd()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		if (tutorial.GetSavedViewDistance() != 0)
			GetGame().SetViewDistance(tutorial.GetSavedViewDistance());
		
		tutorial.EnableArsenal("Ammobox3", false);
		tutorial.SetupTargets("TARGETS_BEACH", null, ETargetState.TARGET_UP, true);
		tutorial.SetupTargets("TARGETS_CLIFF", null, ETargetState.TARGET_UP, true);
		tutorial.SetupTargets("TARGETS_LAST", null, ETargetState.TARGET_UP, true);
	}
}