[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_AUTOHOVER_OFFClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_AUTOHOVER_OFF : SCR_BaseTutorialStage
{
	protected SCR_HelicopterControllerComponent m_HelicopterController;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		SCR_TutorialLogic_Heli logic = SCR_TutorialLogic_Heli.Cast(m_TutorialComponent.GetActiveConfig().GetCourseLogic());
		if (logic)
			logic.m_bForceAutohover = false;
		
		Vehicle helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!helicopter)
			return;

		m_HelicopterController = SCR_HelicopterControllerComponent.Cast(helicopter.FindComponent(SCR_HelicopterControllerComponent));
		
		PlayNarrativeCharacterStage("HELICOPTER_Copilot", 10);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_HelicopterController)
			return false;
		
		return !m_HelicopterController.GetAutohoverEnabled();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		SCR_TutorialLogic_Heli logic = SCR_TutorialLogic_Heli.Cast(m_TutorialComponent.GetActiveConfig().GetCourseLogic());
		if (logic)
			logic.m_bCheckAngle = true;
		
		super.OnStageFinished();
	}
};