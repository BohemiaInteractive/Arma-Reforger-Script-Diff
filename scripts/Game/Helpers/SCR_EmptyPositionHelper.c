//#define SCR_EMPTY_POSITION_HELPER_DEBUG_SHAPES  // Shows Successful and Unsuccessful positions and scans.

class SCR_EmptyPositionHelper
{
	static const float MAX_SCAN_LENGTH = 50;
	static const int SPHERE_AMOUNT_LIMIT = 3; // About 109 Checks assuming half get cancelled for being underground or underwater.
	static const float TRACE_EPSILON = 0.01; // Removes the issue where traces fail due to being too close to object or ground.
	static const float CHARACTER_BOUNDING_RADIUS_MODIFIER = 0.75; // Characters are more flexible, reduce their bounding radius slightly.
	static const float CHARACTER_SPACING_MODIFIER = 0.5; // Characters are more flexible, reduce their spacing slightly.
	protected static const string DEBUG_TAG = "SCR_EmptyPositionHelper"; // Groups debug objects together.

	//------------------------------------------------------------------------------------------------
	//! Finds nearby floor position with enough space for the entity, considering its amphibious trait if applicable.
	//! Does not move or modify the entity.
	//! \param[in] world Game world.
	//! \param[in] entity Entity is used to define Trace params, amphibious trait, and position*.
	//! \param[in] position Optional position to use for searching from instead of entity.GetOrigin().
	//! \param[out] floorPos If successful it is the floor with enough space for the entity can stand.
	//! \return True if a floor with enough space was found.
	static bool TryFindNearbyFloorPositionForEntity(BaseWorld world, IEntity entity, vector position = vector.Zero, out vector floorPos = vector.Zero)
	{
		if (position == vector.Zero)
			position = entity.GetOrigin();

		vector mins, maxs;
		entity.GetBounds(mins, maxs);

		// TraceBox is the correct trace to use. Currently however, the engine sometimes lets TraceBoxes travel through terrain.
		// Switch back to TraceBox once that is fixed.
		//TraceBox traceParam = new TraceBox();
		//entity.GetBounds(traceParam.Mins, traceParam.Maxs);
		//traceParam.Mins = mins;
		//traceParam.Maxs = maxs;

		float horizontalDiagonalLength = Math.Sqrt(Pow2(maxs[0] - mins[0]) + Pow2(maxs[2] - mins[2]));
		float cuboidDiagonalLength = Math.Sqrt(Pow2(horizontalDiagonalLength) + Pow2(maxs[1] - mins[1]));

		// Characters are more flexible, reduce their bounding radius slightly.
		if (SCR_ChimeraCharacter.Cast(entity))
		{
			horizontalDiagonalLength *= CHARACTER_BOUNDING_RADIUS_MODIFIER;
			cuboidDiagonalLength *= CHARACTER_SPACING_MODIFIER;
		}

		TraceSphere traceParam = new TraceSphere();
		traceParam.Radius = 0.5 * horizontalDiagonalLength;
		traceParam.Exclude = entity;
		float entityHeight = maxs[1] - mins[1];
		float spacing = 2 * cuboidDiagonalLength;
		float maxHorizontalDistance = 10 * spacing;
		float maxSubmersionDepth = 0.5 * entityHeight;

		SCR_EditableEntityUIInfo editableEntityUIInfo = GetEntityUIInfo(entity);
		bool isAmphibious = editableEntityUIInfo && editableEntityUIInfo.HasEntityLabel(EEditableEntityLabel.TRAIT_AMPHIBIOUS);

		bool success = TryFindNearbyFloorPosition(world, position, traceParam, entityHeight, spacing, maxHorizontalDistance, isAmphibious, maxSubmersionDepth, floorPos);
		if (!success)
			PrintFormat("[SCR_EmptyPositionHelper] TryFindNearbyFloorPositionForEntity failed for find space nearby %1 for %2", position, entity, level: LogLevel.WARNING);

		return success;
	}

