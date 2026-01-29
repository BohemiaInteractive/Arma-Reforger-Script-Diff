class SCR_TutorialInstructorComponentClass: ScriptComponentClass
{
}

class SCR_TutorialInstructorComponent : ScriptComponent
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETutorialCourses))]
	protected SCR_ETutorialCourses m_eStageConfig;
	
	protected SCR_TutorialGamemodeComponent m_TutorialComponent;
	protected bool m_bIsEnabled = true;
	
	//------------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		return m_bIsEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableCourse(bool enable)
	{
		m_bIsEnabled = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ETutorialCourses GetCourseType()
	{
		return m_eStageConfig;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetAssignedCourseName()
	{
		if (!m_TutorialComponent)
			return string.Empty;
		
		SCR_TutorialCoursesConfig coursesConfig = m_TutorialComponent.GetCoursesConfig();
		if (!coursesConfig)
			return string.Empty;
		
		if (!m_eStageConfig)
			return string.Empty;
		
		SCR_TutorialCourse course = coursesConfig.GetCourse(m_eStageConfig);
		if (!course)
			return string.Empty;
		
		return course.GetTaskTitle();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_TutorialComponent = SCR_TutorialGamemodeComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_TutorialGamemodeComponent));
		if (!m_TutorialComponent)
			return;

		SCR_VoiceoverSystem.GetInstance().RegisterActor(owner);		
		m_TutorialComponent.RegisterInstructor(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_TutorialInstructorComponent()
	{
		if (m_TutorialComponent)
			m_TutorialComponent.UnregisterInstructor(this);
	}
}