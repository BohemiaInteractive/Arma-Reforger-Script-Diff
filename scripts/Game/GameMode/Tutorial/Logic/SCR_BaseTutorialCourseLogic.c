[BaseContainerProps()]
class SCR_BaseTutorialCourseLogic : Managed
{
	//------------------------------------------------------------------------------------------------
	//! Called from SCR_TutorialGamemodeComponent on Start of respective course
	void OnCourseStart(){};
	
	//------------------------------------------------------------------------------------------------
	//! Called from SCR_TutorialGamemodeComponent on End of respective course
	void OnCourseEnd(){};
}