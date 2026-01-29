[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_BANDAGE_EQUIPClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_BANDAGE_EQUIP: SCR_BaseTutorialStage
{
	protected SCR_CharacterInventoryStorageComponent m_PlayerInventory;
	protected SCR_CharacterDamageManagerComponent m_DamageManager;

	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		m_PlayerInventory = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		IEntity victim = GetGame().GetWorld().FindEntityByName("Victim");
		if (victim)
			m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(victim.FindComponent(SCR_CharacterDamageManagerComponent));
		
		if (m_TutorialComponent.GetStageIndexByName("PickupBandage") == -1)
		{
			PlayNarrativeCharacterStage("FIRSTAID_Instructor", 5);
			GetGame().GetCallqueue().CallLater(HintFailsafe, 12000);
		}
		else
		{
			if (m_TutorialComponent.GamepadUsed())
				HintGamepad();
			else
				HintKeyboard();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnVoiceoverEventFinished(string eventName)
	{
		if (eventName == "SOUND_TUTORIAL_EX_NO_BANDAGES_INSTRUCTOR_H_01")
		{
			if (m_TutorialComponent.GamepadUsed())
				HintGamepad();
			else
				HintKeyboard();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void HintFailsafe()
	{
		if (SCR_HintManagerComponent.GetInstance().GetCurrentHint())
			return;
		
		if (m_TutorialComponent.GamepadUsed())
			HintGamepad();
		else
			HintKeyboard();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HintGamepad(bool isSilent = false)
	{
		SCR_HintManagerComponent.ShowHint(m_StageInfo.GetHint(6), isSilent);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HintKeyboard(bool isSilent = false)
	{
		int slotId = GetQuickslotIndexByPrefab("{A81F501D3EF6F38E}Prefabs/Items/Medicine/FieldDressing_01/FieldDressing_US_01.et");
		
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
		
		return consumItem.GetConsumableType() == SCR_EConsumableType.BANDAGE || !m_DamageManager.GetGroupDamageOverTime(ECharacterHitZoneGroup.UPPERTORSO, EDamageType.BLEEDING);
	}
};