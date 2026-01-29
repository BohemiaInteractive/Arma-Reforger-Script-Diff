[BaseContainerProps()]
class SCR_AnalyticsDataCollectionTutorialCompletionModule : SCR_AnalyticsDataCollectionModule
{
	protected ref array<string> m_aTutorialCourseNames = {};
	protected ref array<string> m_aTutorialStartedCourses = {};
	protected ref array<string> m_aTutorialFinishedCourses = {};

	//------------------------------------------------------------------------------------------------
	override void InitModule()
	{
		// We only care about Tutorial Game Mode
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;

		SCR_TutorialCoursesConfig config = tutorial.GetCoursesConfig();
		if (!config)
			return;

		array<ref SCR_TutorialCourse> tutorialCourses = {};
		config.GetConfigs(tutorialCourses);

		if (tutorialCourses.IsEmpty())
			return;

		// Get names of all Tutorial Courses
		string courseName;
		foreach (SCR_TutorialCourse tut : tutorialCourses)
		{
			courseName = SCR_Enum.GetEnumName(SCR_ETutorialCourses, tut.GetCourseType());
			m_aTutorialCourseNames.Insert(courseName);
		}

		Enable();

		SCR_PlayerData playerData = SCR_PlayerData.GetPlayerData(GetGame().GetPlayerController().GetPlayerId());
		if (!playerData)
			return;

		playerData.dataEvent.array_tutorialstep_id = SCR_AnalyticsDataCollectionHelper.GetShortDataArray(m_aTutorialCourseNames.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCourseStarted(int courseId)
	{
		string courseName = SCR_Enum.GetEnumName(SCR_ETutorialCourses, courseId);
		if (m_aTutorialStartedCourses.Contains(courseName))
			return;

		m_aTutorialStartedCourses.Insert(courseName);

		SCR_PlayerData playerData = SCR_PlayerData.GetPlayerData(GetGame().GetPlayerController().GetPlayerId());
		if (!playerData)
			return;

		playerData.dataEvent.array_tutorialstep_id = SCR_AnalyticsDataCollectionHelper.GetShortDataArray(m_aTutorialCourseNames.ToString());
		playerData.dataEvent.array_tutorialstep_started = SCR_AnalyticsDataCollectionHelper.GetShortDataArray(m_aTutorialStartedCourses.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCourseFinished(int courseId)
	{
		string courseName = SCR_Enum.GetEnumName(SCR_ETutorialCourses, courseId);
		if (m_aTutorialFinishedCourses.Contains(courseName))
			return;

		m_aTutorialFinishedCourses.Insert(courseName);

		SCR_PlayerData playerData = SCR_PlayerData.GetPlayerData(GetGame().GetPlayerController().GetPlayerId());
		if (!playerData)
			return;

		playerData.dataEvent.array_tutorialstep_completed = SCR_AnalyticsDataCollectionHelper.GetShortDataArray(m_aTutorialFinishedCourses.ToString());
	}

	//------------------------------------------------------------------------------------------------
	override void Enable()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;

		SCR_TutorialCoursesConfig config = tutorial.GetCoursesConfig();
		if (!config)
			return;

		array<ref SCR_TutorialCourse> courses = {};
		config.GetConfigs(courses);

		if (courses.IsEmpty())
			return;

		foreach (SCR_TutorialCourse course : courses)
		{
			course.GetOnCourseStarted().Insert(OnCourseStarted);
			course.GetOnCourseFinished().Insert(OnCourseFinished);
		}

		super.Enable();
	}

	//------------------------------------------------------------------------------------------------
	override void Disable()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;

		SCR_TutorialCoursesConfig config = tutorial.GetCoursesConfig();
		if (!config)
			return;

		array<ref SCR_TutorialCourse> courses = {};
		config.GetConfigs(courses);

		if (courses.IsEmpty())
			return;

		foreach (SCR_TutorialCourse course : courses)
		{
			course.GetOnCourseStarted().Remove(OnCourseStarted);
			course.GetOnCourseFinished().Remove(OnCourseFinished);
		}

		super.Disable();
	}

	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void DrawContent()
	{
		DbgUI.Text("Started courses:");
		foreach (string courseName : m_aTutorialStartedCourses)
		{
			DbgUI.Text(courseName);
		}

		DbgUI.Spacer(16);

		DbgUI.Text("Finished courses:");
		foreach (string courseName : m_aTutorialFinishedCourses)
		{
			DbgUI.Text(courseName);
		}
	}
	#endif

	//------------------------------------------------------------------------------------------------
	protected void ~SCR_AnalyticsDataCollectionTutorialCompletionModule()
	{
		Disable();
	}
}
