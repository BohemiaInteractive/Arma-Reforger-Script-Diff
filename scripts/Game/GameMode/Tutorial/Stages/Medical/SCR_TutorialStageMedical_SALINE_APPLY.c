[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_SALINE_APPLYClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_SALINE_APPLY: SCR_BaseTutorialStage
{
	protected SCR_CharacterDamageManagerComponent m_DamageManager;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		IEntity victim = GetGame().GetWorld().FindEntityByName("Victim");
		if (!victim)
			return;
		
		RegisterWaypoint(victim, "", "HEAL").SetOffsetVector("0 0.5 0");;
		
		m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!m_DamageManager)
			return;
		
		SCR_TutorialLogic_Medical logic = SCR_TutorialLogic_Medical.Cast(m_Logic);
		if (logic)
			GetGame().GetCallqueue().Remove(logic.RefreshVictimBloodLevel);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (!m_DamageManager)
			return false;

		return m_DamageManager.GetGroupSalineBagged(ECharacterHitZoneGroup.RIGHTARM) || m_DamageManager.GetGroupSalineBagged(ECharacterHitZoneGroup.LEFTARM);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStageFinished()
	{
		if (!m_TutorialComponent.FindPrefabInPlayerInventory("{0D9A5DCF89AE7AA9}Prefabs/Items/Medicine/MorphineInjection_01/MorphineInjection_01.et"))
			m_TutorialComponent.InsertStage("PickupMorphine");
		
		super.OnStageFinished();
	}
};