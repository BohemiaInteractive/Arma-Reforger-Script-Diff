class SCR_TutorialReturnToMenuAction: ScriptedUserAction
{
	protected SCR_TutorialGamemodeComponent m_TutorialComponent;
	protected bool m_Disabled;
	
	//------------------------------------------------------------------------------------------------
	override event void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		m_TutorialComponent.PlayBedAnimation(false);
		m_TutorialComponent.Fadeout(true).GetOnCompleted().Insert(EndTutorial);
	
		m_Disabled = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void EndTutorial()
	{
		ChimeraWorld world = GetGame().GetWorld();
		world.PauseGameTime(false);
		
		GameStateTransitions.RequestGameplayEndTransition();
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool CanBeShownScript(IEntity user)
	{
		if (!m_TutorialComponent)
			m_TutorialComponent = SCR_TutorialGamemodeComponent.GetInstance();
		
		if (!m_Disabled && m_TutorialComponent)
			return SCR_Enum.HasFlag(m_TutorialComponent.GetFinishedCourses(), SCR_ETutorialCourses.OUTRO);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override event bool GetActionNameScript(out string outName)
	{
		outName = "#AR-Tutorial_UserAction_FinishTraining";
		return true;
	}
}