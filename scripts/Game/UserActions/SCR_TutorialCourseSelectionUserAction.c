class SCR_TutorialCourseSelectionUserAction: ScriptedUserAction
{	
	protected SCR_TutorialGamemodeComponent m_TutorialComponent;
	protected SCR_TutorialInstructorComponent m_TutorialInstructor;
	protected LocalizedString m_sCompletedSuffix = " #AR-Tutorial_TaskCompletedSuffix";
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity) 
	{		
		BaseGameMode gamemode = GetGame().GetGameMode();
		if (!gamemode)
			return;
		
		SCR_TutorialGamemodeComponent tutorialComponent = SCR_TutorialGamemodeComponent.Cast(gamemode.FindComponent(SCR_TutorialGamemodeComponent));
		if (!tutorialComponent)
			return;
		
		tutorialComponent.SetCourseConfig(m_TutorialInstructor.GetCourseType());
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (!m_TutorialInstructor)
			return false;
		
		if (!m_TutorialComponent)
		{
			BaseGameMode gamemode = GetGame().GetGameMode();
			if (!gamemode)
				return false;
			
			m_TutorialComponent = SCR_TutorialGamemodeComponent.Cast(gamemode.FindComponent(SCR_TutorialGamemodeComponent));
		}
		
		if (m_TutorialComponent && m_TutorialComponent.IsCourseAvailable(m_TutorialInstructor.GetCourseType()))
			return true;
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBroadcastScript()
	{		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool GetActionNameScript(out string outName)
	{
		if (!m_TutorialComponent || !m_TutorialInstructor)
			return false;
		
		if (!SCR_Enum.HasFlag(m_TutorialComponent.GetFinishedCourses(), m_TutorialInstructor.GetCourseType()))
			return false;
		
		outName = GetUIInfo().GetName() + m_sCompletedSuffix;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user) 
	{ 
		if (!m_TutorialComponent)
		{
			BaseGameMode gamemode = GetGame().GetGameMode();
			if (!gamemode)
				return false;
			
			m_TutorialComponent = SCR_TutorialGamemodeComponent.Cast(gamemode.FindComponent(SCR_TutorialGamemodeComponent));
		}
		
		if (m_TutorialComponent.IsCourseBreaking())
			return false;
		
		m_TutorialInstructor = SCR_TutorialInstructorComponent.Cast(GetOwner().FindComponent(SCR_TutorialInstructorComponent));
		if (!m_TutorialInstructor || !m_TutorialInstructor.IsEnabled() || !m_TutorialComponent)
			return false;
		
		if (m_TutorialComponent && m_TutorialComponent.GetOngoingFadeAnimation())
			return false;
		
		SCR_TutorialCourse currentCourse = m_TutorialComponent.GetActiveConfig();
		if (!currentCourse || currentCourse.GetCourseType() != SCR_ETutorialCourses.FREE_ROAM)
			return false;
		
		SCR_BaseTutorialStage stage = m_TutorialComponent.GetCurrentStage();
		if (!stage.IsInherited(SCR_TutorialStageHub))
			return false;
		
		return true; 
	};
}