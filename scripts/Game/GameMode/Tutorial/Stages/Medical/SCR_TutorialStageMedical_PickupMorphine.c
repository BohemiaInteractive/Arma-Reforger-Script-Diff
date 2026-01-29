[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_PickupMorphineClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_PickupMorphine: SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		m_TutorialComponent.SpawnAsset("Morphine", "{0D9A5DCF89AE7AA9}Prefabs/Items/Medicine/MorphineInjection_01/MorphineInjection_01.et");
		
		RegisterWaypoint("Morphine", "", "PICKUP");
		PlayNarrativeCharacterStage("FIRSTAID_Instructor", 9);
		
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
		
		if (consumItem.GetConsumableType() == SCR_EConsumableType.MORPHINE)
			UnregisterWaypoint("Morphine");
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnVoiceoverEventFinished(string eventName)
	{
		if (eventName == "SOUND_TUTORIAL_TA_SALINE_ADMINISTERED_INSTRUCTOR_H_02")
			PlayNarrativeCharacterStage("Victim", 5);
		else if (eventName == "SOUND_TUTORIAL_TA_SALINE_ADMINISTERED_PARTICIPANT_01")
			PlayNarrativeCharacterStage("FIRSTAID_Instructor", 10);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (GetQuickslotIndexByPrefab("{0D9A5DCF89AE7AA9}Prefabs/Items/Medicine/MorphineInjection_01/MorphineInjection_01.et") >= 0)
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
		
		return consumItem.GetConsumableType() == SCR_EConsumableType.MORPHINE;
	}
}