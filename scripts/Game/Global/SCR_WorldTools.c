class SCR_WorldTools
{
	//------------------------------------------------------------------------------------------------
	/*!
	Check for intersections within given cylinder.
	Performs 4 diagonal traces along cylinders circumference.
	\param pos Cylinder center position
	\param radius Cylinder radius
	\param height Cylinder full height
	\param flags Tracing flags
	\param world World which is being traced
	\return False if an intersection was found, true if the cylinder is devoid of obstacles
	*/
	static bool TraceCylinder(vector pos, float radius = 0.5, float height = 2, TraceFlags flags = TraceFlags.ENTS | TraceFlags.OCEAN, BaseWorld world = null)
	{
		if (!world)
			world = GetGame().GetWorld();

		float heightHalf = height * 0.5;

		autoptr TraceParam trace = new TraceParam();
		trace.Flags = flags;

		vector dir = Vector(radius, heightHalf, 0);
		trace.Start = pos + dir;
		trace.End = pos - dir;
		if (world.TraceMove(trace, null) < 1)
			return false;

		dir = Vector(-radius, heightHalf, 0);
		trace.Start = pos + dir;
		trace.End = pos - dir;
		if (world.TraceMove(trace, null) < 1)
			return false;

		dir = Vector(0, heightHalf, radius);
		trace.Start = pos + dir;
		trace.End = pos - dir;
		if (world.TraceMove(trace, null) < 1)
			return false;

		dir = Vector(0, heightHalf, -radius);
		trace.Start = pos + dir;
		trace.End = pos - dir;
		if (world.TraceMove(trace, null) < 1)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Find empty terrain position for a cylinder.
	Evaluates possible positions within area in hexagonal grid.
	\param[out] outPosition Variable filled with found position. When none was found, areaCenter will be filled.
	\param areaCenter Center of queried area. Height is irrelevant, only terrain positions are queried.
	\param areaRadius Radius of queried area
	\param cylinderRadius Expected cylinder radius
	\param cylinderHeight Expected cylinder full height
	\param flags Tracing flags
	\param world World which is being traced
	\return True if position was found
	*/
	static bool FindEmptyTerrainPosition(out vector outPosition, vector areaCenter, float areaRadius, float cylinderRadius = 0.5, float cylinderHeight = 2, TraceFlags flags = TraceFlags.ENTS | TraceFlags.OCEAN, BaseWorld world = null)
	{
		//--- Incorrect params
		if (areaRadius <= 0 || cylinderRadius <= 0 || cylinderHeight <= 0)
		{
			outPosition = areaCenter;
			return false;
		}

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
					outPosition = areaCenter + Vector(posX, 0, posY + cellH * y);
					if (vector.DistanceXZ(outPosition, areaCenter) > areaRadius - cylinderRadius)
						continue;

					//--- Find nearest surface below (make sure it's not underground)
					trace.Start = outPosition;
					trace.End = outPosition - traceOffset;
					traceCoef = world.TraceMove(trace, null);
					outPosition[1] = Math.Max(trace.Start[1] - traceCoef * traceOffset[1] + 0.01, world.GetSurfaceY(outPosition[0], outPosition[2]));

					if (TraceCylinder(outPosition + cylinderVectorOffset, cylinderRadius, cylinderHeight, flags, world))
						return true;
				}
			}
		}
		outPosition = areaCenter;
		return false;
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

					if (TraceCylinder(position + cylinderVectorOffset, cylinderRadius, cylinderHeight, flags, world))
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
