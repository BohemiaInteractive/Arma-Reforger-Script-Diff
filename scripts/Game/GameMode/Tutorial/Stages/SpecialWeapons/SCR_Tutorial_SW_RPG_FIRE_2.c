[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_RPG_FIRE_2Class: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_RPG_FIRE_2 : SCR_BaseTutorialStage
{
	SCR_TutorialLogic_SW m_CourseLogic;
	bool m_bTargetDestroyed;
	SCR_DestructionMultiPhaseComponent m_DestructionComponent
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_CourseLogic = SCR_TutorialLogic_SW.Cast(m_TutorialComponent.GetActiveConfig().GetCourseLogic());
		if (!m_CourseLogic)
			return;
		
		if (m_CourseLogic.m_OnAmmoDepletedChanged)
			m_CourseLogic.m_OnAmmoDepletedChanged.Insert(OnAmmoChanged);
		
		IEntity target = GetGame().GetWorld().FindEntityByName("TARGET_4");
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
		
		if (m_StageInfo.GetStageName() != "RPG_TARGET")
			return;
		
		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", Math.RandomIntInclusive(17, 19));
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
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_bTargetDestroyed;
	}
}