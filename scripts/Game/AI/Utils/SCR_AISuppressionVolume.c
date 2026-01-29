// Base class of volume object representing space used to calculate suppression lines
// Child classes can represent different shapes (bbox or sphere), can be static or dynamic (e.g. bbox attached to base target)
class SCR_AISuppressionVolumeBase
{	
	// Max angle at which end of suppression line can be placed relative to shooter position (right-left)
	protected static const float MAX_X_ANGLE_DEG = 12;
	
	// Min angle at which end of suppression line should be placed relative to shooter position (right-left)
	protected static const float MIN_X_ANGLE_DEG = 2;
	
	// Max angle at which end of suppression line can be placed relative to shooter position (top-down)
	protected static const float MAX_Y_ANGLE_DEG = 4;
	
	// Min angle at which end of suppression line should be placed relative to shooter position (top-down)
	protected static const float MIN_Y_ANGLE_DEG = 0.5;
	
	// Min Y above surface of suppression line positions
	protected static const float MIN_SURFACE_Y = 0.2;
	
	// Random chance of going at opposite direction (RandomFloat01 < CHANCE)
	protected static const float CHANCE_FOR_OPPOSITE_DIR = 0.150;
	
#ifdef WORKBENCH 	
	ref Shape m_DebugShape;
	
	//---------------------------------------------------------------------------------------
	protected void UpdateDebug()
	{
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_SUPPRESS_DEBUG))
			m_DebugShape = GetDebugShape();
		else
			if (m_DebugShape)
				delete m_DebugShape;
	}
