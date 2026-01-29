[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_PickupBandageClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_PickupBandage: SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_TutorialComponent.SpawnAsset("Bandage", "{A81F501D3EF6F38E}Prefabs/Items/Medicine/FieldDressing_01/FieldDressing_US_01.et");
		m_TutorialComponent.SpawnAsset("Bandage2", "{A81F501D3EF6F38E}Prefabs/Items/Medicine/FieldDressing_01/FieldDressing_US_01.et");
		
		RegisterWaypoint("Bandage", "", "PICKUP");
		PlayNarrativeCharacterStage("FIRSTAID_Instructor", 5);
		
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
		
		if (consumItem.GetConsumableType() == SCR_EConsumableType.BANDAGE)
			UnregisterWaypoint("Bandage");
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (GetQuickslotIndexByPrefab("{A81F501D3EF6F38E}Prefabs/Items/Medicine/FieldDressing_01/FieldDressing_US_01.et") >= 0 && m_TutorialComponent.IsEntityInPlayerInventory("Bandage") && m_TutorialComponent.IsEntityInPlayerInventory("Bandage2"))
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
		
		return consumItem.GetConsumableType() == SCR_EConsumableType.BANDAGE && m_TutorialComponent.IsEntityInPlayerInventory("Bandage") && m_TutorialComponent.IsEntityInPlayerInventory("Bandage2");
	}
}