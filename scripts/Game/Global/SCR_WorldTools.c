class SCR_WorldTools
{
	//------------------------------------------------------------------------------------------------
	//! Check for intersections within given cylinder.
	//! Performs 4 diagonal traces along cylinders circumference.
	//! \param[in] pos Cylinder center position
	//! \param[in] radius Cylinder radius
	//! \param[in] height Cylinder full height
	//! \param[in] flags Tracing flags
	//! \param[in] world World which is being traced
	//! \param[in] debugDrawMgr debug shape manager which will be used to draw trace results. Pass a null if drawing is not desired
	//! \return False if an intersection was found, true if the cylinder is devoid of obstacles
	static bool TraceCilinderUtil(vector pos, float radius = 0.5, float height = 2, TraceFlags flags = TraceFlags.ENTS | TraceFlags.OCEAN, BaseWorld world = null, SCR_DebugShapeManager debugDrawMgr = null)
	{
		if (!world)
			world = GetGame().GetWorld();

		float heightHalf = height * 0.5;

		autoptr TraceParam trace = new TraceParam();
		trace.Flags = flags;

		vector dir = Vector(radius, heightHalf, 0);
		trace.Start = pos + dir;
		trace.End = pos - dir;
		float coef = world.TraceMove(trace, null);
		if (coef < 1)
		{
			if (debugDrawMgr)
				DrawPositionValidationDebug(debugDrawMgr, trace, coef);

			return false;
		}

		dir = Vector(-radius, heightHalf, 0);
		trace.Start = pos + dir;
		trace.End = pos - dir;
		coef = world.TraceMove(trace, null);
		if (coef < 1)
		{
			if (debugDrawMgr)
				DrawPositionValidationDebug(debugDrawMgr, trace, coef);

			return false;
		}

		dir = Vector(0, heightHalf, radius);
		trace.Start = pos + dir;
		trace.End = pos - dir;
		coef = world.TraceMove(trace, null);
		if (coef < 1)
		{
			if (debugDrawMgr)
				DrawPositionValidationDebug(debugDrawMgr, trace, coef);

			return false;
		}

		dir = Vector(0, heightHalf, -radius);
		trace.Start = pos + dir;
		trace.End = pos - dir;
		coef = world.TraceMove(trace, null);
		if (coef < 1)
		{
			if (debugDrawMgr)
				DrawPositionValidationDebug(debugDrawMgr, trace, coef);

			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Find empty terrain position for a cylinder.
	//! Evaluates possible positions within area in hexagonal grid.
	//! \param[out] outPosition Variable filled with found position. When none was found, areaCenter will be filled.
	//! \param[in] areaCenter Center of queried area. Height is irrelevant, only terrain positions are queried.
	//! \param[in] areaRadius Radius of queried area
	//! \param[in] cylinderRadius Expected cylinder radius
	//! \param[in] cylinderHeight Expected cylinder full height
	//! \param[in] flags Tracing flags
	//! \param[in] world World which is being traced
	//! \param[in] debugDrawMgr debug shape manager which will be used to draw trace results. Pass a null if drawing is not desired
	//! \return True if position was found
	static bool FindEmptyTerrainPosition(out vector outPosition, vector areaCenter, float areaRadius, float cylinderRadius = 0.5, float cylinderHeight = 2, TraceFlags flags = TraceFlags.ENTS | TraceFlags.OCEAN, BaseWorld world = null, SCR_DebugShapeManager debugDrawMgr = null)
	{
		//--- Incorrect params
		if (areaRadius <= 0 || cylinderRadius <= 0 || cylinderHeight <= 0)
		{
			outPosition = areaCenter;
			return false;
		}

		if (!world)
		{
			world = GetGame().GetWorld();
#ifdef WORKBENCH // in workbench if we are not in play mode fetching world via GetGame().GetWorld() may return world from xob/prefab preview in workbench window rather than the one from world editor
			if (!GetGame().InPlayMode())
			{
				WorldEditor worldEditor = Workbench.GetModule(WorldEditor);
				if (!worldEditor)
					return false;

				WorldEditorAPI api = worldEditor.GetApi();
				if (!api)
					return false;

				world = api.GetWorld();
			}
#endif
		}

		//--- Precalculate vars
		float cellW = cylinderRadius * Math.Sqrt(3);
		float cellH = cylinderRadius * 2;
		vector cylinderVectorOffset = {0, cylinderHeight * 0.5, 0};
		int rMax = Math.Ceil(areaRadius / cylinderRadius / Math.Sqrt(3));
		TraceParam trace = new TraceParam();
		trace.Flags = flags | TraceFlags.WORLD;
		vector traceOffset = {0, 10, 0};
		TraceParam traceUp = new TraceParam();
		traceUp.Flags = flags | TraceFlags.WORLD; 
		vector traceUpOffset = {0, 10, 0};
		float maxDist = areaRadius - cylinderRadius;
		float maxDistSq = maxDist * maxDist;
		float posX, posY;
		int yMin, yMax, yStep;
		float traceCoef;
		for (int r; r < rMax; r++)
		{
			for (int x = -r; x <= r; x++)
			{
				posX = cellW * x;
				posY = cellH * (x - SCR_Math.fmod(x, 1)) * 0.5;

				yMin = Math.Max(-r - x, -r);
				yMax = Math.Min(r - x, r);
				if (Math.AbsInt(x) == r)
					yStep = 1;
				else
					yStep = yMax - yMin;

				for (int y = yMin; y <= yMax; y += yStep)
				{
					outPosition = areaCenter + {posX, 0, posY + cellH * y};
					if (vector.DistanceSqXZ(outPosition, areaCenter) > maxDistSq)
						continue;

					//--- Find nearest surface below (make sure it's not underground)
					trace.Start = outPosition;
					trace.End = outPosition - traceOffset;
					traceCoef = world.TraceMove(trace, null);
					outPosition[1] = Math.Max(trace.Start[1] - traceCoef * traceOffset[1] + 0.01, world.GetSurfaceY(outPosition[0], outPosition[2]));
					
					//--- Check if postion is inside the rock
					traceUp.Start = outPosition + Vector(0, 0.1, 0);
					traceUp.End = traceUp.Start + traceUpOffset;
					float traceUpCoef = world.TraceMove(traceUp, null);
					if (traceUpCoef < 1.0 && traceUp.TraceEnt && traceUp.TraceEnt.IsInherited(SCR_IndestructibleEnvironmentalEntity))
					{
						if (debugDrawMgr)
							DrawPositionValidationDebug(debugDrawMgr, traceUp, traceUpCoef);

						continue;
					}

					if (TraceCilinderUtil(outPosition + cylinderVectorOffset, cylinderRadius, cylinderHeight, flags, world, debugDrawMgr))
						return true;
				}
			}
		}
		outPosition = areaCenter;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Debug used for providing information about the position where trace has encountered an entity
	//! \param[in] debugDrawMgr debug shape manager which will hold onto the shape which will indicate the trace location, as well as the position where it hit the object, and what was hit
	//! \param[in] trace which was used for creation of the trace
	//! \param[in] traceCoef percentage of the path traveled by the trace, which is returned from TraceMove operation
	protected static void DrawPositionValidationDebug(notnull SCR_DebugShapeManager debugDrawMgr, notnull TraceParam trace, float traceCoef)
	{
		debugDrawMgr.AddArrow(trace.Start, trace.End, 0.3, Color.RED);
		if (!trace.TraceEnt)
			return;

		vector hitPos = trace.Start + (trace.End - trace.Start) * traceCoef;
		debugDrawMgr.AddArrow(hitPos + vector.Up * 5, hitPos, 0.3, Color.PINK);
		if (!trace.TraceEnt.GetPrefabData())
			return;

		BaseContainer prefab = trace.TraceEnt.GetPrefabData().GetPrefab();
		string rn;
		while (prefab && rn.IsEmpty())
		{
			rn = prefab.GetResourceName();
			prefab = prefab.GetAncestor();
		}

		if (!rn.IsEmpty())
			debugDrawMgr.AddText("Collision detected with " + SCR_FileIOHelper.GetShortResourceName(rn), hitPos + vector.Up * 7, 0.5);

		Print("SCR_WorldTools.DrawPositionValidationDebug: Trace obstructed by " + trace.TraceEnt, LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Find all empty terrain positions for a cylinder.
	Evaluates possible positions within area in hexagonal grid.
	\param[out] outPositions Array filled with found positions
	\param areaCenter Center of queried area. Height is irrelevant, only terrain positions are queried.
	\param areaRadius Radius of queried area
	\param cylinderRadius Expected cylinder radius
	\param cylinderHeight Expected cylinder full height
	\param maxResults Maximum number of desired results to return
	\param flags Tracing flags
	\param world World which is being traced
	\return Number of found positions
	*/
	static int FindAllEmptyTerrainPositions(out notnull array<vector> outPositions, vector areaCenter, float areaRadius, float cylinderRadius = 0.5, float cylinderHeight = 2, int maxResults = -1, TraceFlags flags = TraceFlags.ENTS | TraceFlags.OCEAN, BaseWorld world = null)
	{
		//--- Incorrect params
		if (areaRadius <= 0 || cylinderRadius <= 0 || cylinderHeight <= 0)
			return 0;

		if (!world)
			world = GetGame().GetWorld();

		//--- Precalculate vars
		float cellW = cylinderRadius * Math.Sqrt(3);
		float cellH = cylinderRadius * 2;
		vector cylinderVectorOffset = Vector(0, cylinderHeight * 0.5, 0);
		int rMax = Math.Ceil(areaRadius / cylinderRadius / Math.Sqrt(3));

		TraceParam trace = new TraceParam();
		trace.Flags = flags | TraceFlags.WORLD;
		vector traceOffset = Vector(0, 10, 0);

		vector position;
		float posX, posY;
		int yMin, yMax, yStep;
		float traceCoef;
		for (int r; r < rMax; r++)
		{
			for (int x = -r; x <= r; x++)
			{
				posX = cellW * x;
				posY = cellH * (x - SCR_Math.fmod(x, 1)) * 0.5;

				yMin = Math.Max(-r - x, -r);
				yMax = Math.Min(r - x, r);
				if (Math.AbsInt(x) == r)
					yStep = 1;
				else
					yStep = yMax - yMin;

				for (int y = yMin; y <= yMax; y += yStep)
				{
					position = areaCenter + Vector(posX, 0, posY + cellH * y);
					if (vector.DistanceXZ(position, areaCenter) > areaRadius - cylinderRadius)
						continue;

					//--- Find nearest surface below (make sure it's not underground)
					trace.Start = position;
					trace.End = position - traceOffset;
					traceCoef = world.TraceMove(trace, null);
					position[1] = Math.Max(trace.Start[1] - traceCoef * traceOffset[1] + 0.01, world.GetSurfaceY(position[0], position[2]));

					if (TraceCilinderUtil(position + cylinderVectorOffset, cylinderRadius, cylinderHeight, flags, world))
						outPositions.Insert(position);
				}
				
				if (maxResults > -1 && outPositions.Count() >= maxResults)
					break;
			}
		}
		return outPositions.Count();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Return true if position relative to object and physics skeleton bone offset is underwater.
	\param object Entity to test for.
	\param position Position relative to object.
	\param boneID Collider bone index. Collider position is applied as offset.
	\param depth Distance to water surface (optional output)
	\return isUnderwater Whether position is underwater
	*/
	static bool IsObjectUnderwater(notnull IEntity object, vector position = vector.Zero, int boneID = -1, out float depth = -1)
	{
		BaseWorld world = object.GetWorld();
		position = object.CoordToParent(position);

		if (boneID >= 0)
		{
			Physics physics = object.GetPhysics();
			if (physics)
				position += physics.GetGeomPosition(boneID);
		}

		vector outWaterSurfacePoint;
		EWaterSurfaceType outType;
		vector transformWS[4];
		vector obbExtents;

		bool isUnderwater = ChimeraWorldUtils.TryGetWaterSurface(world, position, outWaterSurfacePoint, outType, transformWS, obbExtents);
		if (isUnderwater)
			depth = vector.Distance(outWaterSurfacePoint, position);

		return isUnderwater;
	}
	
	//------------------------------------------------------------------------------------------------		
	static float GetWaterSurfaceY(BaseWorld world, vector worldPos, out EWaterSurfaceType waterSurfaceType, out float lakeArea)
	{		
		vector waterSurfacePos;
		vector transformWS[4];
		vector obbExtents;

		ChimeraWorldUtils.TryGetWaterSurface(world, worldPos, waterSurfacePos, waterSurfaceType, transformWS, obbExtents);
		lakeArea = obbExtents[0] * obbExtents[2];
		
		return waterSurfacePos[1];
	}
};
