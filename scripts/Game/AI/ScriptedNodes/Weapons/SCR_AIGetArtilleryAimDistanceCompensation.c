class SCR_AIGetArtilleryAimDistanceCompensation : AITaskScripted
{
	// Inputs
	protected static const string PORT_ARTILLERY_ENTITY = "ArtilleryEntity";
	protected static const string PORT_AMMO_PREFAB = "AmmoPrefab";
	protected static const string PORT_TARGET_POS = "TargetPos";
	
	// Outputs
	protected static const string PORT_AIM_POS = "AimPos";
	protected static const string PORT_INIT_SPEED_ID = "InitSpeedId";
	
	//---------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		// Read inputs
		IEntity artilleryEntity;
		ResourceName ammoPrefab;
		vector targetPos;
		GetVariableIn(PORT_ARTILLERY_ENTITY, artilleryEntity);
		GetVariableIn(PORT_AMMO_PREFAB, ammoPrefab);
		GetVariableIn(PORT_TARGET_POS, targetPos);
		
		if (!artilleryEntity || ammoPrefab.IsEmpty() || targetPos == vector.Zero)
			return ENodeResult.FAIL;
		
		// Get aim limits of artillery
		float aimAngleMinRad, aimAngleMaxRad;
		GetVerticalAimLimits(artilleryEntity, aimAngleMinRad, aimAngleMaxRad);
		
		// Get initial speed coefficients of ammo
		array<float> initSpeedCoefficients = {};
		bool ammoSupportsSpeedConfigurations = GetAmmoInitialSpeedCoefficients(ammoPrefab, initSpeedCoefficients);
		
		// If different ammo configurations aren't supported, ensure the array is not empty. Use 1.0 initial speed.
		if (initSpeedCoefficients.IsEmpty())
			initSpeedCoefficients.Insert(1.0);
		
		// Load Entity Source
		Resource ammoResource = Resource.Load(ammoPrefab);
		if (!ammoResource.IsValid())
			return ENodeResult.FAIL;

		BaseResourceObject ammoResourceObj = ammoResource.GetResource();
		if (!ammoResourceObj)
			return ENodeResult.FAIL;

		IEntitySource ammoEntitySrc = ammoResourceObj.ToEntitySource();
		if (!ammoEntitySrc)
			return ENodeResult.FAIL;
		
		// Calculate compensation
		
		vector artilleryPos = artilleryEntity.GetOrigin();
		float distToTgtHoriz = vector.DistanceXZ(artilleryPos, targetPos);
		float distToTgtVert = targetPos[1] - artilleryPos[1];
		
		float aimOffsetVert;
		int initSpeedId;
		bool canHitTarget = CalculateDistanceCompensation(distToTgtHoriz, distToTgtVert, ammoEntitySrc, initSpeedCoefficients, aimAngleMinRad, aimAngleMaxRad, aimOffsetVert, initSpeedId);
		
		// If ammo doesn't support speed configurations, initSpeedId is meaningless, set it to -1 so that the BT knows to not try to configure it
		if (!ammoSupportsSpeedConfigurations)
			initSpeedId = -1;
		
		if (!canHitTarget)
			return ENodeResult.FAIL;
		
		vector aimPos = targetPos;
		aimPos[1] = aimPos[1] + aimOffsetVert;
		SetVariableOut(PORT_AIM_POS, aimPos);
		
		SetVariableOut(PORT_INIT_SPEED_ID, initSpeedId);
		
		return ENodeResult.SUCCESS;
	}
	
	//---------------------------------------------------------------------------------
	bool CalculateDistanceCompensation(float distToTgtHoriz, float distToTgtVert, notnull IEntitySource entitySrc, notnull array<float> initSpeedCoeffs, float aimAngleMinRad, float aimAngleMaxRad, out float outAimOffsetVert, out int outInitSpeedId)
	{
		// Bail if distance is 0, this makes no sense, and might cause exception
		if (distToTgtHoriz <= 0)
		{
			outAimOffsetVert = 0;
			outInitSpeedId = -1;
			return false;
		}
		
		// If angle is bigger then tangent is bigger. Tangents are easier to compute than angles in the loop below.
		float aimAngleTanMin = Math.Tan(aimAngleMinRad);
		float aimAngleTanMax = Math.Tan(aimAngleMaxRad);
		
		// Try all variants of initial speed coefficients, until we find the one which can both hit target and is within turret's aiming limits
		bool canHitTarget = false;
		float aimOffsetVert = 0;
		int initSpeedId = -1;
		
		// First try all variants with the proper ballistics algorithm, with vertical interval compensation
		for (int i = 0; i < initSpeedCoeffs.Count(); i++)
		{
			float initialSpeedCoefficient = initSpeedCoeffs[i];
			
			float flightTime;
			bool ballisticsGood = BallisticTable.GetAimHeightOfProjectileAltitudeFromSource(distToTgtHoriz, aimOffsetVert, flightTime, entitySrc, distToTgtVert, initialSpeedCoefficient);
			
			// Can't hit it - try next one
			if (!ballisticsGood)
				continue;
			
			// Can hit it, but is aim angle within the aim limits?
			float aimAngleTan = (aimOffsetVert + distToTgtVert) / distToTgtHoriz;
			if (aimAngleTan > aimAngleTanMax || aimAngleTan < aimAngleTanMin)
				continue;
			
			canHitTarget = true;
			initSpeedId = i;
			break;
		}
		
		// If we still can't hit target with proper ballistics calculation, try the basic ballistic algorithm, without vertical interval compensation
		if (!canHitTarget)
		{			
			for (int i = 0; i < initSpeedCoeffs.Count(); i++)
			{
				float initialSpeedCoefficient = initSpeedCoeffs[i];
				
				float flightTime;
				aimOffsetVert = BallisticTable.GetHeightFromProjectileSource(distToTgtHoriz, flightTime, entitySrc, initSpeedCoef: initialSpeedCoefficient, bDirectFire: false);
				bool ballisticsGood = flightTime > 0;
				
				// Can't hit it - try next one
				if (!ballisticsGood)
					continue;
				
				// Can hit it, but is aim angle within the aim limits?
				float aimAngleTan = (aimOffsetVert + distToTgtVert) / distToTgtHoriz;
				if (aimAngleTan > aimAngleTanMax || aimAngleTan < aimAngleTanMin)
					continue;
				
				canHitTarget = true;
				initSpeedId = i;
				break;
			}
		}

		outAimOffsetVert = aimOffsetVert;
		outInitSpeedId = initSpeedId;
		
		return canHitTarget;
	}
	
	//---------------------------------------------------------------------------------
	void GetVerticalAimLimits(notnull IEntity artilleryEntity, out float outAngleMinRad, out float outAngleMaxRad)
	{
		float angleMinRad = 0;
		float angleMaxRad = Math.PI_HALF;
		
		AimingComponent aimingComp = AimingComponent.Cast(artilleryEntity.FindComponent(AimingComponent));
		
		if (aimingComp)
		{
			vector limitsHoriz, limitsVert;
			aimingComp.GetAimingLimits(limitsHoriz, limitsVert);
			
			angleMinRad = Math.DEG2RAD * limitsVert[0];
			angleMaxRad = Math.DEG2RAD * limitsVert[1];
		}
		
		outAngleMinRad = angleMinRad;
		outAngleMaxRad = angleMaxRad;
	}
	
	//---------------------------------------------------------------------------------
	// Returns true if ammo supports initial speed configurations
	bool GetAmmoInitialSpeedCoefficients(ResourceName ammoResourceName, notnull array<float> outCoefficients)
	{
		// This can be potentially interfaced with other types of ammo which support different configurations
		outCoefficients.Clear();
		
		int defaultConfigId;
		array<float> initSpeedCoefficients = SCR_MortarShellGadgetComponent.GetPrefabInitSpeedCoef(ammoResourceName, defaultConfigId);
		
		if (initSpeedCoefficients && !initSpeedCoefficients.IsEmpty())
		{
			outCoefficients.Copy(initSpeedCoefficients);
			return true;
		}
		
		return false;
	}
	
	//---------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = { PORT_AIM_POS, PORT_INIT_SPEED_ID };
	protected override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	protected static ref TStringArray s_aVarsIn = { PORT_ARTILLERY_ENTITY, PORT_AMMO_PREFAB, PORT_TARGET_POS };
	protected override TStringArray GetVariablesIn() { return s_aVarsIn; }
	
	override static bool VisibleInPalette() { return true; }
	
	override static string GetOnHoverDescription() { return "Performs ballistic calculations for artillery. It resolves where to aim and which initial speed to use."; }
}