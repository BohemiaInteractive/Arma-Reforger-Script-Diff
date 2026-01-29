[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_CANONGROUNDClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_CANONGROUND : SCR_BaseTutorialStage
{
	protected const int SERVICE_DISTANCE = 10 * 10;
	//------------------------------------------------------------------------------------------------
	override void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("VEHICLEMAINTENANCE_Instructor", 11);
		SCR_Waypoint wp = RegisterWaypoint("FuelService");
		if (wp)
			wp.SetOffsetVector("0 1 0");
	}

	//------------------------------------------------------------------------------------------------
	protected bool CheckDistance()
	{
		IEntity service = GetGame().GetWorld().FindEntityByName("FuelService");
		IEntity jerryCan = GetGame().GetWorld().FindEntityByName("Repair_Jerrycan");
		if (!service || !jerryCan)
			return false;
		
		return vector.DistanceSq(jerryCan.GetOrigin(), service.GetOrigin()) <= SERVICE_DISTANCE;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (m_TutorialComponent.IsEntityInPlayerInventory("Repair_Jerrycan"))
			return false;
		
		return CheckDistance();
	}
};