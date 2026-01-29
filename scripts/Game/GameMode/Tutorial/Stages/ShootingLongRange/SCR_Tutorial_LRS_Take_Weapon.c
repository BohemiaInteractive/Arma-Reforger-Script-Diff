[EntityEditorProps(insertable: false)]
class SCR_Tutorial_LRS_Take_WeaponClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_LRS_Take_Weapon : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		PlayNarrativeCharacterStage("LONGRANGESHOOTING_Instructor", 2);
		
		CreateMarkerCustom("LONGRANGESHOOTING_Instructor", SCR_EScenarioFrameworkMarkerCustom.POINT_OF_INTEREST, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnVoiceoverEventFinished(string eventName)
	{
		if (eventName != "SOUND_TUTORIAL_TA_COURSE_START_INSTRUCTOR_K_03")
			return;
		
		IEntity gun = GetGame().GetWorld().FindEntityByName("Course_M21");
				
		if (gun)
			RegisterWaypoint(gun, "", "PICKUP");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(m_Player.FindComponent(BaseWeaponManagerComponent));
		
		if (weaponManager)
			if (weaponManager.GetCurrent())
				return (weaponManager.GetCurrent().GetWeaponType() == EWeaponType.WT_SNIPERRIFLE);
			else
				return false;
		else
			return true;
	}
};