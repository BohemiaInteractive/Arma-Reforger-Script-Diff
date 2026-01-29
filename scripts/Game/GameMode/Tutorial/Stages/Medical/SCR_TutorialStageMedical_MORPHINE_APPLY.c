[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_MORPHINE_APPLYClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_MORPHINE_APPLY: SCR_BaseTutorialStage
{
	protected IEntity m_Figurant;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_Figurant = GetGame().GetWorld().FindEntityByName("Victim");
		if (!m_Figurant)
			return;
		
		RegisterWaypoint(m_Figurant, "", "HEAL").SetOffsetVector("0 0.5 0");
		
		SCR_TutorialLogic_Medical logic = SCR_TutorialLogic_Medical.Cast(m_Logic);
		if (logic)
			GetGame().GetCallqueue().Remove(logic.RefreshVictimResilience);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		int reason;
		SCR_ConsumableMorphine consumableEffect = new SCR_ConsumableMorphine;
		consumableEffect.CanApplyEffect(m_Figurant, m_Player, reason); 
		
		return reason == SCR_EConsumableFailReason.ALREADY_APPLIED;
	}
};