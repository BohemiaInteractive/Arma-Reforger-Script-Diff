[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_GETOUTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_GETOUT : SCR_BaseTutorialStage
{
	protected Vehicle m_Helicopter;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_Helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!m_Helicopter)
			return;
		
		PlayNarrativeCharacterStage("HELICOPTER_Copilot", 15);
		SetPermanentNarrativeStage("Soldier_Friend", 2);
		m_TutorialComponent.ChangeVehicleLockState(m_Helicopter, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Helicopter.GetPilot() != m_Player;
	}
};