#endif
		
	//---------------------------------------------------------------------------------------
	protected Shape GetDebugShape()
	{
		return null;
	}
		
	//---------------------------------------------------------------------------------------
	protected static void GetPositionsAreaBBox(array<vector> positions, out vector bbMin, out vector bbMax)
	{
		bbMin = vector.Zero;
		bbMax = vector.Zero;
		
		if (positions.Count() > 0)
		{
			bbMin = Vector(float.MAX, float.MAX, float.MAX);
			bbMax = Vector(-float.MAX, -float.MAX, -float.MAX);
		}
	
		foreach (vector position : positions)
		{
			for (int i = 0; i < 3; i++)
			{
				float posI = position[i];
				if (posI > bbMax[i])
					bbMax[i] = posI;
				if (posI < bbMin[i])
					bbMin[i] = posI;
			}
		}
	}
		
	//---------------------------------------------------------------------------------------
    protected static vector RotateDirection90Degrees(vector currentDirection, bool rotateRight = true)
    {
        vector newDirection;
		newDirection[1] = currentDirection[1]; // Y remains unchanged

        if (rotateRight)
        {
            // To rotate 90 degrees to the right, swap X and Z and negate the new X
            newDirection[0] = -currentDirection[2]; // New X is -Z
            newDirection[2] = currentDirection[0];  // New Z is X
        }
        else
        {
            // To rotate 90 degrees to the left, swap X and Z and negate the new Z
            newDirection[0] = currentDirection[2];  // New X is Z
            newDirection[2] = -currentDirection[0]; // New Z is -X
        }

        return newDirection;
    }
	
	//---------------------------------------------------------------------------------------
	protected vector GetOutsideEdgePos(vector direction)
	{
		return vector.Zero;
	}
	
	//---------------------------------------------------------------------------------------
	protected void GetEdgePositions(vector startPos, vector dir, out vector minEdgePos, out vector maxEdgePos, out float minEdgeDist, out float maxEdgeDist)
	{
		minEdgePos = GetOutsideEdgePos(dir * -1);
		minEdgeDist = vector.DistanceXZ(startPos, minEdgePos) * -1;
		
		maxEdgePos = GetOutsideEdgePos(dir);
		maxEdgeDist = vector.DistanceXZ(startPos, maxEdgePos);
	}
	
	//---------------------------------------------------------------------------------------
	protected void GetYRange(vector position, out float minY, out float maxY)
	{
		minY = 0;
		maxY = 0;
	}
	
	//---------------------------------------------------------------------------------------
	protected void GetWorldYRange(BaseWorld world, vector position, out float minWorldY, out float maxWorldY)
	{
		float minY, maxY;
		GetYRange(position, minY, maxY);	
		minWorldY = Math.Max(minY, world.GetSurfaceY(position[0], position[2]) + MIN_SURFACE_Y);
		maxWorldY = Math.Max(maxY, minWorldY);
	}
	
	//---------------------------------------------------------------------------------------
	bool IsInsideVolume2D(vector insidePos)
	{
		return false;
	}
	
	//---------------------------------------------------------------------------------------
	vector GetRandomPosition(notnull IEntity shooter, inout vector startPos = vector.Zero, vector lastLineDir = vector.Zero)
	{
		vector centerPos = GetCenterPosition();
		
		// Use center pos as default ref pos
		if (!startPos || startPos == vector.Zero)
			startPos = centerPos;
		
		// Save start pos original Y
		float startPosY = startPos[1];

		// Make everything 2D
		vector shooterPos = shooter.GetOrigin();
		shooterPos[1] = 0;
		startPos[1] = 0;
		centerPos[1] = 0;
		
		// Get direction sideways (rotate 90 degrees right)
        vector rightDir = RotateDirection90Degrees(vector.Direction(shooterPos, centerPos).Normalized());
		
		// Get edge positions (min is in opposite direction, max is forward)
		vector minEdgePos, maxEdgePos;
		float minEdgeDist, maxEdgeDist;
		GetEdgePositions(startPos, rightDir, minEdgePos, maxEdgePos, minEdgeDist, maxEdgeDist);
		
		vector sideDir = rightDir;
		
		float maxDist = maxEdgeDist;
		float minEdgeDistAbs = Math.AbsFloat(minEdgeDist);
		float maxDistToEdge = Math.Max(maxEdgeDist, minEdgeDistAbs);
		float lineLength = minEdgeDistAbs + maxEdgeDist;		
		
		// Get max possible dist based on angle
		float distToCenter = vector.Distance(shooterPos, centerPos);
		float minPossibleDist = Math.Tan(MIN_X_ANGLE_DEG * Math.DEG2RAD) * distToCenter;
		float maxPossibleDist = Math.Tan(MAX_X_ANGLE_DEG * Math.DEG2RAD) * distToCenter;
			
		// Last direction provided
		if (lastLineDir != vector.Zero)
		{
			float dot = vector.DotXZ(lastLineDir, sideDir);
			
			// Last dir is aligned with current dir
			// Use last general dir as default dir
			if (Math.AbsFloat(dot) > 0.9 && dot < 0)
			{
				sideDir = rightDir * -1;
				maxDist = minEdgeDistAbs;
			}
		}
				
		// Outside of BBox or given startPos is misaligned with suppression line
		if (!IsInsideVolume2D(startPos))
		{		
			// Start with min edge pos, line towards max edge
			startPos = minEdgePos;
			vector maxEndPos = maxEdgePos;
			
			// Go opposite if max edge is closer
			if (minEdgeDistAbs > Math.AbsFloat(maxEdgeDist))
			{
				startPos = maxEdgePos;
				maxEndPos = minEdgePos;
			}
			
			// Get final direction and max distance
			sideDir = vector.Direction(startPos, maxEndPos).Normalized();
			maxDist = vector.DistanceXZ(startPos, maxEndPos);
		}
		else
		// Inside BBox
		{	
			// Chance of going opposite direction
			if (Math.RandomFloat01() < CHANCE_FOR_OPPOSITE_DIR)
			{
				sideDir = rightDir * -1;
				maxDist = minEdgeDistAbs;				
			}
		}
		
		BaseWorld world = shooter.GetWorld();
		
		// Get start pos Y ranges
		float startPosMinY, startPosMaxY;
		GetWorldYRange(world, startPos, startPosMinY, startPosMaxY);
		
		// Use closest edge if start pos Y is out of bounds		
		startPos[1] = Math.Max(Math.Min(startPosY, startPosMaxY), startPosMinY);
				
		// It's possible minPossibleDist will make us go out of bounds
		// We ensure minPossibleDist is respected no matter what, so it can be used for movement balancing
		float endPosDistRandMax = Math.Max(minPossibleDist, Math.Min(maxPossibleDist, maxDist));
		float endPosDist;
		if (minPossibleDist == endPosDistRandMax)
			endPosDist = minPossibleDist;
		else
			endPosDist = Math.RandomFloatInclusive(minPossibleDist, endPosDistRandMax);
		
		// Get end pos
		vector endPos = startPos + (sideDir * endPosDist);
		
		// Get end pos Y ranges
		float endPosMinY, endPosMaxY;
		GetWorldYRange(world, endPos, endPosMinY, endPosMaxY);
		
		// Use closest edge if end pos Y is out of bounds		
		float endPosY = Math.Max(Math.Min(endPos[1], endPosMaxY), endPosMinY);
		
		// Get ranges
		float endPosYTopRange = endPosMaxY - endPosY;
		float endPosYBottomRange = endPosY - endPosMinY;
		
		// Assume movement towards the top
		int endPosYDir = 1;
		float endPosYRange = endPosYTopRange;
		
		// Get Y angle limits
		float minYDiff = Math.Tan(MIN_Y_ANGLE_DEG * Math.DEG2RAD) * distToCenter;
		float maxYDiff = Math.Tan(MAX_Y_ANGLE_DEG * Math.DEG2RAD) * distToCenter;
		
		// Min Y is larger than BBox Y range
		if (minYDiff > Math.Max(endPosYTopRange, endPosYBottomRange))
		{
			// Go down if large enough diff
			if (endPosYBottomRange > endPosYTopRange / 2)
			{
				endPosYDir = -1;
				minYDiff = endPosYBottomRange;
			}
			else
			{	
				// Cut Y diff in half, check if it's tolerable
				minYDiff *= 0.5;
				
				// Range still too high, cut to BBox
				if (minYDiff > endPosYTopRange)
					minYDiff = endPosYTopRange;
			}
			
			endPosYRange = minYDiff;			
		}
		else
		{
			// Use opposite direction if top too small or bottom has been drawn
			if (endPosYBottomRange > minYDiff && 
				(endPosYTopRange < minYDiff || Math.RandomFloat01() < (endPosYBottomRange / (endPosYBottomRange + endPosYTopRange))))
			{
				endPosYDir = -1;
				endPosYRange = endPosYBottomRange;
			}
		}
		
		// Get final end Y range with respect to min and max Y diffs
		float endYRange = Math.Max(maxYDiff, Math.Min(endPosYRange - minYDiff, maxYDiff - minYDiff));
		
		endPos[1] = endPosY + (Math.RandomFloatInclusive(minYDiff, endYRange) * endPosYDir);	
				
		return endPos;
	}
	
	//---------------------------------------------------------------------------------------
	vector GetCenterPosition()
	{
		return vector.Zero;
	}
}

