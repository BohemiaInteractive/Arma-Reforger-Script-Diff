[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Driving_DRIVEROADClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Driving_DRIVEROAD : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_1");
		CreateMarkerPath("DRIVING_PATH_DOTS");
		PlayNarrativeCharacterStage("DRIVING_Copilot", 4);

		if (BaseVehicleControllerComponent.GetDrivingAssistanceMode() != EVehicleDrivingAssistanceMode.FULL)
			ShowHint(1);
		else
			ShowHint();
		
		ChimeraWorld world = GetGame().GetWorld();
			
		if (world)
			world.GetMusicManager().Play("SOUND_DRIVING_COURSE");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
};