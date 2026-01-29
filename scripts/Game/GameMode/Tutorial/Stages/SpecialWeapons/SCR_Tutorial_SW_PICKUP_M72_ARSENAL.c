[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_PICKUP_M72_ARSENALClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_PICKUP_M72_ARSENAL : SCR_BaseTutorialStage
{
	SCR_CharacterInventoryStorageComponent m_StorageComponent;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		if (HasLauncher())
		{
			m_bFinished = true;
			return;
		}

		RegisterWaypoint("SW_ARSENAL_M72", "", "PICKUP");
		
		if (m_TutorialComponent)
			m_TutorialComponent.EnableArsenal("SW_ARSENAL_M72", true);
		
		m_StorageComponent = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return HasLauncher();
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool HasLauncher()
	{
		SCR_InventoryStorageManagerComponent inventory = m_TutorialComponent.GetPlayerInventory();
		if (!inventory)
			return false;
		
		SCR_ResourceNamePredicate predicate("{9C5C20FB0E01E64F}Prefabs/Weapons/Launchers/M72/Launcher_M72A3.et");
		array<IEntity> foundItems = {};
		inventory.FindItems(foundItems, predicate, EStoragePurpose.PURPOSE_ANY);
		foreach (IEntity item : foundItems)
		{
			const BaseWeaponComponent weapon = BaseWeaponComponent.Cast(item.FindComponent(BaseWeaponComponent));
			if (!weapon)
				continue;
			
			const BaseMuzzleComponent muzzle = weapon.GetCurrentMuzzle();
			if (!muzzle)
				continue;
			
			if (muzzle.GetAmmoCount() > 0)
				return true;
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_SW_PICKUP_M72_ARSENAL()
	{
		if (m_TutorialComponent)
			m_TutorialComponent.EnableArsenal("SW_ARSENAL_M72", false);
	}
}