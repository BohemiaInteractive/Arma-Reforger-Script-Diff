[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMortars_SettingRangeClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMortars_SettingRange : SCR_BaseTutorialStage
{
	protected bool m_bHelp;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		PlayNarrativeCharacterStage("FIRESUPPORT_Instructor", 4);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (!m_bHelp && m_fStageTimer > 90)
		{
			m_bHelp = true;
			SCR_HintManagerComponent.ShowHint(m_StageInfo.GetHint(1));
		}
		
		TurretControllerComponent turret = TurretControllerComponent.Cast(GetGame().GetWorld().FindEntityByName("Mortar").FindComponent(TurretControllerComponent));
		
		if (!turret)
			return false;
		
		BaseWeaponManagerComponent weaponManager = turret.GetWeaponManager();
		
		if (!weaponManager)
			return false;
		
		vector transform[4];
		weaponManager.GetCurrentMuzzleTransform(transform);
		
		float elevationAngle = transform[2].VectorToAngles()[1];

		return elevationAngle >= 82.5448 && elevationAngle <= 83.3534;
	}
};