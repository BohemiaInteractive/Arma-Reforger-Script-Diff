[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_TOURNIQUET_APPLYClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_TOURNIQUET_APPLY: SCR_BaseTutorialStage
{
	SCR_CharacterDamageManagerComponent m_DamageManager;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		IEntity victim = GetGame().GetWorld().FindEntityByName("Victim");
		if (!victim)
			return;
		
		m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!m_DamageManager)
			return;

		RegisterWaypoint(victim, "", "HEAL");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (!m_DamageManager)
			return false;
		
		return m_DamageManager.GetGroupTourniquetted(ECharacterHitZoneGroup.LEFTLEG) || !m_DamageManager.GetGroupDamageOverTime(ECharacterHitZoneGroup.LEFTLEG, EDamageType.BLEEDING);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStageFinished()
	{	
		if (!m_TutorialComponent.FindPrefabInPlayerInventory("{A81F501D3EF6F38E}Prefabs/Items/Medicine/FieldDressing_01/FieldDressing_US_01.et"))
			m_TutorialComponent.InsertStage("PickupBandage");
		
		super.OnStageFinished();
	}
};