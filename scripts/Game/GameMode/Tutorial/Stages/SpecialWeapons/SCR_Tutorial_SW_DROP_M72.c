[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_DROP_M72Class: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_DROP_M72 : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override void Setup()
	{
		if (m_StageInfo.GetStageName() != "M72_DROP")
			return;
		
		SCR_TutorialLogic_SW.Cast(m_Logic).m_iMisses_M72++;
		
		int misses = SCR_TutorialLogic_SW.Cast(m_Logic).m_iMisses_M72;
		
		if (misses > 2)
			misses = 1;
		
		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 12 + misses);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsEmptyLauncherDropped()
	{
		SCR_InventoryStorageManagerComponent inventory = m_TutorialComponent.GetPlayerInventory();
		if (!inventory)
			return false;
		
		array<IEntity> foundItems = {};		
		inventory.FindItemsWithComponents(foundItems, {BaseWeaponComponent}, EStoragePurpose.PURPOSE_ANY);
		foreach (IEntity item : foundItems)
		{
			const BaseWeaponComponent weapon = BaseWeaponComponent.Cast(item.FindComponent(BaseWeaponComponent));
			if (weapon.GetWeaponType() != EWeaponType.WT_ROCKETLAUNCHER)
				continue;
			
			BaseMuzzleComponent muzzle = weapon.GetCurrentMuzzle();
			if (!muzzle)
				continue;
			
			if (muzzle.GetAmmoCount() == 0)
				return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return IsEmptyLauncherDropped();
	}
}