//! A system controlling AI groups marking enemy targets on the map and managing those markers
class SCR_AIEnemyMarkingSystem : GameSystem
{
	[Attribute(defvalue: "330", desc: "Time in seconds until the markers put by AI groups automatically disappear.\n0 means that markers will get deleted immediately.", params: "0 inf 0.01")]
	protected float m_fTimeUntilMarkerDeletion;

	[Attribute(defvalue: "30", desc: "Time that needs to pass before a group can update a markings position.\n0 means that the markers will get updated as soon as the AI group notices", params: "0 inf 0.01")]
	protected float m_fGroupMarkingCooldown;

	[Attribute(defvalue: "150", desc: "Minimum distance 2 markings need to have to not be lumped together for infantry.\n0 means that if the position is not *exactly* the same it will make a seperate marker", params: "0 inf 0.01")]
	protected float m_fMinimumMarkDistanceInfantry;

	[Attribute(defvalue: "250", desc: "Minimum distance 2 markings need to have to not be lumped together for ground vehicles.\n0 means that if the position is not *exactly* the same it will make a seperate marker", params: "0 inf 0.01")]
	protected float m_fMinimumMarkDistanceVehicles;

	[Attribute(defvalue: "0.2", desc: "Inaccuracy by distance, adds a certain % of the distance to the random inaccuracy.\n0 means that AI spotting will be just as accurate 10 meters away as 1 km away.", params: "0 1 0.01")]
	protected float m_fInaccuracyByDistance;

	[Attribute(defvalue: "15", desc: "Base innaccuracy, flat base addition in meters.\n0 means that (ignoring the inaccuracy by distance) it will always put the marker on the exact spot it spotted the enemy", params: "0 inf 0.01")]
	protected float m_fBaseInaccuracy;

	[Attribute(defvalue: "1", desc: "System update frequency in seconds to check which markers need to be deleted, with this variable being here for performance.\n0 means that it will be updated every frame.", params: "0 inf 0.01")]
	protected float m_fSystemUpdateFrequency;

	protected float m_fUpdateTimeTracker;
	protected const string FORMAT_MILITARY_TIME = "%1:%2";

	protected ref map<ref SCR_MapMarkerBase, EMilitarySymbolIdentity> m_mPlacedMarkersFactions = new map<ref SCR_MapMarkerBase, EMilitarySymbolIdentity>();

