//! Rally bystander reaction to incoming vehicles. Assign only in the bystander agent prefab.
[BaseContainerProps()]
class SCR_AIDangerReaction_Vehicle_Rally : SCR_AIDangerReaction_Vehicle
{
	ref array<vector> m_RaceSpline = {};
	static const float DISTANCE_TOLERANCE = 9;

	//--------------------------------------------------------------------------------------------------------------------------------------------
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount)
	{
		IEntity vehicleEntity = dangerEvent.GetVictim();
		if (!vehicleEntity)
			return false;

		if (SCR_AIMoveFromDangerBehavior.ExistsBehaviorForEntity(utility, vehicleEntity))
			return false;

		bool weHaveRaceTrack = !m_RaceSpline.IsEmpty();

		if (!weHaveRaceTrack)
			// We initialize just once, the track does not change during the race
			weHaveRaceTrack = InitializeRaceSpline();

		// Is vehicle on race track? If so, ignore danger event
		if (weHaveRaceTrack)
		{
			float distanceFromTrack = SCR_Math3D.GetDistanceFromSplineXZ(m_RaceSpline, vehicleEntity.GetOrigin());
			if (distanceFromTrack < DISTANCE_TOLERANCE)
				return false;
			Print("RallyDLC: Vehicle out of track " + distanceFromTrack.ToString(), LogLevel.DEBUG);
		}

		// Ignore if the behavior would fail immediately after start
		if (!SCR_AIMoveFromIncomingVehicleBehavior.ExecuteBehaviorCondition(utility, vehicleEntity))
			return false;

		SCR_ChimeraAIBystander agent = SCR_ChimeraAIBystander.Cast(utility.GetOwner());
		if (agent)
			agent.InvokeRelevantInvokerToEndGesture(true, true);

		SCR_AIMoveFromDangerBehavior behavior = new SCR_AIMoveFromIncomingVehicleBehavior(utility, null, vector.Zero, dangerEntity: vehicleEntity);
		utility.AddAction(behavior);

		return true;
	}

	//--------------------------------------------------------------------------------------------------------------------------------------------
	protected bool InitializeRaceSpline()
	{
		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return false;

		SCR_RaceCustomizationPointEntity customizationPointEntity = settings.GetCustomizationEntity();
		if (!customizationPointEntity)
		{
			// this is not a race map, valid early exit
			return false;
		}

		SCR_RaceEntity raceEntity = SCR_RaceEntity.Cast(GetGame().GetWorld().FindEntityByName(customizationPointEntity.GetAssignedRace()));
		if (!raceEntity)
		{
			Print("Cannot find Race Entity!", LogLevel.ERROR);
			return false;
		}

		m_RaceSpline = raceEntity.GetRaceSpline();

		return !m_RaceSpline.IsEmpty();
	}
};
