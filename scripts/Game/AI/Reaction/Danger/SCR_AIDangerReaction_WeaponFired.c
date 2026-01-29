[BaseContainerProps()]
class SCR_AIDangerReaction_WeaponFired : SCR_AIDangerReaction
{
	protected static const float SOUND_SPEED_MS = 343.0; // At Earth, sea level, 20 deg.c
	protected static const float ENDANGERING_FOR_GROUP_RADIUS = 15; // Gunshot is endangering for group at this distance and lower, even if it's not a fly-by
	protected static const float PROJECTILE_FLYBY_RADIUS = 13;
	protected static const float PROJECTILE_FLYBY_RADIUS_SQ = PROJECTILE_FLYBY_RADIUS * PROJECTILE_FLYBY_RADIUS;
	protected static const float PROJECTILE_FLYBY_DELAY_S = 0.05;
	protected static const float AUDIBLE_DISTANCE_NORMAL = 500.0; // Audible distance for normal gunshots
	protected static const float AUDIBLE_DISTANCE_SUPPRESSED = 100.0; // Audible distance for suppressed gunshots
	
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount)
	{
		AIDangerEventWeaponFire eventWeaponFire = AIDangerEventWeaponFire.Cast(dangerEvent);		
		IEntity shooter = eventWeaponFire.GetObject();
		
		if (!shooter || !eventWeaponFire)
			return false;
		
		IEntity instigatorEntity = eventWeaponFire.GetInstigatorEntity();
		if (!instigatorEntity)
			return false;
		
		// Check faction relations, ignore if not enemy or there is no faction
		Faction instigatorFaction = SCR_AIFactionHandling.GetEntityPerceivedFaction(instigatorEntity);
		
		if (!instigatorFaction)
			return false;
		
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
		
		bool myFactionIsMilitary = utility.IsMilitary();
		if (myFactionIsMilitary && !agent.IsEnemy(instigatorFaction))
			return false;
		
		// Get root entity of shooter, in case it is turret in vehicle hierarchy
		vector shotPos = eventWeaponFire.GetPosition();
		vector shotDir = eventWeaponFire.GetDirection();
		bool isShotSuppressed = eventWeaponFire.IsSuppressed();
		
		vector myOrigin = utility.m_OwnerEntity.GetOrigin();
		float distance = vector.Distance(myOrigin, shotPos);
		
		// Is it a flyby?
		bool isFlyby = IsFlyby(myOrigin, shotPos, shotDir, distance);
		
		// Is it audible?
		float maxAudibleDistance;
		if (isShotSuppressed)
			maxAudibleDistance = AUDIBLE_DISTANCE_SUPPRESSED;
		else
			maxAudibleDistance = AUDIBLE_DISTANCE_NORMAL;
		bool isAudible = distance < maxAudibleDistance;
		
		
		float timeTillFlyby_s = float.MAX;
		float timeTillGunshotHeard_s = float.MAX;
		if (isFlyby)
		{
			float projectileSpeed = eventWeaponFire.GetInitialSpeed();
			WorldTimestamp eventTimestamp = eventWeaponFire.GetTimestamp();
			float flightTime_s = distance / projectileSpeed;
			WorldTimestamp flybyTimestamp = eventTimestamp.PlusSeconds(flightTime_s + PROJECTILE_FLYBY_DELAY_S);
			timeTillFlyby_s = flybyTimestamp.DiffSeconds(GetGame().GetWorld().GetTimestamp());
			
			if (timeTillFlyby_s < 0)
				OnProjectileFlyby(utility, dangerEventCount, shotPos);
			else
			{
				utility.GetCallqueue().CallLater(OnProjectileFlyby, 1000*timeTillFlyby_s, false,
					utility, dangerEventCount, shotPos);
			}
		}
		
		if (isAudible)
		{
			WorldTimestamp eventTimestamp = eventWeaponFire.GetTimestamp();
			float wavefrontTravelTime_s = distance / SOUND_SPEED_MS;
			WorldTimestamp wavefrontArrivalTimestamp = eventTimestamp.PlusSeconds(wavefrontTravelTime_s);
			timeTillGunshotHeard_s = wavefrontArrivalTimestamp.DiffSeconds(GetGame().GetWorld().GetTimestamp());
			
			// Ignore the gunshot sound if projectile flies by sooner than sound of gunshot.
			// This is general case for majority of weapons. We don't want to notify the threat system twice.
			// Threat system was not tuned to be notified twice in such cases.
			bool ignoreGunshotHeard = isFlyby && timeTillFlyby_s < timeTillGunshotHeard_s;
			
			if (!ignoreGunshotHeard)
			{
				if (timeTillGunshotHeard_s < 0)
					OnGunshotHeard(utility, distance, dangerEventCount, shotPos);
				else
				{
					utility.GetCallqueue().CallLater(OnGunshotHeard, 1000*timeTillGunshotHeard_s, false,
						utility, distance, dangerEventCount, shotPos);
				}
			}
		}
		
		if (isFlyby || isAudible)
		{
			// Notify our group, only if we are a leader
			bool endangeringForGroup = isFlyby || distance < ENDANGERING_FOR_GROUP_RADIUS;
			
			AIGroup myGroup = utility.GetOwner().GetParentGroup();
			if (myGroup && myGroup.GetLeaderAgent() == agent)
			{
				float timeTillGroupNotified_s = Math.Min(timeTillFlyby_s, timeTillGunshotHeard_s);
				
				if (timeTillGroupNotified_s < 0)
					NotifyGroup(myGroup, shooter, instigatorEntity, instigatorFaction, shotPos, endangeringForGroup);
				else
				{
					utility.GetCallqueue().CallLater(NotifyGroup, 1000*timeTillGroupNotified_s, false,
						myGroup, shooter, instigatorEntity, instigatorFaction, shotPos, endangeringForGroup);
				}
			}
		}
		
		return true;
	}
	
	void OnGunshotHeard(notnull SCR_AIUtilityComponent utility, float distance, int dangerEventCount, vector pos)
	{
		utility.m_ThreatSystem.ThreatShotFired(distance, dangerEventCount);
		utility.m_SectorThreatFilter.OnShotsFired(pos, dangerEventCount, false);
	}
	
	void OnProjectileFlyby(notnull SCR_AIUtilityComponent utility, int dangerEventCount, vector pos)
	{
		utility.m_ThreatSystem.ThreatProjectileFlyby(dangerEventCount);
		utility.m_SectorThreatFilter.OnShotsFired(pos, dangerEventCount, true);
	}
	
	void NotifyGroup(AIGroup group, IEntity shooterEntity, IEntity instigatorEntity, Faction faction, vector posWorld, bool endangering)
	{
		// This can be called delayed, so check if group still exists
		if (!group)
			return;
		
		PerceptionManager pm = GetGame().GetPerceptionManager();
		SCR_AIGroupUtilityComponent groupUtilityComp = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
		
		if (!groupUtilityComp || !pm)
			return;
		
		// Resolve which entity to report to group perception
		// For static turrets it's Instigator entity (character),
		// For turrets in vehicles it's root of vehicle
		
		IEntity targetEntity; // Target entity for group perception
		
		if (ChimeraCharacter.Cast(shooterEntity))
			targetEntity = shooterEntity; // Character
		else if (Turret.Cast(shooterEntity))
		{
			IEntity shooterEntityRoot = shooterEntity.GetRootParent();
			if (Vehicle.Cast(shooterEntityRoot))
				targetEntity = shooterEntityRoot; // Root of vehicle
			else
				targetEntity = instigatorEntity; // Character in turret
		}
		
		// Bail if can't resolve the target entity for group perception
		if (!targetEntity)
			return;
		
		float timestamp = pm.GetTime();
		groupUtilityComp.m_Perception.AddOrUpdateGunshot(targetEntity, posWorld, faction, timestamp, endangering);
	}
	
	bool IsFlyby(vector myPos, vector shotPos, vector shotDir, float distance)
	{
		return Math3D.IntersectionPointCylinder(myPos, shotPos, shotDir, PROJECTILE_FLYBY_RADIUS);
	}
};
