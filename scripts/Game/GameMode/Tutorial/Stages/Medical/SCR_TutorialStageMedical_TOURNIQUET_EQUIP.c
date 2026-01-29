[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_TOURNIQUET_EQUIPClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_TOURNIQUET_EQUIP : SCR_BaseTutorialStage
{
	protected SCR_CharacterInventoryStorageComponent m_PlayerInventory;
	protected SCR_CharacterDamageManagerComponent m_DamageManager;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{	
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		m_PlayerInventory = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));

		if (m_TutorialComponent.GamepadUsed())
			HintGamepad();
		else
			HintKeyboard();
		
		IEntity victim = GetGame().GetWorld().FindEntityByName("Victim");
		if (!victim)
			return;
		
		m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!m_DamageManager)
			return;
	}

	//------------------------------------------------------------------------------------------------
	override void HintGamepad(bool isSilent = false)
	{
		SCR_HintManagerComponent.ShowHint(m_StageInfo.GetHint(6), isSilent);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HintKeyboard(bool isSilent = false)
	{
		int slotId = GetQuickslotIndexByPrefab("{D70216B1B2889129}Prefabs/Items/Medicine/Tourniquet_01/Tourniquet_US_01.et");
		
		if (slotId < 0)
			return;
		
		SCR_HintManagerComponent.ShowHint(m_StageInfo.GetHint(slotId - 4), isSilent);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (!m_PlayerInventory)
			return false;
		
		IEntity ent = m_PlayerInventory.GetCurrentItem();
		if (!ent)
			return false;
		
		SCR_ConsumableItemComponent consumItem = SCR_ConsumableItemComponent.Cast(ent.FindComponent(SCR_ConsumableItemComponent));
		if (!consumItem)
			return false;

		if (!m_DamageManager)
			return false;

		return consumItem.GetConsumableType() == SCR_EConsumableType.TOURNIQUET || m_DamageManager.GetGroupTourniquetted(ECharacterHitZoneGroup.LEFTLEG) || !m_DamageManager.GetGroupDamageOverTime(ECharacterHitZoneGroup.LEFTLEG, EDamageType.BLEEDING);
	}
};