[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_M72_FIREClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_M72_FIRE : SCR_BaseTutorialStage
{
	SCR_TutorialLogic_SW m_CourseLogic;
	bool m_bTargetDestroyed;
	SCR_DestructionMultiPhaseComponent m_DestructionComponent
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		if (SCR_TutorialLogic_SW.Cast(m_Logic).m_iMisses_M72 == 0)
			PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 7);

		m_CourseLogic = SCR_TutorialLogic_SW.Cast(m_TutorialComponent.GetActiveConfig().GetCourseLogic());
		if (!m_CourseLogic)
			return;
		
		if (m_CourseLogic.m_OnAmmoDepletedChanged)
			m_CourseLogic.m_OnAmmoDepletedChanged.Insert(OnAmmoChanged);
		
		IEntity target = GetGame().GetWorld().FindEntityByName("TARGET_3");
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
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTargetDamaged(EDamageState state)
	{
		if (state != EDamageState.DESTROYED)
			return;
		
		if (m_DestructionComponent)
			m_DestructionComponent.GetOnDamageStateChanged().Remove(OnTargetDamaged);

		m_bTargetDestroyed = true;
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
		if (m_bTargetDestroyed)
			return;
	
		int stageIndex = m_TutorialComponent.GetStageIndexByName("M72_DROP");
		if (stageIndex != -1)
		{
			m_TutorialComponent.SetStage(stageIndex);
			return;
		}
			
		m_TutorialComponent.InsertStage("M72_DROP");
		m_TutorialComponent.InsertStage("M72_PICKUP", m_TutorialComponent.GetStageIndexByName("M72_DROP")+1);
		m_TutorialComponent.InsertStage("M72_FIRE", m_TutorialComponent.GetStageIndexByName("M72_PICKUP")+1);
		m_bFinished = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (m_bTargetDestroyed)
		{
			PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 8);
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStageFinished()
	{
		GetGame().GetCallqueue().Remove(HitCheck);
		super.OnStageFinished();
	}
}