[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_PICKUP_RPG_ROUNDSClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_PICKUP_RPG_ROUNDS : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		CheckRoundCount();
		if (m_bFinished)
			return;

		SCR_InventoryStorageManagerComponent inventory = m_TutorialComponent.GetPlayerInventory();
		if (!inventory)
			return;
		
		inventory.m_OnItemAddedInvoker.Insert(OnItemAdded);
		
		RegisterWaypoint("SW_ARSENAL_USSR", "", "AMMOROCKET");
		m_TutorialComponent.EnableArsenal("SW_ARSENAL_USSR", true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnItemAdded(IEntity item, BaseInventoryStorageComponent storageComponent)
	{
		CheckRoundCount();
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckRoundCount()
	{
		SCR_InventoryStorageManagerComponent inventory = m_TutorialComponent.GetPlayerInventory();
		if (!inventory)
			return;
		
		SCR_ResourceNamePredicate predicate("{32E12D322E107F1C}Prefabs/Weapons/Ammo/Ammo_Rocket_PG7VM.et");
		int count = inventory.CountItem(predicate, EStoragePurpose.PURPOSE_ANY);
		if (count >= 2)
			m_bFinished = true;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_SW_PICKUP_RPG_ROUNDS()
	{
		if (!m_TutorialComponent)
			return;
		
		SCR_InventoryStorageManagerComponent inventory = m_TutorialComponent.GetPlayerInventory();
		if (inventory)
			inventory.m_OnItemAddedInvoker.Remove(OnItemAdded);
	}
}