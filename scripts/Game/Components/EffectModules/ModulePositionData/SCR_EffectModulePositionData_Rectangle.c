/**
Zone data that will spawn the entity within a a ractanglar radius of the owner
*/
[BaseContainerProps(configRoot: true), BaseContainerCustomStringTitleField("Rectangle")]
class SCR_EffectsModulePositionData_Rectangle : SCR_BaseEffectsModulePositionData
{	
	[Attribute("5", desc: "Total width of the Zone. With the EffectsModule owner being the center", params: "0.25 inf")]	
	protected float m_fWidth;
	
	[Attribute("5", desc: "Total lenght of the Zone. With the EffectsModule owner being the center", params: "0.25 inf")]
	protected float m_fLength;

	[Attribute("0.1", desc: "The % of the edge where entities are not allowed to spawn. Ignored for lenght value if m_bSpawnInRow is true as projectiles will be spawned at a set distance", params: "0 0.9")]
	protected float m_fSpawnBufferPercentage;
	
	[Attribute("0.6", desc: "Distance in percentage from average lenght (width * lenght * 0.5 * m_fDistanceSpreadPercentage). If the random position is within the marge than the system will randomize a new position. It will retry a finite amount of times the higher the value the more chance it will rerandomize. Set this to 0 to simply ignore it and ranzomize", params: "0 0.9", precision: 5, category: "Projectile in barrage")]
	protected float m_fDistanceSpreadPercentage;

	[Attribute("-1", desc: "Minimal distance in meters between spawn positions. If -1 or more than the outcome of m_fDistanceSpreadPercentage then outcome of m_fDistanceSpreadPercentage will be used.", params: "-1 inf", category: "Projectile in barrage")]
	protected float m_fMinimalDistanceBetweenPositions;
	
	[Attribute("0.5", desc: "Percentage of zone percentage that is considered the center. If more than 0 and m_fCenterBias is more than 0 it will randomize a value between 0 and 1 and spawn the entity within the Center zone. If it is higher or equal to m_fCenterBias than it will spawn outside of the center zone. Note for Spawn in row only width is effected by Center zone", params: "0 1")]
	protected float m_fCenterZonePercentage;
	
	[Attribute("0.8", desc: "Percentage of chance to spawn entity in the center zone rather than in the entire zone. ignored if 1 or if m_fCenterZonePercentage is 0", params: "0 1")]
	protected float m_fCenterBias;
	
	[Attribute("1", desc: "If true will ignore m_fSpawnBufferPercentage and spawn projectile one after each other in the lenght of the zone (widght is still randomized) the distance between projectiles is lenght / projectile amount. Can only be used with barrages")]
	protected bool m_bSpawnInRow;
	
	[Attribute("0", desc: "If true the direction in which the barrage is moving will alternate with each barrage. Only applicable when SpawnInRow is true")]
	protected bool m_bShouldAlternateDirection;
	
	[Attribute("0.1", desc: "Similar to m_fSpawnBufferPercentage but only used on lenght when SpawnInRow is true. Will add slight variation betwen the distance of spawning transforms from current to prev. (ZSpawnPosition + (ZSpawnPosition * Math.RandomFloat(-m_fSpawnInRowLenghtVariantPercentage, m_fSpawnInRowLenghtVariantPercentage))", params: "0 0.5")]
	protected float m_fSpawnInRowLenghtVariantPercentage;
	
	protected ref array<vector> m_aPreviousTargetPositions;
	protected bool m_bIsMovingInAlternateDirection = false;
	
	//~ Max amount of times the target position will be randomized if to close to previous. Never set below 1
	protected const int MAX_TARGET_RERANDOMIZES = 10;
	
	/*!
	Get Width and Lenght for Area mesh generation
	\param[out] width Width of zone
	\param[out] lenght Lenght of zone
	*/
	void GetDimensions2D(out float width, out float lenght)
	{
		width = m_fWidth;
		lenght = m_fLength;
	}
	
