class SCR_TutorialOpenFastTravelInterface: ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		MenuManager menuMan = GetGame().GetMenuManager();
		if (!menuMan)
			return;
		
		menuMan.OpenMenu(ChimeraMenuPreset.TutorialFastTravel);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return false;
		
		if (tutorial.IsFastTraveling())
			return false;
		
		SCR_TutorialCourse course = tutorial.GetActiveConfig();
		if (!course || course.GetCourseType() != SCR_ETutorialCourses.FREE_ROAM & course.GetCourseType() != SCR_ETutorialCourses.OUTRO)
			return false;
		
		return true;
	}
}