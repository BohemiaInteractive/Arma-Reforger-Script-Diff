[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_AUTOHOVER_ONClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_AUTOHOVER_ON : SCR_BaseTutorialStage
{
	protected SCR_HelicopterControllerComponent m_HelicopterController;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		Vehicle helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!helicopter)
			return;
		
		m_HelicopterController = SCR_HelicopterControllerComponent.Cast(helicopter.FindComponent(SCR_HelicopterControllerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_HelicopterController)
			return false;
		
		return m_HelicopterController.GetAutohoverEnabled();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		SCR_TutorialLogic_Heli logic = SCR_TutorialLogic_Heli.Cast(m_TutorialComponent.GetActiveConfig().GetCourseLogic());
		if (logic)
			logic.m_bForceAutohover = true;
		
		super.OnStageFinished();
	}
};