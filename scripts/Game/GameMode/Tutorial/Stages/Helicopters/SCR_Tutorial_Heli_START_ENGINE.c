[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_START_ENGINEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_START_ENGINE : SCR_BaseTutorialStage
{
	protected SCR_HelicopterControllerComponent m_HeliController;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		Vehicle helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!helicopter)
			return;
		
		SCR_VoiceoverSystem.GetInstance().RegisterActor(GetGame().GetWorld().FindEntityByName("HELICOPTER_Copilot"));
		PlayNarrativeCharacterStage("HELICOPTER_Copilot", 1);
		
		m_HeliController = SCR_HelicopterControllerComponent.Cast(helicopter.FindComponent(SCR_HelicopterControllerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_HeliController)
			return false;
		
		return m_HeliController.IsEngineOn();
	}
};