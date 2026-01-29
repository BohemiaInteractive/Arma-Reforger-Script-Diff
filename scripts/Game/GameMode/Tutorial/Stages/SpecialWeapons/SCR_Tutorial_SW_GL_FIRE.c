[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_GL_FIREClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_GL_FIRE : SCR_BaseTutorialStage
{
	SCR_DestructionMultiPhaseComponent m_DestructionComponent
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		IEntity target = GetGame().GetWorld().FindEntityByName("TARGET_2");
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
		
		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 23);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTargetDamaged(EDamageState state)
	{
		if (state != EDamageState.DESTROYED)
			return;
		
		if (m_DestructionComponent)
			m_DestructionComponent.GetOnDamageStateChanged().Remove(OnTargetDamaged);

		m_bFinished = true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_SW_GL_FIRE()
	{
		if (m_TutorialComponent)
			m_TutorialComponent.EnableArsenal("SW_ARSENAL_AMMO", false);
	}
}