class SCR_AISuppressionVolumeBox : SCR_AISuppressionVolumeBase
{
	vector m_vBBMin;
	vector m_vBBMax;
	
	//---------------------------------------------------------------------------------------
	void SCR_AISuppressionVolumeBox(vector bbMin, vector bbMax)
	{
		m_vBBMin = bbMin;
		m_vBBMax = bbMax;
		
#ifdef WORKBENCH
		UpdateDebug();
#endif
	}
	
	//---------------------------------------------------------------------------------------
	override protected Shape GetDebugShape()
	{		
		return Shape.Create(ShapeType.BBOX, Color.DARK_GREEN, ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, m_vBBMin, m_vBBMax);
	}
	
	//---------------------------------------------------------------------------------------
	override protected void GetYRange(vector position, out float minY, out float maxY)
	{
		minY = m_vBBMin[1];
		maxY = m_vBBMax[1];
	}
				
	//---------------------------------------------------------------------------------------
	override protected vector GetOutsideEdgePos(vector direction)
	{
		vector centerPos = GetCenterPosition();
		
		// Calculate the slope of the direction vector
		float slope = direction[0] / direction[2];
		
		// Calculate intersection with vertical edge
		float zEdge = m_vBBMin[2];
		if (direction[2] > 0)
			zEdge = m_vBBMax[2];
		float xIntersectVertical = slope * (zEdge - centerPos[2]) + centerPos[0];
		
		// Calculate intersection with horizontal edge
		float xEdge = m_vBBMin[0];
		if (direction[0] > 0)
			xEdge = m_vBBMax[0];
		float zIntersectHorizontal = (xEdge - centerPos[0]) / slope + centerPos[2];
		
		// Check if intersections are within bounds and get possible intersection points
		vector vEdgeIntersectPos = vector.Zero;
		if (xIntersectVertical >= m_vBBMin[0] && xIntersectVertical <= m_vBBMax[0])
			vEdgeIntersectPos = Vector(xIntersectVertical, 0, zEdge);
		
		vector hEdgeIntersectPos = vector.Zero;
		if (zIntersectHorizontal >= m_vBBMin[2] && zIntersectHorizontal <= m_vBBMax[2])
			hEdgeIntersectPos = Vector(xEdge, 0, zIntersectHorizontal);
		
		// Get closest valid edge position
		if (hEdgeIntersectPos == vector.Zero || vector.DistanceXZ(centerPos, vEdgeIntersectPos) < vector.DistanceXZ(centerPos, hEdgeIntersectPos))
			return vEdgeIntersectPos;
		else
			return hEdgeIntersectPos;
	}
	
