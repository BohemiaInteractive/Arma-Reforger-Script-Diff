[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_GETIN_STATICClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_GETIN_STATIC : SCR_BaseTutorialStage
{
	protected IEntity m_Turret;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		m_Turret = GetGame().GetWorld().FindEntityByName("M2");
		if (m_Turret)
			RegisterWaypoint(m_Turret, "", "GETIN");

		PlayNarrativeCharacterStage("SHOOTINGRANGE_Instructor", 13);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_Turret)
			return false;
		
		return m_Player.GetParent() == m_Turret;
	}
};