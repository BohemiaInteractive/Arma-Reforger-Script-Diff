[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMortars_GetOutMortarClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMortars_GetOutMortar : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("FIRESUPPORT_Instructor", 5);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		TurretControllerComponent turret = TurretControllerComponent.Cast(GetGame().GetWorld().FindEntityByName("Mortar").FindComponent(TurretControllerComponent));
		
		if (!turret)
			return false;
		
		BaseWeaponManagerComponent weaponManager = turret.GetWeaponManager();
		
		if (!weaponManager)
			return false;
		
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		
		if (!player)
			return false;
		
		vector transform[4];
		weaponManager.GetCurrentMuzzleTransform(transform);
		
		float azimuth = transform[2].VectorToAngles()[0];
		float elevationAngle = transform[2].VectorToAngles()[1];

		return azimuth >= 181.85 && azimuth < 183.1 && elevationAngle >= 82.5448 && elevationAngle <= 83.3534 && !player.IsInVehicle();
	}
};