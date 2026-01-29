[BaseContainerProps(configRoot: true)]
class SCR_TutorialCoursesConfig
{
	[Attribute()]
	private ref array<ref SCR_TutorialCourse> m_TutorialCourses;
	
	//------------------------------------------------------------------------------------------------
	SCR_TutorialCourse GetCourse(SCR_ETutorialCourses courseType)
	{
		foreach (SCR_TutorialCourse course : m_TutorialCourses)
		{
			if (course.GetCourseType() == courseType)
				return course;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] SCR_TutorialCoursesConfig
	void GetConfigs(out notnull array<ref SCR_TutorialCourse> tutorialCourses)
	{
		tutorialCourses = m_TutorialCourses;
	}
}