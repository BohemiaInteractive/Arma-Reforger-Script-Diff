[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Driving_ENDClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Driving_END : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		PlayNarrativeCharacterStage("DRIVING_Copilot", 14);
		SCR_HintManagerComponent.HideHint();
		
		IEntity car = GetGame().GetWorld().FindEntityByName("SmallJeep");
			
		if (car)
		{
			SCR_VehicleSpawnProtectionComponent comp = SCR_VehicleSpawnProtectionComponent.Cast(car.FindComponent(SCR_VehicleSpawnProtectionComponent));
			
			if (comp)
				comp.SetReasonText("#AR-Campaign_Action_CannotEnterVehicle-UC");

			m_TutorialComponent.ChangeVehicleLockState(car, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_EVE_PLAYER_GETS_OUT_JEEP_INSTRUCTOR_F_01" || GetDuration() > 5000;
	}
};