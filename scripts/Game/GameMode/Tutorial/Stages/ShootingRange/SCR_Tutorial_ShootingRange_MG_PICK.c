[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_MG_PICKClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_MG_PICK : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		IEntity weapon = m_TutorialComponent.SpawnAsset("M249", "{D2B48DEBEF38D7D7}Prefabs/Weapons/MachineGuns/M249/MG_M249.et", null, true);
		if (!weapon)
			return;
		
		m_TutorialComponent.InsertIntoGarbage(weapon);
		
		RegisterWaypoint(weapon, "", "PICKUP");

		PlayNarrativeCharacterStage("SHOOTINGRANGE_Instructor", 10);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(m_Player.FindComponent(BaseWeaponManagerComponent));
		
		if (weaponManager)
			if (weaponManager.GetCurrent())
				return (weaponManager.GetCurrent().GetWeaponType() == EWeaponType.WT_MACHINEGUN);
			else
				return false;
		else
			return true;
	}
};