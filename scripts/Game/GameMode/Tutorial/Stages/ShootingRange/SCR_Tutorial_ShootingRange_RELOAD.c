[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_RELOADClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_RELOAD : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		PlayNarrativeCharacterStage("SHOOTINGRANGE_Instructor", 4);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(m_Player.FindComponent(BaseWeaponManagerComponent));
		
		if (!weaponManager)
			return false;

		BaseWeaponComponent weaponComponent = weaponManager.GetCurrent();
		
		if (!weaponComponent)
			return false;
		
		BaseMagazineComponent mag = weaponComponent.GetCurrentMagazine();
		
		if (mag)
			return (mag.GetAmmoCount() != 0);
		else
			return false;
	}
	
};