	//------------------------------------------------------------------------------------------------
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		outInfo.SetAbstract(false);
		outInfo.SetLocation(WorldSystemLocation.Server);
		outInfo.AddPoint(WorldSystemPoint.FixedFrame);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnUpdatePoint(WorldUpdatePointArgs args)
	{
		m_fUpdateTimeTracker += args.GetTimeSliceSeconds();
		if (m_fUpdateTimeTracker < 0)
			return;

		m_fUpdateTimeTracker -= m_fSystemUpdateFrequency;

		WorldTimestamp currentTime = ChimeraWorld.CastFrom(GetWorld()).GetServerTimestamp();
		SCR_MapMarkerManagerComponent m_MarkerMgr = SCR_MapMarkerManagerComponent.GetInstance();

		SCR_MapMarkerBase marker;
		WorldTimestamp previousTime;
		for (int idx = m_mPlacedMarkersFactions.Count() - 1; idx >= 0; idx--)
		{
			marker = m_mPlacedMarkersFactions.GetKey(idx);
			previousTime = marker.GetTimestamp();
			if (previousTime.PlusSeconds(m_fTimeUntilMarkerDeletion).Less(currentTime))
			{
				m_MarkerMgr.RemoveStaticMarker(marker);
				m_mPlacedMarkersFactions.Remove(marker);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Validates and attempts to mark a new target on the map
	//! \param[in] targetInfo Spotted and identified target, can be both a vehicle or character.
	//! \param[in] spottingGroup AIGroup that identified the target
	void MarkTarget(notnull SCR_AITargetInfo targetInfo, notnull SCR_AIGroup spottingGroup)
	{
		ChimeraWorld world = GetWorld();
		SCR_Faction spottingGroupFaction = SCR_Faction.Cast(spottingGroup.GetFaction());
		if (!spottingGroupFaction || !spottingGroupFaction.CanAIMarkTargets())
			return;

		WorldTimestamp currentTime = world.GetServerTimestamp();
		WorldTimestamp markerUpdateCutoffTime = currentTime.PlusSeconds(-m_fGroupMarkingCooldown);

		EMilitarySymbolIdentity identity = GetIdentitySymbol(targetInfo.m_Faction);
		EMilitarySymbolIcon Icon = GetMilitarySymbolFromTarget(targetInfo);

		array<SCR_MapMarkerBase> overriddenMarkers = {};

		WorldTimestamp previousTime;
		float markerSqDistance;
		float minimumSqDistance = Math.Pow(GetMinimumMarkDistanceFromType(Icon), 2);
		foreach (SCR_MapMarkerBase thisMarker, EMilitarySymbolIdentity thisFactionIdentitySymbol : m_mPlacedMarkersFactions)
		{
			if (Icon != thisMarker.GetFlags() // no need to compare distance if its a different "type" of marker (faction spotted, faction set or infantry / motorized)
			|| identity != thisFactionIdentitySymbol
			|| thisMarker.GetMarkerFactionFlags() != Math.Pow(2, spottingGroup.GetFactionIndex()))
				continue;

			markerSqDistance = GetMarkerDistanceSqFromPoint(targetInfo.m_vWorldPos, thisMarker);

			if (markerSqDistance < minimumSqDistance)
			{
				previousTime = thisMarker.GetTimestamp();

				if (previousTime.Greater(markerUpdateCutoffTime))
					return; // There already is a recent marker placed here and it is too soon to update it

				overriddenMarkers.Insert(thisMarker);
			}
		}

		SCR_MapMarkerManagerComponent m_MarkerMgr = SCR_MapMarkerManagerComponent.GetInstance();

		SCR_MapMarkerBase marker = m_MarkerMgr.PrepareMilitaryMarker(identity, EMilitarySymbolDimension.LAND, Icon);

		float distance = vector.Distance(targetInfo.m_vWorldPos, spottingGroup.GetLeaderEntity().GetOrigin());
		vector randomness = SCR_Math.GetMathRandomGenerator().GenerateRandomPointInRadius(0, m_fBaseInaccuracy + distance * m_fInaccuracyByDistance, vector.Zero, false);

		marker.SetWorldPos(targetInfo.m_vWorldPos[0] + randomness[0], targetInfo.m_vWorldPos[2] + randomness[1]);
		marker.SetMarkerFactionFlags(Math.Pow(2, spottingGroup.GetFactionIndex()));
		marker.SetTimestamp(currentTime);
		marker.SetTimestampVisibility(true);
		marker.SetDragged(false);

		TimeContainer ingameTime = world.GetTimeAndWeatherManager().GetTime();
		marker.SetCustomText(string.Format(FORMAT_MILITARY_TIME, ingameTime.m_iHours.ToString(2), ingameTime.m_iSeconds.ToString(2)));

		m_MarkerMgr.InsertStaticMarker(marker, false, true);
		m_mPlacedMarkersFactions.Insert(marker, identity);

		for (int idx = overriddenMarkers.Count() - 1; idx >= 0; idx--)
		{
			SCR_MapMarkerBase overriddenMarker = overriddenMarkers[idx];
			m_MarkerMgr.RemoveStaticMarker(overriddenMarker);
			m_mPlacedMarkersFactions.Remove(overriddenMarker);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected float GetMinimumMarkDistanceFromType(EMilitarySymbolIcon markType)
	{
		if (markType == EMilitarySymbolIcon.MOTORIZED)
			return m_fMinimumMarkDistanceVehicles;

		return m_fMinimumMarkDistanceInfantry;
	}

	//------------------------------------------------------------------------------------------------
	protected EMilitarySymbolIcon GetMilitarySymbolFromTarget(notnull SCR_AITargetInfo targetInfo)
	{
		if (targetInfo.m_Entity.Type().IsInherited(Vehicle))
			return EMilitarySymbolIcon.MOTORIZED;

		return EMilitarySymbolIcon.INFANTRY;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	protected EMilitarySymbolIdentity GetIdentitySymbol(notnull Faction targetFaction)
	{
		switch (targetFaction.GetFactionKey())
		{
			case "US":
				return EMilitarySymbolIdentity.BLUFOR;

			case "USSR":
				return EMilitarySymbolIdentity.OPFOR;
		}

		return EMilitarySymbolIdentity.INDFOR;
	}

	//------------------------------------------------------------------------------------------------
	// Helper method that calculates a points distance from a marker
	//! \param[in] point
	//! \param[in] marker
	protected float GetMarkerDistanceSqFromPoint(vector point, notnull SCR_MapMarkerBase marker)
	{
		int worldPos[2];
		marker.GetWorldPos(worldPos);
		vector markerPointComparison = {worldPos[0], 0, worldPos[1]};
		return point.DistanceSqXZ(point, markerPointComparison);
	}
}