	//------------------------------------------------------------------------------------------------
	override vector GetNewPosition(SCR_EffectsModule effectModule)
	{
		bool useCenterZone = false;

		if (m_fCenterBias > 0 && m_fCenterZonePercentage > 0)
		{
			useCenterZone = Math.RandomFloat01() <= m_fCenterBias;
		}
		
		vector randomLocalPosition;
		SCR_BarrageEffectsModule barrageEffect;
		
		if (m_bSpawnInRow)
		{
			barrageEffect = SCR_BarrageEffectsModule.Cast(effectModule);
			if (!barrageEffect)
				Print("'SCR_EffectsModulePositionData_Rectangle' with 'm_bSpawnInRow' set to true cannot be used if not 'SCR_BarrageEffectsModule'", LogLevel.ERROR);
		}
		
		if (m_bSpawnInRow && barrageEffect)
		{
			if (useCenterZone)
			{
				float centerZoneStartX = m_fWidth * m_fCenterZonePercentage * 0.5;

				randomLocalPosition[0] = Math.RandomFloatInclusive(-centerZoneStartX, centerZoneStartX);
			}
			else
			{
				float edgeStartX = (m_fWidth - m_fWidth * m_fSpawnBufferPercentage) * 0.5;

				randomLocalPosition[0] = Math.RandomFloatInclusive(-edgeStartX, edgeStartX);
			}
			
			float edgeStartY = (m_fLength - m_fLength * m_fSpawnBufferPercentage) * 0.5;
			float stepY = (m_fLength - m_fLength * m_fSpawnBufferPercentage) / (barrageEffect.GetTotalProjectilesInBarrage() - 1);

			if (m_bIsMovingInAlternateDirection)
				randomLocalPosition[2] = edgeStartY - stepY * (barrageEffect.GetTotalProjectilesInBarrage() - barrageEffect.GetProjectilesLeftInBarrage());
			else
				randomLocalPosition[2] = -edgeStartY + stepY * (barrageEffect.GetTotalProjectilesInBarrage() - barrageEffect.GetProjectilesLeftInBarrage());
				
			
			if (m_fSpawnInRowLenghtVariantPercentage > 0 && barrageEffect.GetTotalProjectilesInBarrage() != 0)
				randomLocalPosition[2] = randomLocalPosition[2] + (edgeStartY * 2 / barrageEffect.GetTotalProjectilesInBarrage()) * Math.RandomFloatInclusive(-m_fSpawnInRowLenghtVariantPercentage, m_fSpawnInRowLenghtVariantPercentage);

			return randomLocalPosition;
		}
		
		int targetRandomizesLeft = MAX_TARGET_RERANDOMIZES;

		float minDesiredDistanceSq = Math.Pow(m_fMinimalDistanceBetweenPositions, 2);
		float minAchievableDistance = Math.Pow(m_fWidth * m_fLength * 0.5 * m_fDistanceSpreadPercentage, 2);
		if (minDesiredDistanceSq > minAchievableDistance)
			minDesiredDistanceSq = minAchievableDistance;

		bool minDistanceAchived;
		while (targetRandomizesLeft > 0)
		{
			targetRandomizesLeft--;

			if (useCenterZone)
			{
				float centerZoneStartX = m_fWidth * m_fCenterZonePercentage / 2;
				float centerZoneStartY = m_fLength * m_fCenterZonePercentage / 2;
			
				randomLocalPosition[0] = Math.RandomFloatInclusive(-centerZoneStartX, centerZoneStartX);
				randomLocalPosition[2] = Math.RandomFloatInclusive(-centerZoneStartY, centerZoneStartY);
			}
			else
			{
				float edgeStartX = (m_fWidth - m_fWidth * m_fSpawnBufferPercentage) / 2;
				float edgeStartY = (m_fLength - m_fLength * m_fSpawnBufferPercentage) / 2;
			
				randomLocalPosition[0] = Math.RandomFloatInclusive(-edgeStartX, edgeStartX);
				randomLocalPosition[2] = Math.RandomFloatInclusive(-edgeStartY, edgeStartY);
			}
			
			if (minDesiredDistanceSq <= 0 || !m_aPreviousTargetPositions || m_aPreviousTargetPositions.IsEmpty())
				break;

			minDistanceAchived = true;
			foreach (vector previousPosition : m_aPreviousTargetPositions)
			{
				if (vector.DistanceSq(previousPosition, randomLocalPosition) < minDesiredDistanceSq)
				{
					minDistanceAchived = false;
					break;
				}
			}

			if (minDistanceAchived)
				break;
		}

		if (!m_aPreviousTargetPositions)
			m_aPreviousTargetPositions = {};

		m_aPreviousTargetPositions.Insert(randomLocalPosition);
		
		return randomLocalPosition;
	}

	//------------------------------------------------------------------------------------------------
	override void ResetPositions(SCR_EffectsModule effectModule)
	{
		super.ResetPositions(effectModule);
		
		m_aPreviousTargetPositions = null;
		if (m_bShouldAlternateDirection)
			m_bIsMovingInAlternateDirection = !m_bIsMovingInAlternateDirection;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_EffectsModulePositionData_Rectangle()
	{
		if ((m_fCenterBias > 0 && m_fCenterZonePercentage == 0) || (m_fCenterZonePercentage > 0 && m_fCenterBias == 0))
		{
			Print(string.Format("'SCR_EffectsModulePositionData_Rectangle' m_fCenterBias is '%1' while m_fCenterZonePercentage is '%2' both values should either be 0 (ingore center bias) or more than 0 (use center bias)", m_fCenterBias, m_fCenterZonePercentage), LogLevel.WARNING);
		}
		
		if (m_fSpawnBufferPercentage + m_fCenterZonePercentage > 1)
		{
			m_fCenterZonePercentage -= (m_fSpawnBufferPercentage + m_fCenterZonePercentage) - 1;
			Print(string.Format("'SCR_EffectsModulePositionData_Rectangle' m_fSpawnBufferPercentage + m_fCenterZonePercentage cannot exceed 1. So m_fCenterZonePercentage is decreased to %1!", m_fCenterZonePercentage), LogLevel.WARNING);
		}
	}
};