	//---------------------------------------------------------------------------------------
	override bool IsInsideVolume2D(vector insidePos)
	{
		return Math.IsInRange(insidePos[0], m_vBBMin[0], m_vBBMax[0]) && Math.IsInRange(insidePos[2], m_vBBMin[2], m_vBBMax[2]);
	}
	
	//---------------------------------------------------------------------------------------
	override vector GetCenterPosition()
	{
		return 0.5 * (m_vBBMin + m_vBBMax);
	}
}

class SCR_AISuppressionVolumeSphere : SCR_AISuppressionVolumeBase
{
	vector m_vPos;
	float m_fRadius;
	
	//---------------------------------------------------------------------------------------
	void SCR_AISuppressionVolumeSphere(vector pos, float radius)
	{
		m_vPos = pos;
		m_fRadius = radius;
		
#ifdef WORKBENCH
		UpdateDebug();
#endif
	}
	
	//---------------------------------------------------------------------------------------
	override protected Shape GetDebugShape()
	{		
		return Shape.CreateSphere(Color.DARK_GREEN, ShapeFlags.NOZBUFFER | ShapeFlags.WIREFRAME, m_vPos, m_fRadius);
	}	
	
	//---------------------------------------------------------------------------------------
	override protected void GetYRange(vector position, out float minY, out float maxY)
	{
		vector centerPos = GetCenterPosition();
		float distToCenter2D = vector.DistanceXZ(centerPos, position);
		
		float maxDist = Math.Sqrt((m_fRadius * m_fRadius) - (distToCenter2D * distToCenter2D));
		
		maxY = centerPos[1] + maxDist;
		minY = centerPos[1] - maxDist;
	}
	
	//---------------------------------------------------------------------------------------
	override protected vector GetOutsideEdgePos(vector direction)
	{
		return m_vPos + direction * m_fRadius;
	}
	
	//---------------------------------------------------------------------------------------
	override bool IsInsideVolume2D(vector insidePos)
	{
		return vector.DistanceXZ(GetCenterPosition(), insidePos) < m_fRadius;
	}	
		
	//---------------------------------------------------------------------------------------
	override vector GetCenterPosition()
	{
		return m_vPos;
	}
}


class SCR_AISuppressionObjectVolumeBox : SCR_AISuppressionVolumeBox
{
	// Object volume scaling settings
	protected static const float OBJECT_VOLUME_MAX_SCALE_DISTANCE = 600.0;
	protected static const float OBJECT_VOLUME_MAX_SCALE = 5; // How much bigger volume will get at max scaling distance
	protected static const float OBJECT_VOLUME_MIN_SCALE = 1.5;
	
	protected static const float TARGET_MIN_SIZE = 3; // Minimal size of target in meters
	protected static const float OBJECT_VOLUME_MIN_Y = 4; // Min vertical size of object volume
	protected static const float NOT_RECOGNIZED_CLUSTER_SCALE = 3; // Scale multiplier for cluster volumes of not recognized targets
	
