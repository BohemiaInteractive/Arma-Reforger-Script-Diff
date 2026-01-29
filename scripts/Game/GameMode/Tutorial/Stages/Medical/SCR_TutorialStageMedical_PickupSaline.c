[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_PickupSalineClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_PickupSaline: SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		m_TutorialComponent.SpawnAsset("Saline", "{00E36F41CA310E2A}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_01.et");
		
		RegisterWaypoint("Saline", "", "PICKUP");
		PlayNarrativeCharacterStage("FIRSTAID_Instructor", 7);
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));

		if (storageManComp)	
		{
			storageManComp.m_OnItemAddedInvoker.Remove(OnItemAdded);
			storageManComp.m_OnItemAddedInvoker.Insert(OnItemAdded);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnItemAdded(IEntity item, BaseInventoryStorageComponent storageComponent)
	{
		SCR_ConsumableItemComponent consumItem = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		
		if (!consumItem)
			return true;
		
		if (consumItem.GetConsumableType() == SCR_EConsumableType.SALINE)
			UnregisterWaypoint("Saline");
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnVoiceoverEventFinished(string eventName)
	{
		if (eventName == "SOUND_TUTORIAL_TA_LEG_BANDAGED_INSTRUCTOR_H_02")
			PlayNarrativeCharacterStage("Victim", 4);
		else if (eventName == "SOUND_TUTORIAL_TA_LEG_BANDAGED_PARTICIPANT_01")
			PlayNarrativeCharacterStage("FIRSTAID_Instructor", 8);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (GetQuickslotIndexByPrefab("{00E36F41CA310E2A}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_01.et") >= 0)
			return true;

		SCR_CharacterInventoryStorageComponent inventory = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		if (!inventory)
			return false;
		
		IEntity ent = inventory.GetCurrentItem();
		
		if (!ent)
			return false;
		
		SCR_ConsumableItemComponent consumItem = SCR_ConsumableItemComponent.Cast(ent.FindComponent(SCR_ConsumableItemComponent));
		
		if (!consumItem)
			return false;
		
		return consumItem.GetConsumableType() == SCR_EConsumableType.SALINE;
	}
}