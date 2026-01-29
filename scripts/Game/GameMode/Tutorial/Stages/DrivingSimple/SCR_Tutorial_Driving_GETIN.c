[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Driving_GETINClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Driving_GETIN : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		if (!m_Player.IsInVehicle())
		{
			IEntity vehicle = GetGame().GetWorld().FindEntityByName("SmallJeep");
			
			if (!vehicle)
				return;
			
			RegisterWaypoint(vehicle, "", "GETIN");
			m_TutorialComponent.ChangeVehicleLockState(GetGame().GetWorld().FindEntityByName("SmallJeep"), false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (m_Player.IsInVehicle())
			return true;
		else
			return m_Player.IsInVehicle() && (m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_DRIVING_COURSE_IN_INSTRUCTOR_F_02" || GetDuration() > 6000);
	}
};