	//------------------------------------------------------------------------------------------------
	//! Attempts to find a nearby position with suitable space defined by traceParam.
	//! \param[in] world Game world.
	//! \param[in] position The start position for searching.
	//! \param[in,out] traceParam TraceParam must define a shape with dimensions.
	//! \param[in] minHeight Minimum scan length is the minimum distance that the shape must be traced.
	//! \param[in] spacing Spacing represents the distance between checks, this should be atleast or greater than the size of the entity.
	//! \param[in] maxDistance MaxDistance is the maximum distance from the start position to search for a valid floor positions.
	//! \param[in] isAmphibious Amphibious switch indicates whether the search for floor position should consider ocean surface as a valid floor.
	//! \param[out] floorPos Represents the final found floor position within the specified search area.
	//! \return True if a valid floor was found.
	static bool TryFindNearbyFloorPosition(BaseWorld world, vector position, TraceParam traceParam, float minHeight, float spacing, float maxDistance = 100, bool isAmphibious = false, float maxSubmersionDepth = 0, out vector floorPos = "0 0 0")
	{
		if (!traceParam.Flags)
		{
			traceParam.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			if (isAmphibious)
				traceParam.Flags |= TraceFlags.OCEAN;
		}

		if (traceParam.LayerMask == -1)
			traceParam.LayerMask = EPhysicsLayerDefs.Projectile;

		float traceHeight = GetTraceParamHeight(traceParam);
		if (float.AlmostEqual(traceHeight, 0))
			traceHeight = 0.001;

		position = RaiseToTerrain(world, position, 0.75 * traceHeight + TRACE_EPSILON);
		float minFloorY = GetMinYOnTerrainOrOcean(world, position, maxSubmersionDepth);
		TraceSphere preTraceParam = CreatePreTraceParam(traceParam);

		vector ceilingPos;
		if (TryGetValidFloorPositionWithPreTrace(world, position, minHeight, traceParam, minFloorY, preTraceParam, floorPos, ceilingPos))
			return true;

		float halfSpacing = 0.5 * spacing;
		int maxSpheres = Math.Min(Math.Ceil(maxDistance / spacing), SPHERE_AMOUNT_LIMIT);
		for (int sphereI = 1; sphereI <= maxSpheres; ++sphereI)
		{
			float unitSpacing = 1 / sphereI;
			int pointsAmount = Math.Min(100, SCR_SpherePointGenerator.EstimatePointsFromSpacingOnUnitSphere(unitSpacing));
			float radius = sphereI * spacing;
			for (int pointI = 0; pointI < pointsAmount; ++pointI)
			{
				vector point = position + radius * SCR_SpherePointGenerator.GetPointOnUnitSphereFromEquator(pointsAmount, pointI);
				float terrainHeight = world.GetSurfaceY(point[0], point[2]);
				// Skip if underground.
				if (point[1] < terrainHeight + TRACE_EPSILON)
				{
#ifdef SCR_EMPTY_POSITION_HELPER_DEBUG_SHAPES
					SCR_DebugShapeHelperComponent.AddText(world, position, "Underground", tag: DEBUG_TAG);
#endif
					continue;
				}

				float oceanHeight = world.GetOceanHeight(point[0], point[2]);
				if (isAmphibious)
				{
					point[1] = Math.Max(oceanHeight + TRACE_EPSILON, point[1]);
				}
				else
				{
					if (point[1] < oceanHeight + TRACE_EPSILON)
					{
#ifdef SCR_EMPTY_POSITION_HELPER_DEBUG_SHAPES
						SCR_DebugShapeHelperComponent.AddText(world, position, "Underwater", tag: DEBUG_TAG);
#endif
						continue;
					}

				}
				minFloorY = GetMinYOnTerrainOrOcean(world, point, maxSubmersionDepth);
				if (TryGetValidFloorPositionWithPreTrace(world, point, minHeight, traceParam, minFloorY, preTraceParam, floorPos, ceilingPos))
					return true;
			}
		}
		// Failed to find valid position. Reset floorPos to position.
		floorPos = position;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks if there's a floor with enough empty space above it.
	//! Ocean surface counts as a floor.
	//! Does not scan further than MAX_SCAN_LENGTH
	//! \param[in] world Game world.
	//! \param[in] position Position to start search from.
	//! \param[in] minHeight MinHeight is the minimium length that the shape needs to be trace out to fit the height requirement for empty space above the floor.
	//! \param[in] traceParam TraceParam should specify flags, entities, shape, and size to trace. Start and end position are overwritten on reference.
	//! \param[in] If the scanned floor position is lower than this, the function will return false.
	//! \param[in] Extra ceiling height to scan for if knowing the ceiling is desired.
	//! \param[out] floorPos Represents the position of the floor detected by tracing downwards from the input position.
	//! \param[out] ceilingPos Ceiling position represents the ceiling detected by tracing upwards. Limited to floor + minHeight.
	//! \return True if a floor with enough empty space was found.
	static bool TryGetValidFloorPosition(BaseWorld world, vector position, float minHeight, TraceParam traceParam, float minFloorY, float bonusTraceLength, out vector floorPos, out vector ceilingPos)
	{
		// Future feature goes here when engine function becomes accessible.
		// If (position inside another entity's collision geometry)
		// return false;

		float traceHeight = GetTraceParamHeight(traceParam);
		if (float.AlmostEqual(traceHeight, 0))
			traceHeight = 0.001;
		float halfTraceHeight = 0.5 * traceHeight;
		bonusTraceLength = Math.Min(bonusTraceLength, MAX_SCAN_LENGTH);
		float surfaceHeightWithBuffer = world.GetSurfaceY(position[0], position[2]) + traceHeight + TRACE_EPSILON;

		traceParam.Start = position;
		traceParam.End = position - Vector(0, Math.Max(MAX_SCAN_LENGTH, minHeight - traceHeight), 0);
		bool floorFound = TryDetectSurface(world, traceParam, floorPos);

		// Check if instant collision.
		if (floorPos == traceParam.Start)
		{
			ceilingPos = position;
#ifdef SCR_EMPTY_POSITION_HELPER_DEBUG_SHAPES
			SCR_DebugShapeHelperComponent.CreateFloorCeilingTraceShapes(world, position, traceParam, floorPos, ceilingPos, false, "Floor clipped", tag: DEBUG_TAG);
#endif
			return false;
		}

		// Adjust for height of traced shape.
		floorPos -= Vector(0, halfTraceHeight, 0);

		if (!floorFound)
		{
			ceilingPos = position;
#ifdef SCR_EMPTY_POSITION_HELPER_DEBUG_SHAPES
			SCR_DebugShapeHelperComponent.CreateFloorCeilingTraceShapes(world, position, traceParam, floorPos, ceilingPos, false, "No floor", tag: DEBUG_TAG);
#endif
			return false;
		}

		if (floorPos[1] < minFloorY)
		{
			ceilingPos = position;
#ifdef SCR_EMPTY_POSITION_HELPER_DEBUG_SHAPES
			SCR_DebugShapeHelperComponent.CreateFloorCeilingTraceShapes(world, position, traceParam, floorPos, ceilingPos, false, "Floor too low", tag: DEBUG_TAG);
#endif
			return false;
		}

		float remainingScanLength = minHeight - (position[1] - floorPos[1]) - halfTraceHeight;
		if (remainingScanLength + bonusTraceLength < 0) // intentially < rather than <=, if the trace instantly collides, that means there was no space.
		{
			ceilingPos = position + Vector(0, halfTraceHeight, 0);
#ifdef SCR_EMPTY_POSITION_HELPER_DEBUG_SHAPES
			SCR_DebugShapeHelperComponent.CreateFloorCeilingTraceShapes(world, position, traceParam, floorPos, ceilingPos, true, "Enough height, skipped ceiling", tag: DEBUG_TAG);
#endif
			return true;
		}

		traceParam.Start = position - Vector(0, halfTraceHeight, 0);
		traceParam.End = position + Vector(0, remainingScanLength + bonusTraceLength, 0);
		TryDetectSurface(world, traceParam, ceilingPos);

		// Check if instant collision.
		if (ceilingPos == traceParam.Start)
		{
#ifdef SCR_EMPTY_POSITION_HELPER_DEBUG_SHAPES
			SCR_DebugShapeHelperComponent.CreateFloorCeilingTraceShapes(world, position, traceParam, floorPos, ceilingPos, false, "Ceiling clipped", tag: DEBUG_TAG);
#endif
			return false;
		}

		// Adjust for height of traced shape.
		ceilingPos += Vector(0, halfTraceHeight, 0);

		bool enoughSpace = (ceilingPos[1] - floorPos[1]) >= minHeight;
#ifdef SCR_EMPTY_POSITION_HELPER_DEBUG_SHAPES
		if (enoughSpace)
			SCR_DebugShapeHelperComponent.CreateFloorCeilingTraceShapes(world, position, traceParam, floorPos, ceilingPos, enoughSpace, "Good position", tag: DEBUG_TAG);
		else
			SCR_DebugShapeHelperComponent.CreateFloorCeilingTraceShapes(world, position, traceParam, floorPos, ceilingPos, enoughSpace, "Insufficient height", tag: DEBUG_TAG);
#endif
		return enoughSpace;
	}

	//------------------------------------------------------------------------------------------------
	//! Detects surface position along trace path in world, returns true if surface was found.
	//! \param[in] world Game world.
	//! \param[in] traceParam TraceParam needs to have everythging nessary defined.
	//! \param[out] surfacePos Surface position of center of trace in 3D space where the trace intersects a surface.
	//! \return True if a surface was detected.
	static bool TryDetectSurface(BaseWorld world, TraceParam traceParam, out vector surfacePos)
	{
		float pathTravelled = world.TraceMove(traceParam, null);
		surfacePos = vector.Lerp(traceParam.Start, traceParam.End, pathTravelled);
		return pathTravelled < 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Wraps TryGetValidFloorPosition
	//! Adds a pre-trace to allow better room finding in cluttered spaces like interiors and offices.
	//! \param[in] world Game world.
	//! \param[in] position Position to start search from.
	//! \param[in] minHeight MinHeight is the minimium length that the shape needs to be trace out to fit the height requirement for empty space above the floor.
	//! \param[in] traceParam TraceParam should specify flags, entities, shape, and size to trace. Start and end position are overwritten on reference.
	//! \param[out] If the scanned floor position is lower than this, the function will return false.
	//! \param[out] floorPos Represents the position of the floor detected by tracing downwards from the input position.
	//! \param[out] ceilingPos Ceiling position represents the ceiling detected by tracing upwards. Limited to floor + minHeight.
	//! \return True if a floor with enough empty space was found.
	static bool TryGetValidFloorPositionWithPreTrace(BaseWorld world, vector position, float minHeight, TraceParam traceParam, float minFloorY, TraceParam preTraceParam, out vector floorPos, out vector ceilingPos)
	{
		// Pre-scan with thin radius allows better detection of floors and ceilings in cluttered enviroments like offices.
		if (!TryGetValidFloorPosition(world, position, minHeight, preTraceParam, minFloorY, 3 * minHeight, floorPos, ceilingPos))
			return false;

		position = 0.5 * (floorPos + ceilingPos);
		return TryGetValidFloorPosition(world, position, minHeight, traceParam, minFloorY, 0, floorPos, ceilingPos);
	}

	//------------------------------------------------------------------------------------------------
	//! Tries to get SCR_EditableEntityUIInfo from entity.
	//! \param[in] entity Represents an in-game object with editable properties.
	//! \return the SCR_EditableEntityUIInfo associated with the entity.
	protected static SCR_EditableEntityUIInfo GetEntityUIInfo(IEntity entity)
	{
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(entity.FindComponent(SCR_EditableEntityComponent));
		if (!editableEntity)
			return null;
		return SCR_EditableEntityUIInfo.Cast(editableEntity.GetInfo());
	}

	//------------------------------------------------------------------------------------------------
	//! Calculates height from trace parameter supports derivatives of TraceBox or TraceSphere.
	//! \param[in] traceParam TraceParam to determine height of.
	//! \return height of trace.
	static float GetTraceParamHeight(TraceParam traceParam)
	{
		TraceBox traceBox = TraceBox.Cast(traceParam);
		if (traceBox) // Include OBB
			return traceBox.Maxs[1] - traceBox.Mins[1];

		TraceSphere traceSphere = TraceSphere.Cast(traceParam);
		if (traceSphere)
			return 2 * traceSphere.Radius;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Raises position to be above ocean height with optional offset.
	//! Will not do anything if the position is already above the ocean + offset.
	//! \param[in] world Game world.
	//! \param[in] position Position checked if bellow ocean and raised if it is.
	//! \param[in] offset Offset is an optional parameter that adds to the position's height above ocean level.
	//! \return the modified position with height adjusted to ocean level plus offset.
	static vector RaiseToTerrain(BaseWorld world, vector position, float offset = 0)
	{
		float oceanHeightAtPos = world.GetSurfaceY(position[0], position[2]);
		position[1] = Math.Max(oceanHeightAtPos + offset, position[1]);
		return position;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns minimum height allowed for a valid floor over terrain or ocean considering max submersion depth.
	//! \param[in] world Game world.
	//! \param[in] maxSubmersionDepth MaxSubmersionDepth represents the maximum depth at which the object can submerge in water.
	//! \return the minimum height of a valid floor.
	static float GetMinYOnTerrainOrOcean(BaseWorld world, vector position, float maxSubmersionDepth)
	{
		float terrainMin = world.GetSurfaceY(position[0], position[2]) - TRACE_EPSILON;
		float oceanMin = world.GetOceanHeight(position[0], position[2]) - maxSubmersionDepth - TRACE_EPSILON;
		return Math.Max(terrainMin, oceanMin);
	}

	//------------------------------------------------------------------------------------------------
	//! Squares a value.
	//! \param[in] Value
	//! \return square
	protected static float Pow2(float base)
	{
		return base * base;
	}

	//------------------------------------------------------------------------------------------------
	//! Clones properties from TraceParam to create a TraceSphere with small radius.
	//! Pre-traces allow better room finding in cluttered spaces like interiors and offices.
	//! \param[in] traceParam TraceParam represents input parameters for sphere tracing, including flags, layer mask, radii, exclusions, and inclusions.
	//! \return a pre-trace parameter with adjusted radius and same flags, layer mask, exclusions, inclusions, and arrays as
	static TraceSphere CreatePreTraceParam(TraceParam traceParam)
	{
		float traceMinRadius = 0.1;
		TraceBox traceBox = TraceBox.Cast(traceParam);
		if (traceBox) // Include OBB
		{
			traceMinRadius = 0.5 * Math.Min(Math.Min(traceBox.Maxs[0] - traceBox.Mins[0], traceBox.Maxs[1] - traceBox.Mins[1]), traceBox.Maxs[2] - traceBox.Mins[2]);
		}
		else
		{
			TraceSphere traceSphere = TraceSphere.Cast(traceParam);
			if (traceSphere)
				traceMinRadius = traceSphere.Radius;
		}

		TraceSphere preTraceParam = new TraceSphere();
		preTraceParam.Flags = traceParam.Flags;
		preTraceParam.LayerMask = traceParam.LayerMask;
		preTraceParam.Radius = 0.1 * traceMinRadius;
		preTraceParam.Exclude = traceParam.Exclude;
		preTraceParam.ExcludeArray = traceParam.ExcludeArray;
		preTraceParam.Include = traceParam.Include;
		preTraceParam.IncludeArray = traceParam.IncludeArray;
		return preTraceParam;
	}
}
