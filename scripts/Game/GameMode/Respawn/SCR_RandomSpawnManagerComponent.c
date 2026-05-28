[ComponentEditorProps(category: "GameMode/Respawn", description: "Dynamically generates and manages random spawn points.")]
class SCR_RandomSpawnManagerComponentClass : SCR_BaseGameModeComponentClass
{
}

class SCR_RandomSpawnManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute("20", params: "1 inf", desc: "Target cached spawnpoint amount for quicker simultaneous spawn")]
	protected int m_iCachedSpawnPointCount;

	[Attribute("300", params: "0 inf", desc: "Safe distance from other spawnpoints")]
	protected float m_fSafeDistance;

	[Attribute("2500", params: "1 inf", desc: "Target amount of cells to split the terrain into")]
	protected int m_iTargetCellDensity;

	[Attribute("10", params: "1 inf", desc: "Number of spawn points to generate per batch frame")]
	protected int m_iGenerationBatchSize;

	[Attribute("20", params: "1 inf", desc: "Max emergency spawn generation attempts")]
	protected int m_iEmergencyAttempts;

	[Attribute(category: "Blacklist Zones", desc: "Blacklisted parts of the map")]
	protected ref array<ref SCR_SpawnPointBlacklistZone> m_aBlacklistZones;

	protected const float MAX_SURFACE_DELTA = 0.8;
	protected const float PLAYER_CYLINDER_HEIGHT = 2;
	protected const float PLAYER_CYLINDER_RADIUS = 0.5;
	protected const float WATER_LEVEL_THRESHOLD = 1;
	protected const float RAYCAST_HEIGHT_OFFSET = 10;
	protected const float MIN_CELL_SIZE = 10;
	protected const float JUMP_GOLDEN_RATIO = 0.6180339887;

	protected ref array<vector> m_aPrecalculatedSpawns = {};
	protected ref array<vector> m_aSafeCells = {};
	protected bool m_bIsGenerating;
	protected float m_fCellSize, m_fHalfCellOffset, m_fRayLength, m_fSafeDistanceSq;
	protected int m_iBatchDelayMs, m_iLastUsedIndex;
	protected TraceFlags m_TraceFlags;
	protected ref TraceParam m_TraceParam;
	protected vector m_vCylinderVectorOffset, m_vTraceOffset;
	protected BaseWorld m_World;

	//------------------------------------------------------------------------------------------------
	override void OnGameModeStart()
	{
		if (!Replication.IsServer())
			return;

		super.OnGameModeStart();
		if (!GetGame().InPlayMode())
			return;

		InitializeGrid();
	}

	//------------------------------------------------------------------------------------------------
	protected void InitializeGrid()
	{
		vector mins, maxs, cellCenter = vector.Zero;
		float worldWidth, worldLength, totalArea, cellArea;
		float halfCell, safeDistLimit, safeDistSq, effectiveRadius;
		int blCount, basesCount;
		bool bNeedsRandomSpawns = false;
		bool cellIsSafe;

		array<SCR_SpawnPoint> initialSpawns;
		array<vector> staticBaseOrigins = {};
		array<vector> blacklistCenters = {};
		array<float> blacklistSqRadii = {};

		m_aSafeCells.Clear();
		m_World = GetGame().GetWorld();

		if (!m_World)
			return;

		initialSpawns = SCR_SpawnPoint.GetSpawnPoints();

		if (initialSpawns)
		{
			foreach (SCR_SpawnPoint spawnPoint : initialSpawns)
			{
				if (!spawnPoint)
					continue;

				if (spawnPoint.IsSpawnPointRandom())
				{
					bNeedsRandomSpawns = true;
				}
				else
				{
					IEntity parent = spawnPoint.GetParent();
					if (parent)
					{
						SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(parent.FindComponent(SCR_CampaignMilitaryBaseComponent));
						if (base && !base.IsInitialized())
							continue;
					}

					staticBaseOrigins.Insert(spawnPoint.GetOrigin());
				}
			}
		}

		if (!bNeedsRandomSpawns)
			return;

		m_fRayLength = RAYCAST_HEIGHT_OFFSET * 2;
		m_vTraceOffset = vector.Up * RAYCAST_HEIGHT_OFFSET;

		GetGame().GetWorldEntity().GetTerrain(0, 0).GetTerrainBoundBox(mins, maxs);
		worldWidth = maxs[0] - mins[0];
		worldLength = maxs[2] - mins[2];
		totalArea = worldWidth * worldLength;

		cellArea = totalArea / m_iTargetCellDensity;
		m_vCylinderVectorOffset = { 0, PLAYER_CYLINDER_HEIGHT * 0.5, 0 };
		m_fSafeDistanceSq = m_fSafeDistance * m_fSafeDistance;
		m_fCellSize = Math.Sqrt(cellArea);

		if (m_fCellSize < MIN_CELL_SIZE)
			m_fCellSize = MIN_CELL_SIZE;

		m_fHalfCellOffset = m_fCellSize * 0.45;
		halfCell = m_fCellSize * 0.5;

		if (m_aBlacklistZones)
		{
			foreach (SCR_SpawnPointBlacklistZone zone : m_aBlacklistZones)
			{
				if (!zone || zone.m_fRadius <= 0)
					continue;

				blacklistCenters.Insert(zone.m_vCenter);
				effectiveRadius = zone.m_fRadius + halfCell;
				blacklistSqRadii.Insert(effectiveRadius * effectiveRadius);
			}
		}

		safeDistLimit = m_fSafeDistance + halfCell;
		safeDistSq = safeDistLimit * safeDistLimit;
		blCount = blacklistCenters.Count();
		basesCount = staticBaseOrigins.Count();

		for (float x = mins[0], maxX = maxs[0]; x < maxX; x += m_fCellSize)
		{
			for (float z = mins[2], maxY = maxs[2]; z < maxY; z += m_fCellSize)
			{
				cellCenter[0] = x + halfCell;
				cellCenter[2] = z + halfCell;

				if (m_World.GetSurfaceY(cellCenter[0], cellCenter[2]) <= WATER_LEVEL_THRESHOLD)
					continue;

				cellIsSafe = true;
				for (int i; i < blCount; i++)
				{
					if (vector.DistanceSqXZ(cellCenter, blacklistCenters[i]) <= blacklistSqRadii[i])
					{
						cellIsSafe = false;
						break;
					}
				}

				if (cellIsSafe && basesCount > 0)
				{
					for (int i; i < basesCount; i++)
					{
						if (vector.DistanceSqXZ(cellCenter, staticBaseOrigins[i]) < safeDistSq)
						{
							cellIsSafe = false;
							break;
						}
					}
				}

				if (cellIsSafe)
					m_aSafeCells.Insert(cellCenter);
			}
		}

		if (!m_aSafeCells.IsEmpty())
		{
			m_iLastUsedIndex = Math.RandomInt(0, m_aSafeCells.Count());
			WakeUpGenerator();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] playerFaction
	//! \param[in] currentWorldSpawns
	//! \param[out] outPos
	//! \return
	bool RequestSpawnPosition(FactionKey playerFaction, array<SCR_SpawnPoint> currentWorldSpawns, out vector outPos)
	{
		vector candidatePos;
		for (int i = m_aPrecalculatedSpawns.Count() - 1; i >= 0; i--)
		{
			candidatePos = m_aPrecalculatedSpawns[i];
			m_aPrecalculatedSpawns.Remove(i);

			if (IsDynamicallySafe(candidatePos, playerFaction, currentWorldSpawns))
			{
				outPos = candidatePos;
				WakeUpGenerator();
				return true;
			}
		}

		vector emergencyPos;
		int emergencyAttempts = 0;
		while (emergencyAttempts < m_iEmergencyAttempts && !m_aSafeCells.IsEmpty())
		{
			if (TryGenerateSinglePoint(emergencyPos) && IsDynamicallySafe(emergencyPos, playerFaction, currentWorldSpawns))
			{
				outPos = emergencyPos;
				WakeUpGenerator();
				return true;
			}

			emergencyAttempts++;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void WakeUpGenerator()
	{
		if (m_bIsGenerating || m_aSafeCells.IsEmpty())
			return;

		if (m_aPrecalculatedSpawns.Count() < m_iCachedSpawnPointCount)
		{
			m_bIsGenerating = true;
			GetGame().GetCallqueue().CallLater(ProcessGenerationBatch, m_iBatchDelayMs, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ProcessGenerationBatch()
	{
		int processedThisFrame = 0;
		vector newPos;

		while (m_aPrecalculatedSpawns.Count() < m_iCachedSpawnPointCount && !m_aSafeCells.IsEmpty())
		{
			if (processedThisFrame >= m_iGenerationBatchSize)
			{
				GetGame().GetCallqueue().CallLater(ProcessGenerationBatch, m_iBatchDelayMs, false);
				return;
			}

			if (TryGenerateSinglePoint(newPos))
				m_aPrecalculatedSpawns.Insert(newPos);

			processedThisFrame++;
		}

		m_bIsGenerating = false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool TryGenerateSinglePoint(out vector outPos)
	{
		int availableCells = m_aSafeCells.Count();
		if (availableCells == 0)
			return false;
		
		// Quasirandom jump roughly 61.8% across the sequentially-ordered array to find furthest point almost randomly
		int jumpAmount = availableCells * JUMP_GOLDEN_RATIO;
		m_iLastUsedIndex = (m_iLastUsedIndex + jumpAmount) % availableCells;

		int chosenIndex = m_iLastUsedIndex;
		vector cellCenter = m_aSafeCells[chosenIndex];
		
		vector testPos;
		float surfaceY, traceCoef, hitY;

		for (int attempt = 0; attempt < 20; attempt++)
		{
			testPos = cellCenter;
			testPos[0] = testPos[0] + Math.RandomFloat(-m_fHalfCellOffset, m_fHalfCellOffset);
			testPos[2] = testPos[2] + Math.RandomFloat(-m_fHalfCellOffset, m_fHalfCellOffset);

			surfaceY = m_World.GetSurfaceY(testPos[0], testPos[2]);
			testPos[1] = surfaceY;

			if (testPos[1] <= WATER_LEVEL_THRESHOLD)
				continue;

			if (ChimeraWorldUtils.TryGetWaterSurfaceSimple(m_World, testPos))
				continue;

			if (!m_TraceParam)
				m_TraceParam = new TraceParam();

			m_TraceFlags = TraceFlags.ENTS | TraceFlags.OCEAN;
			m_TraceParam.Flags = m_TraceFlags | TraceFlags.WORLD;
			m_TraceParam.Start = testPos + m_vTraceOffset;
			m_TraceParam.End = testPos - m_vTraceOffset;

			traceCoef = m_World.TraceMove(m_TraceParam, null);
			hitY = m_TraceParam.Start[1] - (traceCoef * m_fRayLength);

			if (hitY - surfaceY > MAX_SURFACE_DELTA)
				continue;

			testPos[1] = Math.Max(hitY + 0.01, surfaceY);

			if (SCR_WorldTools.TraceCylinder(testPos + m_vCylinderVectorOffset, PLAYER_CYLINDER_RADIUS, PLAYER_CYLINDER_HEIGHT, m_TraceFlags, m_World))
			{
				outPos = testPos;
				return true;
			}
		}

		int lastIndex = availableCells - 1;
		m_aSafeCells[chosenIndex] = m_aSafeCells[lastIndex];
		m_aSafeCells.Remove(lastIndex);

		if (m_iLastUsedIndex == lastIndex)
			m_iLastUsedIndex = chosenIndex;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsDynamicallySafe(vector pos, FactionKey playerFaction, array<SCR_SpawnPoint> currentWorldSpawns)
	{
		if (!currentWorldSpawns)
			return true;

		FactionKey spFaction;
		foreach (SCR_SpawnPoint spawnPoint : currentWorldSpawns)
		{
			if (!spawnPoint || !spawnPoint.IsSpawnPointEnabled())
				continue;

			spFaction = spawnPoint.GetFactionKey();
			if (spFaction != playerFaction && spFaction != string.Empty)
			{
				if (vector.DistanceSqXZ(pos, spawnPoint.GetOrigin()) < m_fSafeDistanceSq)
					return false;
			}
		}

		return true;
	}
}
