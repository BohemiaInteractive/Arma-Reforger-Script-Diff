//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: false)]
class SCR_SoundGroup
{	
	[Attribute("", UIWidgets.EditBox, "Sound event name")]
	string  m_sSoundEvent;
	
	[Attribute("", UIWidgets.Object, "")]
	ref array<ref SCR_SoundType> m_aSoundType;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] randomPositionaSounds
	//! \param[in] cameraPosition
	//! \param[in] spawnDistance
	//! \param[out] soundType
	//! \param[out] position
	// Called by SCR_RandomPositionalSounds when searching for new sound to play
	void GetSoundTypeAndPosition(SCR_RandomPositionalSounds randomPositionaSounds, vector cameraPosition, float spawnDistance, out int soundType, out vector position)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] world
	//! \param[in] worldPos
	//! \param[out] waterSurfaceType
	//! \param[out] lakeArea
	protected float GetWaterSurfaceY(ChimeraWorld world, vector worldPos, out EWaterSurfaceType waterSurfaceType, out float lakeArea)
	{		
		vector waterSurfacePos;
		vector transformWS[4];
		vector obbExtents;

		ChimeraWorldUtils.TryGetWaterSurface(world, worldPos, waterSurfacePos, waterSurfaceType, transformWS, obbExtents);
		lakeArea = obbExtents[0] * obbExtents[2];
		
		return waterSurfacePos[1];
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_TreeSoundGroup : SCR_SoundGroup
{
	private const float TREE_LEAFY_HEIGHT_LIMIT = 10;
	private const float TREE_CONIFER_HEIGHT_LIMIT = 12;
	
	//------------------------------------------------------------------------------------------------
	//!
	override void GetSoundTypeAndPosition(SCR_RandomPositionalSounds randomPositionaSounds, vector cameraPosition, float spawnDistance, out int soundType, out vector position)
	{
		// Choose dominant tree type
		SCR_AmbientSoundsComponent ambientSoundsComponent = randomPositionaSounds.GetAmbientSoundsComponent();
		
		ETreeSoundTypes treeSoundType = ambientSoundsComponent.GetDominantTree();					
		if (treeSoundType == EQueryType.TreeConifer)
		{					
			IEntity tree = ambientSoundsComponent.GetRandomTree(EQueryType.TreeConifer, TREE_CONIFER_HEIGHT_LIMIT);
			if (!tree)
				return;
				
			soundType = 0;
			position = GetRandomPositionOnEntity(tree);			
		}
		else if  (treeSoundType == EQueryType.TreeLeafy)
		{
			IEntity tree = ambientSoundsComponent.GetRandomTree(EQueryType.TreeLeafy, TREE_LEAFY_HEIGHT_LIMIT);
			if (!tree)
				return;
				
			soundType = 1;
			position = GetRandomPositionOnEntity(tree);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	private vector GetRandomPositionOnEntity(const IEntity entity)
	{
		vector mins, maxs;
		entity.GetWorldBounds(mins, maxs);				
				
		vector pos;							
			
		// Get position
		pos = entity.GetOrigin();
				
		// Set random height
		pos[1] = pos[1] + (maxs[1] - mins[1]) * Math.RandomFloatInclusive(0.5, 1);
		
		return pos;	
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_PositionSoundGroup : SCR_SoundGroup
{
	protected const int LAKE_AREA_LIMIT = 100000;
	
	protected int m_iAngleOffset;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] spawnDistance
	//! Returns random position in local space
	vector GenerateRandomPositionLocal(float spawnDistance)
	{
		// Angle for random position is rotated by 36 deg for each GenerateRandomPosition() call
		const float angle = m_iAngleOffset * 36.0;
		
		m_iAngleOffset++;
		if (m_iAngleOffset > 9)
			m_iAngleOffset = 0;	
											
		return vector.FromYaw(angle) * spawnDistance;	
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_TerrainSoundGroup : SCR_PositionSoundGroup
{
	private const int COAST_HIGHT_LIMIT = 3;
		
	override void GetSoundTypeAndPosition(SCR_RandomPositionalSounds randomPositionaSounds, vector cameraPosition, float spawnDistance, out int soundType, out vector position)
	{
		position = cameraPosition + GenerateRandomPositionLocal(spawnDistance);
		
		// Get surface height
		float surfaceHeight = randomPositionaSounds.GetWorld().GetSurfaceY(position[0], position[2]);		
		position[1] = surfaceHeight;
		
		// Ignore when near sea coast
		if (Math.AbsFloat(position[1]) < COAST_HIGHT_LIMIT)	
		{
			return;
		}
					
		EWaterSurfaceType waterSurfaceType;
		float lakeArea;
		float waterSurfaceY = GetWaterSurfaceY(randomPositionaSounds.GetWorld(), position, waterSurfaceType, lakeArea);
		
		switch (waterSurfaceType)
		{
			case EWaterSurfaceType.WST_NONE:
			{
				// Trace and get soundInfo
				int soundInfo = -1;
				randomPositionaSounds.GetAmbientSoundsComponent().TracePointToTerrain(position, soundInfo);
						
				// Get soundGroup				
				soundType = randomPositionaSounds.GetTerrainType(soundInfo);
				return;
			}
			case EWaterSurfaceType.WST_OCEAN:
			{
				position[1] = 0;
				soundType = ETerrainType.OCEAN;
				return;
			}
			case EWaterSurfaceType.WST_POND:
			{
				if (lakeArea > LAKE_AREA_LIMIT)
				{
					position[1] = waterSurfaceY;
					soundType = ETerrainType.POND;
					return;
				}
				else
				{
					soundType = ETerrainType.POND_SMALL;
					return;
				}
			}
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_SoundMapSoundGroup : SCR_PositionSoundGroup
{	
	override void GetSoundTypeAndPosition(SCR_RandomPositionalSounds randomPositionaSounds, vector cameraPosition, float spawnDistance, out int soundType, out vector position)
	{
		ChimeraWorld world = randomPositionaSounds.GetWorld();
		
		position = cameraPosition + GenerateRandomPositionLocal(spawnDistance);
		
		// Check for fresh water
		float surfaceHeight = world.GetSurfaceY(position[0], position[2]);		
		position[1] = surfaceHeight;
							
		EWaterSurfaceType waterSurfaceType;
		float lakeArea;
		float waterSurfaceY = GetWaterSurfaceY(world, position, waterSurfaceType, lakeArea);
			
		switch (waterSurfaceType)
		{
			case EWaterSurfaceType.WST_NONE:
			{
				SoundWorld soundWorld = world.GetSoundWorld();
				
				if(!soundWorld)
					return;
				
				float sea, forest, city, meadow;
				soundWorld.GetMapValuesAtPos(position, sea, forest, city, meadow);
				
				// Get maximum
				float vals[] = { meadow, forest, city, sea };
				float v = vals[0];
				soundType = 0;
				for (int i = 1; i < 4; i++)
				{
					if (vals[i] > v)
					{
						v = vals[i];
						soundType = i;
					}
				}
				
				return;
			}
			case EWaterSurfaceType.WST_OCEAN:
			{
				position[1] = 0;
				soundType = ESoundMapType.SEA;
				return;
			}
			case EWaterSurfaceType.WST_POND:
			{
				if (lakeArea > LAKE_AREA_LIMIT)
				{
					position[1] = waterSurfaceY;
					soundType = ESoundMapType.POND;
					return;
				}
			}
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_VegetationSoundGroup : SCR_SoundGroup
{
	private const float TREE_HEIGHT_LIMIT = 5.0;
	private const float BUSH_HEIGHT_LIMIT = 0.5;
	
	//------------------------------------------------------------------------------------------------
	//!
	override void GetSoundTypeAndPosition(SCR_RandomPositionalSounds randomPositionaSounds, vector cameraPosition, float spawnDistance, out int soundType, out vector position)
	{					
		IEntity entity = randomPositionaSounds.GetAmbientSoundsComponent().GetRandomTree(EQueryType.TreeWithered, TREE_HEIGHT_LIMIT);		
		if (entity)
		{
			soundType = 0;
			position = entity.GetOrigin();
			position[1] = position[1] + TREE_HEIGHT_LIMIT;
			return;
		}
		
		entity = randomPositionaSounds.GetAmbientSoundsComponent().GetRandomTree(EQueryType.TreeBush, BUSH_HEIGHT_LIMIT);
		if (entity)
		{
			soundType = 1;
			position = entity.GetOrigin();
			return;
		}		
	}			
}