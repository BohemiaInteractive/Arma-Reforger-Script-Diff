class SCR_AIGetAimCompensation : AITaskScripted
{	
	protected static float FIXED_AIM_POINT_LEAD_TIME_FACTOR = 3.5; // How much of predicted projectile flight time we want to lead for fixed aim points (leadTime = predictionTime * FIXED_AIM_POINT_LEAD_TIME_FACTOR)
	protected static float FIXED_AIM_POINT_TIMEOUT_FACTOR = 0.5; // After what proportion of lead time we want to timeout aim point (and start closing it to target, timeoutTime = leadTime * FIXED_AIM_POINT_TIMEOUT_FACTOR)
	protected static float FIXED_AIM_POINT_TIMEOUT_LERP_FACTOR = 0.04; // How much fixed aim point should lerp between current pos and target pos during each evaluation after fixed aim point is timeouted

	protected static float FIXED_AIM_POINT_MIN_SPEED_MS = 6; //  Minimal relative speed of target in meters/second for fixed aim point aiming
	protected static float FIXED_AIM_POINT_MIN_DIST_M = 15; // Minimal distance to target in meters for fixed aim point aiming
	
	protected static float CQB_COMPENSATION_MAX_DIST_M = 30; // Max distance in meters to use CQB compensation
	protected static float CQB_COMPENSATION_MAX_MULTIPLIER = 0.4; // Max multiplier used to compensate CQB (linear distance scale, from 1 to CQB_COMPENSATION_MAX_DIST_M)
	
	protected static float SURFACE_CORRECTION_MAX_ELEVATION_M = 2.5; // Max elevation of target (ATL) to correct for surface in meters
	
	// Minimal distance at which grenade launchers should aim at surface instead of target
	// NOTE: This is used/applied only if target is below SURFACE_CORRECTION_MAX_ELEVATION_M
	protected static float GL_SURFACE_AIMING_MIN_DIST_M = 30;
	
	protected static float SOLUTION_UPDATE_INTERVAL_MS = 750; // Default interval of solution updates in milliseconds
	
	// Input
	protected static string TARGET_ENTITY_PORT = "TargetEntity";
	protected static string TARGET_POSITION_PORT = "TargetPosition";
	protected static string AIMPOINT_PORT = "AimPoint";
	protected static string VECTOR_IN_PORT = "VectorIn";
	protected static string INITIAL_SPEED_COEFFICIENT = "InitialSpeedCoefficient";
	
	// Output
	protected static string VECTOR_OUT_PORT = "VectorOut";
	
	// Entities
	protected ChimeraCharacter m_ShooterCharacter;
	
	// Components
	protected SCR_AIUtilityComponent m_UtilityComponent;
	protected CharacterControllerComponent m_CharacterControllerComponent;
	protected BaseWeaponManagerComponent m_WeaponManagerComponent;
	
	// Logic data
	protected vector m_vPointLeadAimPos; // World pos of fixed lead aim point, v.Zero means we're following target constantly
	protected float m_fPointLeadAimPosTimeout; // World time after which lead point will be timed out (starts closing to target)
	protected float m_fPointLeadAimTargetDist; // Last measured distance from lead aim point to target, used to check validity of current aim point
	protected float m_fRandomFactor; // Random factor
	protected float m_fNextUpdate; // World time of next solution update
	
	protected IEntity m_TargetEntity; // Last passed target entity
	protected EAIUnitType m_RelevantTargetEntityType; // Perceived type of relevant target entity, used in surface checks
	protected IEntity m_RelevantTargetEntity; // Target or vehicle he's inside
	protected IEntity m_RelevantShooterEntity; // Shooter or vehicle he's inside
	protected EWeaponType m_eCurrentWeaponType; // Type of currently used weapon
		
	// Node attributes
	[Attribute("1", UIWidgets.CheckBox, "Compensate for distance")]
	protected bool m_bRangeCompensation;
	
	[Attribute("1", UIWidgets.CheckBox, "Compensate for velocity")]
	protected bool m_bVelocityCompensation;
	
#ifdef WORKBENCH
	//Diagnostic visualization
	ref array<ref Shape> m_aDbgShapes = {};
#endif		
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{	
		m_ShooterCharacter = ChimeraCharacter.Cast(owner.GetControlledEntity());
		if (!m_ShooterCharacter)
			return;
		
		m_UtilityComponent = SCR_AIUtilityComponent.Cast(owner.FindComponent(SCR_AIUtilityComponent));
		m_CharacterControllerComponent = CharacterControllerComponent.Cast(m_ShooterCharacter.FindComponent(CharacterControllerComponent));
		m_WeaponManagerComponent = BaseWeaponManagerComponent.Cast(m_ShooterCharacter.FindComponent(BaseWeaponManagerComponent));
	}
		
	//------------------------------------------------------------------------------------------------
	vector GetEntityVelocity(IEntity entity)
	{
		if (entity)
		{
			Physics physics = entity.GetPhysics();
			if (physics)
				return physics.GetVelocity();
		}

		return vector.Zero;
	}
		
	//------------------------------------------------------------------------------------------------
	float GetMuzzleAimData(BaseMuzzleComponent currentMuzzle, vector targetPos, vector targetVelocity, int iterations, out float targetDistance, out float predictionTime, out vector muzzlePos, out vector muzzleDir)
	{
		vector muzzleMat[4];
		m_WeaponManagerComponent.GetCurrentMuzzleTransform(muzzleMat);
		muzzlePos = muzzleMat[3];
		muzzleDir = muzzleMat[2];
		
		float rangeOffset;
		vector futureTargetPos = targetPos;
		targetDistance = vector.Distance(muzzlePos, futureTargetPos);
			
		// Add random error in initial distance measurement
		targetDistance += targetDistance * m_fRandomFactor;
		
		// Refine measurements
		for (int i = 0; i < iterations; i++)
		{
			rangeOffset = BallisticTable.GetAimHeightOfNextProjectile(targetDistance, predictionTime, currentMuzzle);
			futureTargetPos = targetPos + (targetVelocity * predictionTime);
			targetDistance = vector.Distance(muzzlePos, futureTargetPos);
		}
	
		return rangeOffset;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetEntityAimData(IEntity entity, float initSpeedCoef, vector targetPos, vector targetVelocity, int iterations, out float targetDistance, out float predictionTime, out vector muzzlePos, out vector muzzleDir)
	{
		vector shooterMat[3];
		entity.GetTransform(shooterMat);
		muzzlePos = entity.GetOrigin();
		muzzleDir = shooterMat[2];

		float rangeOffset;
		vector futureTargetPos = targetPos;
		targetDistance = vector.Distance(muzzlePos, futureTargetPos);
		
		// Add random error in initial distance measurement
		targetDistance += targetDistance * m_fRandomFactor;
		
		// Refine measurements
		for (int i = 0; i < iterations; i++)
		{
			rangeOffset = BallisticTable.GetHeightFromProjectile(targetDistance, predictionTime, entity, initSpeedCoef);
			futureTargetPos = targetPos + (targetVelocity * predictionTime);
			targetDistance = vector.Distance(muzzlePos, futureTargetPos);
		}
	
		return rangeOffset;
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns vehicle if given entity is a character inside a vehicle, otherwise returns given entity
	IEntity GetRelevantEntity(IEntity entity)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (!character || !character.IsInVehicle())
			return entity;
		
		CompartmentAccessComponent compAccComp = character.GetCompartmentAccessComponent();
		if (!compAccComp)
			return entity;
		
		IEntity vehicle = compAccComp.GetVehicleIn(character);
		if (!vehicle)
			return entity;
		
		return vehicle;
	}
		
	//------------------------------------------------------------------------------------------------
	void CorrectForSurface(out vector aimCorrectionVector, vector targetPos, vector aimPos, float speedMs, float targetDistance)
	{
		// No surface correction if target is flying
		if (m_RelevantTargetEntityType == EAIUnitType.UnitType_Aircraft)
			return;
		
		BaseWorld world = GetGame().GetWorld();
		float targetElev = targetPos[1] - world.GetSurfaceY(targetPos[0], targetPos[2]);
		
		// No surface correction if target is too high over surface
		if (targetElev > SURFACE_CORRECTION_MAX_ELEVATION_M)
			return;
		
		float aimPosSurfY = world.GetSurfaceY(aimPos[0], aimPos[2]);
		float aimPosElev = aimPos[1] - aimPosSurfY;
		
		// Aim at the surface if GL and outside of CQB range
		if (m_eCurrentWeaponType == EWeaponType.WT_GRENADELAUNCHER && targetDistance > GL_SURFACE_AIMING_MIN_DIST_M)
			aimCorrectionVector[1] = aimCorrectionVector[1] - (aimPosElev * 0.85);
		else
		{
			float surfYDiff = targetElev - aimPosElev;
			
			// Confidence in Y prediction, faster the target is moving, less confident we are in predicting elevation in relation to surface
			float confidence = Math.Clamp(Math.Map(speedMs, 3, 32, 1, 0.7), 0.2, 1);
			surfYDiff *= confidence;
				
			// Prevent going into underworld with predicted position
			float predSurfYDiff = (aimPos[1] + aimCorrectionVector[1] + surfYDiff) - aimPosSurfY;
			if (predSurfYDiff < 0.1)
				surfYDiff += Math.AbsFloat(predSurfYDiff - 0.1);
						
			aimCorrectionVector[1] = aimCorrectionVector[1] + surfYDiff;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Creates new fixed aim point (will be assigned to member variable)
	void CreateFixedAimPoint(BaseWeaponComponent currentWeapon, BaseMuzzleComponent currentMuzzle, vector targetPos, vector targetVelocity, float initSpeedCoef, float time)
	{		
		float targetDistance, predictionTime, rangeOffset;
		vector muzzlePos, muzzleDir;
				
		if (currentWeapon && currentMuzzle)
			rangeOffset = GetMuzzleAimData(currentMuzzle, targetPos, targetVelocity, 2, targetDistance, predictionTime, muzzlePos, muzzleDir);
		else if (currentWeapon)
			rangeOffset = GetEntityAimData(m_ShooterCharacter, initSpeedCoef, targetPos, targetVelocity, 1, targetDistance, predictionTime, muzzlePos, muzzleDir);
				
		float leadTime = predictionTime * FIXED_AIM_POINT_LEAD_TIME_FACTOR;
				
		leadTime += predictionTime * m_fRandomFactor;
		
		m_vPointLeadAimPos = targetPos + targetVelocity * leadTime;
		m_fPointLeadAimPosTimeout = time + (leadTime * FIXED_AIM_POINT_TIMEOUT_FACTOR * 1000);
		m_fNextUpdate = time + ((leadTime - predictionTime) * 1000);
		
		// Measure distance to compare later
		m_fPointLeadAimTargetDist = vector.Distance(targetPos, m_vPointLeadAimPos);
				
		// Adjust position for surface
		vector surfaceCorrection;
		CorrectForSurface(surfaceCorrection, targetPos, m_vPointLeadAimPos, targetVelocity.Length(), m_fPointLeadAimTargetDist);
		m_vPointLeadAimPos[1] = m_vPointLeadAimPos[1] + surfaceCorrection[1];
	}
	
	//------------------------------------------------------------------------------------------------
	// Validates fixed aim point - checks if should be discarded or timeouted
	void ValidateFixedAimPoint(out vector fixedAimPoint, vector targetPos, float time)
	{
		if (fixedAimPoint == vector.Zero)
			return;
		
		float aimPointTargetDist = vector.Distance(targetPos, fixedAimPoint);
			
		// Discard fixed aim point if it's moving away from target
		if (aimPointTargetDist > m_fPointLeadAimTargetDist)
			fixedAimPoint = vector.Zero;
		else
		{
			m_fPointLeadAimTargetDist = aimPointTargetDist;
			
			// If waypoint timeouted, start moving it towards target
			if (time > m_fPointLeadAimPosTimeout)
				fixedAimPoint -= (fixedAimPoint - targetPos) * FIXED_AIM_POINT_TIMEOUT_LERP_FACTOR;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Returns EMPIRICAL factors for aim correction based on angle and distance
	void GetDynamicAimCorrectionFactors(out float angleFactor, out float distanceFactor)
	{
		if (m_eCurrentWeaponType == EWeaponType.WT_ROCKETLAUNCHER)
			angleFactor = 2.32;
		else if (m_eCurrentWeaponType == EWeaponType.WT_GRENADELAUNCHER)
			angleFactor = 0.82;
		else
			angleFactor = 6.48;
		
		distanceFactor = 0.0006; // Average distance error per 1m
	}
	
	//------------------------------------------------------------------------------------------------
	// Calculates aim correction vector for aiming "disability" during dynamic aiming (following target constantly)
	vector GetDynamicAimCorrectionVector(vector targetPos, vector aimVector, vector muzzlePos, vector targetVelocity, float targetDistance)
	{			
		// Adjust for angle difference between target and aim position
		vector aimPos = targetPos + aimVector;
		vector dirToAimPoint = vector.Direction(muzzlePos, aimPos).Normalized();
		vector dirToTarget = vector.Direction(muzzlePos, targetPos).Normalized();	
		
		float angleFactor, distanceFactor;
		GetDynamicAimCorrectionFactors(angleFactor, distanceFactor);
		
		float angle = Math.Acos(vector.Dot(dirToTarget, dirToAimPoint));				
		float angleTime = angle * angleFactor; 
		float distanceTime = targetDistance * distanceFactor; 
			
		// CQB aiming compensation - Temporarly disabled due to changes in aim speed
		// if (targetDistance < CQB_COMPENSATION_MAX_DIST_M && targetDistance > 1)
		//	angleTime += angleTime * CQB_COMPENSATION_MAX_MULTIPLIER * ((CQB_COMPENSATION_MAX_DIST_M - targetDistance) / CQB_COMPENSATION_MAX_DIST_M);
			
		// Adjust random factor
		float correctionTime = angleTime + distanceTime;		
		correctionTime += correctionTime * m_fRandomFactor;
		
		vector aimCorrectionVector = targetVelocity * correctionTime;
			
		// Adjust for surface differences
		CorrectForSurface(aimCorrectionVector, targetPos, aimPos, targetVelocity.Length(), targetDistance);	
		
		return aimCorrectionVector;
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_CharacterControllerComponent || !m_UtilityComponent || !m_ShooterCharacter)
			return ENodeResult.FAIL;
		
		float time = GetGame().GetWorld().GetWorldTime();
		
		// Input
		vector aimVector;
		float initSpeedCoef = 1.0;
		GetVariableIn(VECTOR_IN_PORT, aimVector);
		GetVariableIn(INITIAL_SPEED_COEFFICIENT, initSpeedCoef);
				
		vector targetPos;
		
		// Entity
		IEntity targetEntity;
		GetVariableIn(TARGET_ENTITY_PORT, targetEntity);
		if (!targetEntity)
		{
			Print("GetAimCompensation: provided null entity", LogLevel.WARNING);
			return ENodeResult.FAIL;
		}
		
		// Aimpoint
		AimPoint aimPoint;
		if (GetVariableIn(AIMPOINT_PORT, aimPoint))
		{
			if (!aimPoint)
			{
				Print("GetAimCompensation: provided null aimpoint", LogLevel.WARNING);
				return ENodeResult.FAIL;
			}
			
			targetPos = aimPoint.GetPosition();
		}
		// Fallback to connected pos var or entity position
		else
		{
			// Get position of center of the model
			if (!GetVariableIn(TARGET_POSITION_PORT, targetPos))
				targetPos = targetEntity.GetOrigin();
		}
		
		//-------------------------------------------------------------------------------------- IN
				
		// Check if we need to update aim solution
		bool updateSolution = m_TargetEntity != targetEntity || !m_RelevantTargetEntity || time > m_fNextUpdate;
		
		// Update entities
		if (updateSolution)
		{
			m_fRandomFactor = Math.RandomGaussFloat(0.045, 0.065);
			m_TargetEntity = targetEntity;
			m_RelevantTargetEntity = GetRelevantEntity(m_TargetEntity);
			m_RelevantShooterEntity = GetRelevantEntity(m_ShooterCharacter);
			
			if (m_UtilityComponent.m_CombatComponent)
				m_WeaponManagerComponent = m_UtilityComponent.m_CombatComponent.GetCurrentWeaponManager();
			
			// Fail and reset update time update time so we can check again in next node run
			if (!m_WeaponManagerComponent)
			{
				m_fNextUpdate = 0;
				return ENodeResult.FAIL;
			}	
			
			// Get target entity unit type
			m_RelevantTargetEntityType = EAIUnitType.UnitType_Infantry;
			PerceivableComponent perceivable = PerceivableComponent.Cast(m_RelevantTargetEntity.FindComponent(PerceivableComponent));
			if (perceivable)
				m_RelevantTargetEntityType = perceivable.GetUnitType();
			
			// Schedule next update (note: fixed aim point selection can override update time)
			m_fNextUpdate = time + SOLUTION_UPDATE_INTERVAL_MS;
		}
		
		// Resolve current weapon and muzzle
		BaseWeaponComponent currentWeapon = m_WeaponManagerComponent.GetCurrentWeapon();
		BaseMuzzleComponent currentMuzzle;
		if (currentWeapon)
			currentMuzzle = currentWeapon.GetCurrentMuzzle();
		
		// Velocities
		vector targetVelocity = GetEntityVelocity(m_RelevantTargetEntity);
		vector shooterVelocity = GetEntityVelocity(m_RelevantShooterEntity);

		// Aim data
		vector muzzlePos, muzzleDir;
		float targetDistance, rangeOffset, predictionTime;
		float relativeAbsSpeed = Math.AbsFloat((targetVelocity - shooterVelocity).Length()); // Relative abs speed in m/s
		
		// Check if fixed aim point should be used
		if (updateSolution)
		{			
			m_vPointLeadAimPos = vector.Zero;

			// Update weapon type
			m_eCurrentWeaponType = EWeaponType.WT_RIFLE;
			if (currentWeapon)
				m_eCurrentWeaponType = SCR_AIWeaponHandling.GetWeaponType(currentWeapon, true);
						
			// Simplified distance just to check if we should use fixed aim point
			targetDistance = vector.Distance(targetPos, m_RelevantShooterEntity.GetOrigin());
			
			// Fixed aim point is used for MGs shooting at fast-moving, distant targets
			/*
			// For now it's disabled because the 0.5*dt compensation shows better results
			if (m_eCurrentWeaponType == EWeaponType.WT_MACHINEGUN && relativeAbsSpeed > FIXED_AIM_POINT_MIN_SPEED_MS && 
				targetDistance > FIXED_AIM_POINT_MIN_DIST_M)
				CreateFixedAimPoint(currentWeapon, currentMuzzle, targetPos, targetVelocity, initSpeedCoef, time);
			*/
		}
		
		// Check validity of current fixed aim pos
		if (m_vPointLeadAimPos != vector.Zero)
			ValidateFixedAimPoint(m_vPointLeadAimPos, targetPos, time);
				
		// Determine how accurate we want to be in aiming / how many times we will calculate correction for aim prediction
		int iterations = 2;
		if (!m_bVelocityCompensation || relativeAbsSpeed < 2)
			iterations = 1;
		
		// Get aim data
		if (currentWeapon && currentMuzzle)
			rangeOffset = GetMuzzleAimData(currentMuzzle, targetPos, targetVelocity, iterations, targetDistance, predictionTime, muzzlePos, muzzleDir);
		else if (currentWeapon)
			rangeOffset = GetEntityAimData(m_ShooterCharacter, initSpeedCoef, targetPos, targetVelocity, iterations, targetDistance, predictionTime, muzzlePos, muzzleDir);
				
		// Threat based compensation error
		if (m_UtilityComponent.m_ThreatSystem)
		{
			float suppression = m_UtilityComponent.m_ThreatSystem.GetSuppressionMeasure();	
			if (suppression > 0.01)
				predictionTime += predictionTime * (suppression - Math.RandomFloat(0, suppression * 2));
		}
		
		vector aimCorrectionVector;
		if (m_bVelocityCompensation)
		{
			// Adjust for shooter velocity
			aimVector -= (shooterVelocity * predictionTime);
			
			// Dynamic aiming
			if (m_vPointLeadAimPos == vector.Zero)
			{
				// Adjust for target velocity
				// 0.5*dt is here because AI aiming samples target position at AI update interval (dt).
				// therefore data is delayed by 0.5*dt on average.
				aimVector += (targetVelocity * (predictionTime + 0.5*dt));
			}
			// Fixed aim point
			else
				aimVector += m_vPointLeadAimPos - targetPos;
		}
		
		// Apply range compensation
		if (m_bRangeCompensation)
			aimVector[1] = aimVector[1] + rangeOffset;
		
		//-------------------------------------------------------------------------------------- OUT		
#ifdef WORKBENCH
		m_aDbgShapes.Clear();
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_AIM_LEAD_DEBUG))
		{
			m_aDbgShapes.Insert(Shape.CreateSphere(Color.DARK_GREEN, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE, targetPos, 0.12));
			m_aDbgShapes.Insert(Shape.CreateSphere(Color.RED, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE, targetPos + aimVector, 0.12));
			m_aDbgShapes.Insert(Shape.CreateSphere(Color.CYAN, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE, targetPos + aimVector + aimCorrectionVector, 0.12));
			
			if (m_vPointLeadAimPos != vector.Zero)
				m_aDbgShapes.Insert(Shape.CreateSphere(Color.VIOLET, ShapeFlags.TRANSP | ShapeFlags.NOOUTLINE, m_vPointLeadAimPos, 0.12));
		}
#endif
		SetVariableOut(VECTOR_OUT_PORT, aimVector + aimCorrectionVector);
		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {VECTOR_OUT_PORT};
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {TARGET_ENTITY_PORT, TARGET_POSITION_PORT, AIMPOINT_PORT, VECTOR_IN_PORT, INITIAL_SPEED_COEFFICIENT};
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	//------------------------------------------------------------------------------------------------
	override static bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected override static string GetOnHoverDescription() {return "Returns offset for compensation of target range and velocity. Offset will be added to VectorIn, so it can be used for build-in imprecision.";}
}