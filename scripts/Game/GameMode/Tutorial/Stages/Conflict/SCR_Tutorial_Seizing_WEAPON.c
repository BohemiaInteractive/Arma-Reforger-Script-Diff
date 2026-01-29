[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_WEAPONClass : SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Seizing_WEAPON : SCR_BaseTutorialStage
{
	SCR_TutorialLogic_SeizingCourse m_CastedLogic;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("Seizing_Weapon", "", "PICKUP");
		
		m_CastedLogic = SCR_TutorialLogic_SeizingCourse.Cast(m_Logic);
		
		PlayNarrativeCharacterStage("CONFLICT_Instructor_B", 3);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (!m_CastedLogic)
			return false;
		
		return m_CastedLogic.HasPlayerAnyFirearm();
	}
	
}