	//---------------------------------------------------------------------------------------
	static void ScaleTargetBBox(inout vector bbMin, inout vector bbMax, float distance, bool recognized = true)
	{		
		distance = Math.Clamp(distance, 0, OBJECT_VOLUME_MAX_SCALE_DISTANCE);
		float targetSize = Math.Max(vector.DistanceXZ(bbMin, bbMax), TARGET_MIN_SIZE);
		float factor = Math.Map(distance, 0, OBJECT_VOLUME_MAX_SCALE_DISTANCE, OBJECT_VOLUME_MIN_SCALE, OBJECT_VOLUME_MAX_SCALE);
		
		// Volumes for not recognized targets are much bigger
		if (!recognized)
			factor *= NOT_RECOGNIZED_CLUSTER_SCALE;
		
		float extraSize = targetSize * (factor - 1);
					
		bbMin[0] = bbMin[0] - extraSize;
		bbMin[2] = bbMin[2] - extraSize;
		bbMax[0] = bbMax[0] + extraSize;
		bbMax[2] = bbMax[2] + extraSize;
		
		// Add Y if not much vertical room
		if ((bbMax[1] - bbMin[1]) < OBJECT_VOLUME_MIN_Y)
		{
			bbMin[1] = bbMin[1] - OBJECT_VOLUME_MIN_Y / 2;
			bbMax[1] = bbMax[1] + OBJECT_VOLUME_MIN_Y / 2;
		}		
	}
}

class SCR_AISuppressionVolumeBaseTargetBox : SCR_AISuppressionObjectVolumeBox
{	
	protected ref BaseTarget m_BaseTarget;
	
	//---------------------------------------------------------------------------------------
	void SetTarget(BaseTarget baseTarget, IEntity shooter, vector destination)
	{
		m_BaseTarget = baseTarget;
		
		if (!m_BaseTarget)
			return;
			
		IEntity targetEnt = m_BaseTarget.GetTargetEntity();
		if (!targetEnt)
			return;
		
		vector lastSeenPos = m_BaseTarget.GetLastSeenPosition();
		
		// Get volume BBox
		array<vector> positions = {};
		
		if (lastSeenPos != vector.Zero)
			positions.Insert(lastSeenPos);
		
		if (destination != vector.Zero)
			positions.Insert(destination);
		
		GetPositionsAreaBBox(positions, m_vBBMin, m_vBBMax);
		
		// Get target local bounds
		vector localBBMin, localBBMax;
		targetEnt.GetBounds(localBBMin, localBBMax);
		
		// Adjust volume Y to target
		m_vBBMax[1] = m_vBBMax[1] + (localBBMax[1] - localBBMin[1]);
		
		// Scale BBox
		ScaleTargetBBox(m_vBBMin, m_vBBMax, m_BaseTarget.GetDistance());
		
#ifdef WORKBENCH
		// Update debug shape
		UpdateDebug();
#endif
	}
}

class SCR_AISuppressionVolumeClusterBox : SCR_AISuppressionObjectVolumeBox
{
	protected ref SCR_AITargetClusterState m_ClusterState;
		
	//---------------------------------------------------------------------------------------
	static SCR_AISuppressionVolumeClusterBox GetVolumeFromClusterState(SCR_AITargetClusterState clusterState)
	{
		SCR_AISuppressionVolumeClusterBox volume = new SCR_AISuppressionVolumeClusterBox(vector.Zero, vector.Zero);
		volume.SetClusterState(clusterState);
		return volume;
	}
	
	//---------------------------------------------------------------------------------------	
	SCR_AITargetClusterState GetClusterState()
	{
		return m_ClusterState;
	}
	
	//---------------------------------------------------------------------------------------
	void SetClusterState(SCR_AITargetClusterState clusterState, bool scaleTarget = true)
	{
		m_ClusterState = clusterState;		
		
		if (!m_ClusterState)
			return;
		
		m_vBBMin = m_ClusterState.m_vBBMin;
		m_vBBMax = m_ClusterState.m_vBBMax;
		
		// Scale BBox
		if (scaleTarget)
			ScaleTargetBBox(m_vBBMin, m_vBBMax, m_ClusterState.m_fDistMin, clusterState.m_iCountIdentified > 0);
		
#ifdef WORKBENCH
		UpdateDebug();
#endif
	}
}

class SCR_AISuppressionVolumeWaypoint : SCR_AISuppressionObjectVolumeBox
{
	protected AIWaypoint m_Waypoint;
	
	void SetWaypoint(notnull AIWaypoint wp, float height)
	{
		m_Waypoint = wp;
		
		// Take approximate BB based on waypoint radius and height
		float r = wp.GetCompletionRadius();
		vector pos = wp.GetOrigin();
		
		m_vBBMin = pos - Vector(r, 0, r);
		m_vBBMax = pos + Vector(r, height, r);
		
		#ifdef WORKBENCH
		UpdateDebug();
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	AIWaypoint GetWaypoint()
	{
		return m_Waypoint;
	}
}