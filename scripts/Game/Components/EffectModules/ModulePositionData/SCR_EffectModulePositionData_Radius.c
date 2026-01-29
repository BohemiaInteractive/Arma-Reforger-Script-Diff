/**
Zone data that will spawn the entity within a radius of the owner
*/
[BaseContainerProps(configRoot: true), BaseContainerCustomStringTitleField("Radius")]
class SCR_EffectsModulePositionData_Radius : SCR_BaseEffectsModulePositionData
{	
	[Attribute("1", desc: "Radius in which the effects will either spawn in, or in case of a projectile will fly towards.", params: "1 inf")]
	protected float m_fRadius;

	[Attribute("0.1", desc: "The radius from the edge where entities are not allowed to spawn", params: "0 0.9")]
	protected float m_fSpawnBufferPercentage;
	
	[Attribute("0.6", desc: "Distance in percentage from total radius (or central radius) the next entity can be spawned. If the random position is within the marge than the system will randomize a new position. It will retry a finite amount of times the higher the value the more chance it will rerandomize. Set this to 0 to simply ignore it and ranzomize", params: "0 0.9", precision: 5, category: "Projectile in barrage")]
	protected float m_fDistanceSpreadPercentage;

	[Attribute("-1", desc: "Minimal distance in meters between spawn positions. If -1 or more than the outcome of m_fDistanceSpreadPercentage then outcome of m_fDistanceSpreadPercentage will be used.", params: "-1 inf", category: "Projectile in barrage")]
	protected float m_fMinimalDistanceBetweenPositions;
	
	[Attribute("0", desc: "If true will apply m_fDistanceSpreadPercentage on all entities that where spawned before not just the last spawned entity. This should make for a better spread but takes more processing power depending on the amount of entities")]
	protected bool m_bApplySpreadOnAllEntities;
	
	[Attribute("0.5", desc: "Percentage of radius that is considered the center zone. If more than 0 and m_fCenterBias is more than 0 it will randomize a value between 0 and 1 and spawn the entity within the CenterCenter zone. If it is higher or equal to m_fCenterBias than it will spawn outside of the center zone", params: "0 1")]
	protected float m_fCenterRadiusPercentage;
	
	[Attribute("0.8", desc: "Percentage of chance to spawn entity in the center zone rather than in the entire zone. ignored if 0 or if m_fCenterRadiusPercentage is 0", params: "0 1")]
	protected float m_fCenterBias;
	
	protected ref array<vector> m_aPreviousTargetPositions;
	
	//~ Max amount of times the target position will be randomized if to close to previous. Never set below 1
	protected const int MAX_TARGET_RERANDOMIZES = 10;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get radius of zone
	\return Radius
	*/
	float GetRadius()
	{
		return m_fRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get center radius of zone
	\return Radius
	*/
	float GetCenterRadius()
	{
		return m_fRadius * m_fCenterRadiusPercentage;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetSpawnRadius()
	{
		if (m_fSpawnBufferPercentage == 0)
			return m_fRadius;
		else 
			return m_fRadius * (1 - m_fSpawnBufferPercentage);
	}
	
	//------------------------------------------------------------------------------------------------
	override vector GetNewPosition(SCR_EffectsModule effectModule)
	{
		vector randomLocalPosition;

		//~ Make sure it randomizes a x amount of time
		int targetRandomizesLeft = MAX_TARGET_RERANDOMIZES;

		float minDesiredDistanceSq = Math.Pow(m_fMinimalDistanceBetweenPositions, 2);
		float minAchievableDistance;

		bool minDistanceAchived;
		bool useCenterRadius;
		float maxRadius;
		float minRadius;
		while (targetRandomizesLeft > 0)
		{
			targetRandomizesLeft--;
			
			minRadius = 0;
			useCenterRadius = false; 
			
			//~ Has center radius
			if (m_fCenterBias > 0 && m_fCenterRadiusPercentage > 0)
			{
				useCenterRadius = Math.RandomFloat01() <= m_fCenterBias;
				
				//~ Spawn in center radius
				if (useCenterRadius)
					maxRadius = GetCenterRadius();
				//~ Spawn in entire radius but center
				else 
					minRadius = GetCenterRadius();
			}

			//~ Set max radius as total radius
			if (!useCenterRadius) 
				maxRadius =  GetSpawnRadius();
			
			//~ Make sure GenerateRandomPointInRadius is valid
			if (maxRadius <= minRadius)
				maxRadius = minRadius + 0.001;
				
			//~ Get random point on radius taking into account the buffer percentage
			randomLocalPosition = SCR_Math2D.GenerateRandomPointInRadius(minRadius, maxRadius, vector.Zero, false);
			
			//~ No spread percentage
			if (m_fDistanceSpreadPercentage <= 0)
				break;

			//no previous target positions thus there is nothing to compare to
			if (!m_aPreviousTargetPositions || m_aPreviousTargetPositions.IsEmpty())
				break;

			minAchievableDistance = Math.Pow(maxRadius * m_fDistanceSpreadPercentage, 2);
			if (minDesiredDistanceSq > minAchievableDistance)
				minDesiredDistanceSq = minAchievableDistance;

			if (minDesiredDistanceSq <= 0)
				break;

			if (m_bApplySpreadOnAllEntities)
			{
				minDistanceAchived = true;
				//check if we are not close to any previously choosen position
				foreach(vector previousPosition : m_aPreviousTargetPositions)
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
			else 
			{
				//check if there is enough distance between current and last location
				if (vector.DistanceSq(m_aPreviousTargetPositions[m_aPreviousTargetPositions.Count() - 1], randomLocalPosition) >= minDesiredDistanceSq)
					break;
			}
		}

		if (!m_aPreviousTargetPositions)
			m_aPreviousTargetPositions = {};

		m_aPreviousTargetPositions.Insert(randomLocalPosition);

		return randomLocalPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	override void ResetPositions(SCR_EffectsModule effectModule)
	{
		m_aPreviousTargetPositions = null;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_EffectsModulePositionData_Radius()
	{
		if ((m_fCenterBias > 0 && m_fCenterRadiusPercentage == 0) || (m_fCenterRadiusPercentage > 0 && m_fCenterBias == 0))
		{
			Print(string.Format("'SCR_EffectsModulePositionData_Radius' m_fCenterBias is '%1' while m_fCenterRadiusPercentage is '%2' both values should either be 0 (ingore center bias) or more than 0 (use center bias)", m_fCenterBias, m_fCenterRadiusPercentage), LogLevel.WARNING);
		}
		
		if (m_fSpawnBufferPercentage + m_fCenterRadiusPercentage > 1)
		{
			m_fCenterRadiusPercentage -= (m_fSpawnBufferPercentage + m_fCenterRadiusPercentage) - 1;
			Print(string.Format("'SCR_EffectsModulePositionData_Radius' m_fSpawnBufferPercentage + m_fCenterRadiusPercentage cannot exceed 1. So m_fCenterRadiusPercentage is decreased to %1!", m_fCenterRadiusPercentage), LogLevel.WARNING);
		}
	}
};