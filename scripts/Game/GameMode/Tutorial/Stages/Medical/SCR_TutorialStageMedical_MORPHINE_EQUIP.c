[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_MORPHINE_EQUIPClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_MORPHINE_EQUIP: SCR_BaseTutorialStage
{
	protected SCR_CharacterInventoryStorageComponent m_PlayerInventoryManager;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_PlayerInventoryManager = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));

		if (m_TutorialComponent.GetStageIndexByName("PickupMorphine") == -1)
		{
			PlayNarrativeCharacterStage("FIRSTAID_Instructor", 9);
			GetGame().GetCallqueue().CallLater(HintFailsafe, 30000);
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
		if (eventName == "SOUND_TUTORIAL_TA_SALINE_ADMINISTERED_INSTRUCTOR_H_02")
		{
			PlayNarrativeCharacterStage("Victim", 5);
		}
		else if (eventName == "SOUND_TUTORIAL_TA_SALINE_ADMINISTERED_PARTICIPANT_01")
		{
			PlayNarrativeCharacterStage("FIRSTAID_Instructor", 10);
		}
		else if (eventName == "SOUND_TUTORIAL_TA_SALINE_ADMINISTERED_INSTRUCTOR_H_05")
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
		int slotId = GetQuickslotIndexByPrefab("{0D9A5DCF89AE7AA9}Prefabs/Items/Medicine/MorphineInjection_01/MorphineInjection_01.et");
		
		if (slotId < 0)
			return;
		
		SCR_HintManagerComponent.ShowHint(m_StageInfo.GetHint(slotId - 4), isSilent);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{	
		if (!m_PlayerInventoryManager)
			return false;
		
		IEntity ent = m_PlayerInventoryManager.GetCurrentItem();
		if (!ent)
			return false;
		
		SCR_ConsumableItemComponent consumItem = SCR_ConsumableItemComponent.Cast(ent.FindComponent(SCR_ConsumableItemComponent));
		
		return consumItem && consumItem.GetConsumableType() == SCR_EConsumableType.MORPHINE;
	}
};