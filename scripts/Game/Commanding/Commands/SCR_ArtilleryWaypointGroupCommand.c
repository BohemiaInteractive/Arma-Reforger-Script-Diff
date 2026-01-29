[BaseContainerProps(), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_ArtilleryWaypointGroupCommand : SCR_WaypointGroupCommand
{
	[Attribute(desc: "Distance below which random offset will not be applied. [m]", params: "0 inf")]
	protected float m_fPositionOffsetCloseDistance;

	[Attribute(desc: "Distance from which target position offsetting will use max values. [m]\nIf this value is 0 then offset will not be applied", params: "0 inf")]
	protected float m_fPositionOffsetFarDistance;

	[Attribute(desc: "Max possible offset that can be added to the target position. [m]", params: "0 inf")]
	protected float m_fMaxPositionOffset;

	[Attribute(desc: "Curve that describes bottom end of the random distance.\nWhen values are reed, the X axis is determined by (DistanceFromCharacterToTargetPosition - PositionOffsetCloseDistance) / PositionOffsetFarDistance,\n while Y axis will be used to determine min offset distance by multipling it by MaxPositionOffset", uiwidget: UIWidgets.GraphDialog, params: "1 1 0 0")]
	protected ref Curve m_aMinPositionOffset;
	
	[Attribute(desc: "Curve that describes top end of the random distance.\nWhen values are reed, the X axis is determined by (DistanceFromCharacterToTargetPosition - PositionOffsetCloseDistance) / PositionOffsetFarDistance,\n while Y axis will be used to determine max offset distance by multipling it by MaxPositionOffset", uiwidget: UIWidgets.GraphDialog, params: "1 1 0 0")]
	protected ref Curve m_aMaxPositionOffset;

#ifdef ENABLE_DIAG
	protected static ref SCR_DebugShapeManager s_DebugShapes = new SCR_DebugShapeManager();
#endif

	//------------------------------------------------------------------------------------------------
	override bool SetWaypointForAIGroup(IEntity target, vector targetPosition, int playerID)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID));
		if (!character)
			return false;

		SpoofTargetPosition(targetPosition, character);

		return super.SetWaypointForAIGroup(target, targetPosition, playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Attempts to offset the target position by random value based on the configuration of this command
	//! \param[inout] targetPosition position that is going to be moddified
	//! \param[in] character issued this command
	protected void SpoofTargetPosition(inout vector targetPosition, notnull ChimeraCharacter character)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_COMMANDING_WAYPOINT_POSITIONS))
		{
			s_DebugShapes.Clear();
			s_DebugShapes.AddSphere(targetPosition, 1, Color.GREEN, ShapeFlags.WIREFRAME);
		}
#endif
		if (m_fPositionOffsetFarDistance == 0)
			return;//no spoofing configured for this command

		SCR_AdditionalGameModeSettingsComponent gameModeSettings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!gameModeSettings || !gameModeSettings.IsAdditionalArtileryOrderDistancePenaltyEnabled())
			return;//spoofing disabled in gamemode settings

		vector characterPosition = character.GetOrigin();
		float distance = vector.Distance(targetPosition, characterPosition);
		if (distance < m_fPositionOffsetCloseDistance)
			return;//too close to spoof

		float distanceNormal = (distance - m_fPositionOffsetCloseDistance) / m_fPositionOffsetFarDistance;
		float minOffsetValue = LegacyCurve.Curve(ECurveType.CurveProperty2D, distanceNormal, m_aMinPositionOffset)[1] * m_fMaxPositionOffset;
		float maxOffsetValue = LegacyCurve.Curve(ECurveType.CurveProperty2D, distanceNormal, m_aMaxPositionOffset)[1] * m_fMaxPositionOffset;

		if (maxOffsetValue < minOffsetValue)
		{
			Print("Warning! Minimal order offset value is lower than maximal! Check configuration of curves for " + m_sCommandName + "at point " + distanceNormal, LogLevel.WARNING);
			return;
		}

		targetPosition = SCR_Math2D.GenerateRandomPointInRadius(minOffsetValue, maxOffsetValue, targetPosition, false);
		targetPosition[1] = SCR_TerrainHelper.GetTerrainY(targetPosition, noUnderwater: true);

#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_COMMANDING_WAYPOINT_POSITIONS))
		{
			s_DebugShapes.AddSphere(targetPosition, 0.5 + distanceNormal * 5, Color.RED, ShapeFlags.WIREFRAME);
			Print("Debug: Order position is " + distance + " meters away from the player.");
			Print("Debug: Using curve values for X = " + Math.Clamp(distanceNormal, 0, 1));
			Print("Debug: Min possible offset for this distance is " + minOffsetValue);
			Print("Debug: Max possible offset for this distance is " + maxOffsetValue);
		}
#endif
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_ArtilleryWaypointGroupCommand()
	{
		if (m_fPositionOffsetFarDistance < m_fPositionOffsetCloseDistance)
		{
			Print("ERROR: Command " + m_sCommandName + " is misconfigured in Commands.conf! Its 'Position Offset Close Distance' (" + m_fPositionOffsetCloseDistance + ") is larger than 'Position Offset Far Distance' (" + m_fPositionOffsetFarDistance + ")", LogLevel.ERROR);
			m_fPositionOffsetCloseDistance = 0;
			m_fPositionOffsetFarDistance = 0;
		}

#ifdef ENABLE_DIAG
		if (Replication.IsServer()) //position is only spoofed on the server as it is spawning the waypoint
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_COMMANDING_WAYPOINT_POSITIONS, "", "Artillery command debug", "AI");
#endif
	}
}
