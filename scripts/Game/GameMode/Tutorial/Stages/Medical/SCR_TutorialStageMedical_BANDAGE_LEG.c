[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_BANDAGE_LEGClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_BANDAGE_LEG: SCR_BaseTutorialStage
{
	protected SCR_CharacterDamageManagerComponent m_DamageManager;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		IEntity victim = GetGame().GetWorld().FindEntityByName("Victim");
		if (!victim)
			return;
		
		m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!m_DamageManager)
			return;
		
		RegisterWaypoint(victim, "", "HEAL").SetOffsetVector("0 0.5 0");
		PlayNarrativeCharacterStage("FIRSTAID_Instructor", 6);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnVoiceoverEventFinished(string eventName)
	{
		if (eventName == "SOUND_TUTORIAL_TA_CHEST_BANDAGED_INSTRUCTOR_H_01")
			PlayNarrativeCharacterStage("Victim", 3);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (!m_DamageManager)
			return false;
		
		return !m_DamageManager.GetGroupDamageOverTime(ECharacterHitZoneGroup.LEFTLEG, EDamageType.BLEEDING) && !m_DamageManager.GetGroupTourniquetted(ECharacterHitZoneGroup.LEFTLEG);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStageFinished()
	{
		if (!m_TutorialComponent.FindPrefabInPlayerInventory("{00E36F41CA310E2A}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_01.et"))
			m_TutorialComponent.InsertStage("PickupSaline");
		
		super.OnStageFinished();
	}
};