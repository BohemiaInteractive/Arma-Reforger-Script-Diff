[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_VEHICLEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_VEHICLE : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_TutorialComponent.SetSavedTime(GetGame().GetWorld().GetWorldTime());

		IEntity vehicle = GetGame().GetWorld().FindEntityByName("SmallJeep2");
		if (!vehicle)
			return;
		
		m_TutorialComponent.ChangeVehicleLockState(vehicle, false);
		RegisterWaypoint(vehicle, "", "GETIN");

		GetGame().GetCallqueue().CallLater(DelayedSpeech, 1000, true);
		
		if (!SCR_TutorialLogic_Navigation.Cast(m_Logic).m_bMarkerCreated)
			CreateMarkerCustom(GetGame().GetWorld().FindEntityByName("WP_ENDING"), SCR_EScenarioFrameworkMarkerCustom.OBJECTIVE_MARKER, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE, "032-028");
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedSpeech()
	{
		if (vector.Distance(m_Player.GetOrigin(), GetGame().GetWorld().FindEntityByName("SpawnPos_NAVIGATION_Instructor").GetOrigin()) < 10)
			return;
		
		PlayNarrativeCharacterStage("NAVIGATION_Instructor", 17);
		GetGame().GetCallqueue().Remove(DelayedSpeech);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return CompartmentAccessComponent.GetVehicleIn(m_Player) == GetGame().GetWorld().FindEntityByName("SmallJeep2");
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_Navigation_VEHICLE()
	{
		GetGame().GetCallqueue().Remove(DelayedSpeech);
	}
};