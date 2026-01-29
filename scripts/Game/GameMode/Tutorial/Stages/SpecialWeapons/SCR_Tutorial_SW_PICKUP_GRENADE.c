[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_PICKUP_GRENADEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_PICKUP_GRENADE : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		//TODO: Check for equipped!
		if (m_TutorialComponent.FindPrefabInPlayerInventory("{E8F00BF730225B00}Prefabs/Weapons/Grenades/Grenade_M67.et"))
		{
			m_bFinished = true;
			return;
		}
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManComp)	
			storageManComp.m_OnItemAddedInvoker.Insert(OnItemAdded);
		
		m_TutorialComponent.EnableArsenal("SW_ARSENAL_GRENADES", true);
		RegisterWaypoint("SW_ARSENAL_GRENADES", "", "AMMO");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnItemAdded(IEntity item, BaseInventoryStorageComponent storageComponent)
	{
		WeaponComponent wepComp = WeaponComponent.Cast(item.FindComponent(WeaponComponent));
		
		if (!wepComp)
			return false;
		 
		if (wepComp.GetWeaponType() == EWeaponType.WT_FRAGGRENADE)
			m_bFinished = true;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_SW_PICKUP_GRENADE()
	{
		if (!m_Player)
			return;
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManComp)	
			storageManComp.m_OnItemAddedInvoker.Remove(OnItemAdded);
	}
	
};