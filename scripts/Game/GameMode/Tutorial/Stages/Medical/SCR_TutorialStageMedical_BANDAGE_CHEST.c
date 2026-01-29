[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_BANDAGE_CHESTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_BANDAGE_CHEST: SCR_BaseTutorialStage
{
	SCR_CharacterDamageManagerComponent m_DamageManager;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		IEntity victim = GetGame().GetWorld().FindEntityByName("Victim");
		if (!victim)
			return;
		
		m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
		
		
	}

	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (!m_DamageManager)
			return false;
		
		return !m_DamageManager.GetGroupDamageOverTime(ECharacterHitZoneGroup.UPPERTORSO, EDamageType.BLEEDING);
	}
};