[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_PickupTourniquetClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_PickupTourniquet: SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_TutorialComponent.SpawnAsset("Tourniquet", "{D70216B1B2889129}Prefabs/Items/Medicine/Tourniquet_01/Tourniquet_US_01.et");
		
		RegisterWaypoint("Tourniquet", "", "PICKUP");
		
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
		
		if (consumItem.GetConsumableType() == SCR_EConsumableType.TOURNIQUET)
			UnregisterWaypoint("Tourniquet");
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (GetQuickslotIndexByPrefab("{D70216B1B2889129}Prefabs/Items/Medicine/Tourniquet_01/Tourniquet_US_01.et") >= 0)
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
		
		return consumItem.GetConsumableType() == SCR_EConsumableType.TOURNIQUET;
	}
}