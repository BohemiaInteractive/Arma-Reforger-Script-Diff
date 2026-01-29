[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_PULLOUTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_PULLOUT: SCR_BaseTutorialStage
{
	protected Vehicle m_Jeep;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("Victim", "", "HEAL");
		PlayNarrativeCharacterStage("FIRSTAID_Instructor", 2);
	
		m_Jeep = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("AccidentJeep"));
	}

	//------------------------------------------------------------------------------------------------	
	override protected bool GetIsFinished()
	{		
		return m_Jeep && !m_Jeep.GetPilot() && GetDuration() > 2000;
	}
};