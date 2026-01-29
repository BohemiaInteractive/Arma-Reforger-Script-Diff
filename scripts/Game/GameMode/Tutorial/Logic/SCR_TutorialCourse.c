[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomTitleFlags(SCR_ETutorialCourses, "m_eCourseType")]
class SCR_TutorialCourse : Managed
{
	[Attribute(category: "Flow")]
	private ref array<ref SCR_TutorialStageInfo> m_aTutorialStageInfos;
	
	[Attribute(category: "Flow")]
	private ref array<ref SCR_TutorialStageInfo> m_aNonLinearStages;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ETutorialCourses), category: "Flow")]
	protected SCR_ETutorialCourses m_eCourseType;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_ETutorialCourses), category: "Flow")]
	protected SCR_ETutorialCourses m_eRequiredCourses;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, "Config with voice over data for tutorial stages and subtitles.", "conf", category: "Flow")]
	protected ResourceName m_sVoiceOverDataConfig;
	
	[Attribute(defvalue: "1", category: "Task")]
	protected bool m_bCreateTask;
	
	[Attribute(category: "Task")]
	protected string m_sTaskTitle;
	
	[Attribute(category: "Task")]
	protected string m_sTaskDescription;
	
	[Attribute(category: "Task")]
	protected string m_sTaskIcon;
	
	[Attribute(category: "Logic")]
	protected ref SCR_BaseTutorialCourseLogic m_CourseLogic;
	
	[Attribute(category: "Area restriction")]
	protected string m_sCourseAreaCenterEntityName;
	
	[Attribute(defvalue: "-1", category: "Area restriction")]
	protected float m_fWarningDistance;
	
	[Attribute(defvalue: "-1", category: "Area restriction")]
	protected float m_fCourseBreakDistance;
	
	[Attribute(category: "Fast Travel")]
	protected string m_sFastTravelPosition;
	
	[Attribute(category: "Entities")]
	private ref array<ref SCR_TutorialCoursePrefabInfo> m_aCourseEntities;
	
	protected ref ScriptInvokerInt s_OnCourseStarted;
	protected ref ScriptInvokerInt s_OnCourseFinished;

	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnCourseStarted()
	{
		if (!s_OnCourseStarted)
			s_OnCourseStarted = new ScriptInvokerInt();

		return s_OnCourseStarted;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnCourseFinished()
	{
		if (!s_OnCourseFinished)
			s_OnCourseFinished = new ScriptInvokerInt();

		return s_OnCourseFinished;
	}

	//------------------------------------------------------------------------------------------------
	string GetCourseAreaCenterEntityName()
	{
		return m_sCourseAreaCenterEntityName;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetCourseBreakDistance()
	{
		return m_fCourseBreakDistance;
	}

	//------------------------------------------------------------------------------------------------
	float GetWarningDistance()
	{
		return m_fWarningDistance;
	}
	
	//------------------------------------------------------------------------------------------------
	bool CanCreateTask()
	{
		return m_bCreateTask;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetFastTravelPosition()
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sFastTravelPosition))
			return null;
		
		return GetGame().GetWorld().FindEntityByName(m_sFastTravelPosition);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] TutorialArlandStages
	int GetPrefabs(out notnull array<SCR_TutorialCoursePrefabInfo> outArray)
	{
		foreach(SCR_TutorialCoursePrefabInfo ent : m_aCourseEntities)
		{
			outArray.Insert(ent);
		}
		
		return outArray.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ETutorialCourses GetRequiredCourses()
	{
		return m_eRequiredCourses;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetStageCount()
	{
		if (!m_aTutorialStageInfos)
			return 0;
		
		return m_aTutorialStageInfos.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTaskDescription()
	{
		return m_sTaskDescription;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTaskTitle()
	{
		return m_sTaskTitle;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetTaskIcon()
	{
		return m_sTaskIcon;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_TutorialStageInfo GetStageByName(string stageName)
	{
		foreach (SCR_TutorialStageInfo stage : m_aTutorialStageInfos)
		{
			if (stage.GetStageName() == stageName)
				return stage;
		}
		
		foreach (SCR_TutorialStageInfo stage : m_aNonLinearStages)
		{
			if (stage.GetStageName() == stageName)
				return stage;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseTutorialCourseLogic GetCourseLogic()
	{
		return m_CourseLogic;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnStart()
	{
		if (m_CourseLogic)
			m_CourseLogic.OnCourseStart();

		if (s_OnCourseStarted)
			s_OnCourseStarted.Invoke(m_eCourseType);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnFinish()
	{
		if (m_CourseLogic)
			m_CourseLogic.OnCourseEnd();

		if (s_OnCourseFinished)
			s_OnCourseFinished.Invoke(m_eCourseType);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_TutorialStageInfo GetStage(int index)
	{
		if (!m_aTutorialStageInfos || !m_aTutorialStageInfos.IsIndexValid(index))
			return null;
		
		return m_aTutorialStageInfos[index];
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ETutorialCourses GetCourseType()
	{
		return m_eCourseType;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] TutorialArlandStages
	void GetStages(out notnull array<ref SCR_TutorialStageInfo> TutorialArlandStages)
	{
		foreach(SCR_TutorialStageInfo stage : m_aTutorialStageInfos)
		{
			TutorialArlandStages.Insert(stage);
		}
		//TutorialArlandStages = m_aTutorialStageInfos;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetVoiceOverDataConfig()
	{
		return m_sVoiceOverDataConfig;
	}
}