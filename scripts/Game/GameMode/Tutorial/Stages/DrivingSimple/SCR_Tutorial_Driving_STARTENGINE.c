[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Driving_STARTENGINEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Driving_STARTENGINE : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		if (BaseVehicleControllerComponent.GetDrivingAssistanceMode() == EVehicleDrivingAssistanceMode.NONE)
			ShowHint(1);
		else
			ShowHint();
		
		PlayNarrativeCharacterStage("DRIVING_Copilot", 3);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
      	Vehicle jeep = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("SmallJeep"));
		if (!jeep)
			return false;
		
		IEntity waypoint = GetGame().GetWorld().FindEntityByName("WP_DRIVING_1");
		if (!waypoint)
			return false;
		
		VehicleControllerComponent controller = VehicleControllerComponent.Cast(jeep.FindComponent(VehicleControllerComponent));
		if (controller)
		{
			return controller.IsEngineOn() && (m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_IN_JEEP_INSTRUCTOR_F_01" || GetDuration() > 8000 || vector.Distance(jeep.GetOrigin(), waypoint.GetOrigin()) < 10);
		}
		
		return false;
	}
};