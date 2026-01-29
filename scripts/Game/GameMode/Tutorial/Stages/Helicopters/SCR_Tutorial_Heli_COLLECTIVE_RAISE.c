[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_COLLECTIVE_RAISEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_COLLECTIVE_RAISE : SCR_BaseTutorialStage
{
	protected SignalsManagerComponent m_SignalsManagerComponent;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		Vehicle helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!helicopter)
			return;
		
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(helicopter.FindComponent(SignalsManagerComponent));
		if (m_SignalsManagerComponent.GetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("Altitude")) >= 90)
		{
			m_bFinished = true;
			return;
		}
		
		PlayNarrativeCharacterStage("HELICOPTER_Copilot", 3);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnVoiceoverEventFinished(string eventName)
	{
		if (eventName == "SOUND_TUTORIAL_TA_RAISE_COLLECTIVE_INSTRUCTOR_J_01")
			PlayNarrativeCharacterStage("HELICOPTER_Copilot", 4);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SignalsManagerComponent)
			return false;

		return m_SignalsManagerComponent.GetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("Altitude")) >= 90 && (m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_COLLECTIVE_RAISED_INSTRUCTOR_J_03" || GetDuration() > 19000);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		SCR_TutorialLogic_Heli logic = SCR_TutorialLogic_Heli.Cast(m_TutorialComponent.GetActiveConfig().GetCourseLogic());
		if (logic)
			logic.m_bCheckAltitude = true;
		
		super.OnStageFinished();
	}
};