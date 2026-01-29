[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_RPG_FIREClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_RPG_FIRE : SCR_BaseTutorialStage
{
	SCR_TutorialLogic_SW m_CourseLogic;
	SCR_DestructionMultiPhaseComponent m_DestructionComponent;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_CourseLogic = SCR_TutorialLogic_SW.Cast(m_TutorialComponent.GetActiveConfig().GetCourseLogic());
		if (!m_CourseLogic)
			return;
		
		if (m_CourseLogic.m_OnAmmoDepletedChanged)
			m_CourseLogic.m_OnAmmoDepletedChanged.Insert(OnAmmoChanged);
		
		IEntity target = GetGame().GetWorld().FindEntityByName("TARGET_1");
		if (!target)
			return;
		
		IEntity destructible = target.GetChildren();
		if (!destructible)
			return;
		
		m_DestructionComponent = SCR_DestructionMultiPhaseComponent.Cast(destructible.FindComponent(SCR_DestructionMultiPhaseComponent));
		if (!m_DestructionComponent)
			return;
		
		m_DestructionComponent.EnableDamageHandling(true);
		m_DestructionComponent.GetOnDamageStateChanged().Insert(OnTargetDamaged);
		
		RegisterWaypoint(destructible, string.Empty, "EXPLOSIVES").ShowDistance(false);
		
		if (!m_CourseLogic.m_bRPGVoiceLinePlayed)
		{
			PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 9);
			m_CourseLogic.m_bRPGVoiceLinePlayed = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTargetDamaged(EDamageState state)
	{
		if (state != EDamageState.DESTROYED)
			return;
		
		if (m_DestructionComponent)
			m_DestructionComponent.GetOnDamageStateChanged().Remove(OnTargetDamaged);

		GetGame().GetCallqueue().Remove(HitCheck);
		
		m_bFinished = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnAmmoChanged()
	{
		m_CourseLogic.m_OnAmmoDepletedChanged.Remove(OnAmmoChanged);
		GetGame().GetCallqueue().CallLater(HitCheck, 2000, false);
	}
	
	//------------------------------------------------------------------------------------------------
	void HitCheck()
	{
		GetGame().GetCallqueue().Remove(HitCheck);
		
		m_bFinished = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_bFinished;
	}
}