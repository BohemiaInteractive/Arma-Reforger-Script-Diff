enum EAimingPreference
{
	NONE,
	RANDOM,
	AUTOMATIC,
	FIXED,
};

class SCR_AIGetAimErrorOffset: AITaskScripted
{
	static const string PORT_ERROR_OFFSET = "ErrorOffset";
	static const string PORT_BASE_TARGET = "BaseTargetIn";
	static const string PORT_AIM_POINT = "AimPoint";
	static const string PORT_TOLERANCE = "AimingTolerance";
	static const string PORT_AIMPOINT_TYPE_0 = "AimpointType0";	// Primary aimpoint type
	static const string PORT_AIMPOINT_TYPE_1 = "AimpointType1";	// Secondary aimpoint type
	static const float CLOSE_RANGE_THRESHOLD = 15.0;
	static const float LONG_RANGE_THRESHOLD = 200.0;
	static const float AIMING_ERROR_SCALE = 1.0; // TODO: game master and server option
	static const float AIMING_ERROR_FACTOR_MIN = 0.4; 
	static const float AIMING_ERROR_CLOSE_RANGE_FACTOR_MIN = 0.05;
	static const float AIMING_ERROR_FACTOR_MAX = 1.4;
	static const float MAXIMAL_TOLERANCE = 10.0;
	static const float MINIMAL_TOLERANCE = 0.003;
	
	protected SCR_AICombatComponent m_CombatComponent;
	// private SCR_AIInfoComponent m_InfoComponent;		temporary removed (adding threat effect later)
	
#ifdef AI_DEBUG
	protected ref array<ref Shape> m_aDebugShapes = {};
#endif
	[Attribute("2", UIWidgets.ComboBox, "Hit zone selection", "", ParamEnumArray.FromEnum(EAimingPreference) )]
	protected EAimingPreference m_eAimingPreference;
	
	[Attribute("0", UIWidgets.ComboBox, "AimPoint type for fixed option", "", ParamEnumArray.FromEnum(EAimPointType) )]
	protected EAimPointType m_eAimPointType;
	
	[Attribute("0.03", UIWidgets.EditBox, "Default PrecisionXY")]
	protected float m_fDefaultPrecisionXY;

	private int m_iTorsoCount = 0;	
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		IEntity ent = owner.GetControlledEntity();
		if (ent)
			m_CombatComponent = SCR_AICombatComponent.Cast(ent.FindComponent(SCR_AICombatComponent));		
		//m_InfoComponent = SCR_AIInfoComponent.Cast(owner.FindComponent(SCR_AIInfoComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_ERROR_OFFSET,
		PORT_AIM_POINT,
		PORT_TOLERANCE
	};
    override array<string> GetVariablesOut()
    {
        return s_aVarsOut;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_BASE_TARGET,
		PORT_AIMPOINT_TYPE_0,
		PORT_AIMPOINT_TYPE_1
	};
    override array<string> GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
    //------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		//if (!m_CombatComponent || !m_InfoComponent)
		if (!m_CombatComponent)
			return ENodeResult.FAIL;
		
		IEntity entity = owner.GetControlledEntity();
		if (!entity)
			return ENodeResult.FAIL;
		
#ifdef AI_DEBUG
		m_aDebugShapes.Clear();
		
#endif
		
		BaseTarget target;
		GetVariableIn(PORT_BASE_TARGET, target);
		
		// Bail if target is invalid
		if (!target)
		{
			ClearPorts();
			return ENodeResult.FAIL;
		}
		
		// Bail if target is invalid
		IEntity targetEntity = target.GetTargetEntity();
		if (!targetEntity)
		{
			ClearPorts();
			return ENodeResult.FAIL;
		}
		
		// Resolve which aimpoint types to use
		EAimPointType aimpointTypes[3];
		EAimPointType aimpointType0, aimpointType1;
		if (!GetVariableIn(PORT_AIMPOINT_TYPE_0, aimpointType0))
			aimpointType0 = -1;
		if (!GetVariableIn(PORT_AIMPOINT_TYPE_1, aimpointType1))
			aimpointType1 = -1;
		aimpointTypes[0] = aimpointType0;
		aimpointTypes[1] = aimpointType1;
		aimpointTypes[2] = m_eAimPointType;
		
		// Try to find aimpoint
		AimPoint aimPoint = GetAimPoint(target, aimpointTypes);
		
		// Bail if aimpoint was not found
		if (!aimPoint)
		{
			ClearPorts();
			return ENodeResult.FAIL;
		}
		
		EWeaponType weaponType = m_CombatComponent.GetCurrentWeaponType();

#ifdef AI_DEBUG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_TARGET_AIMPOINT))
			m_aDebugShapes.Insert(Shape.CreateSphere(COLOR_YELLOW_A, ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP, aimPoint.GetPosition(),aimPoint.GetDimension()));
#endif
		
		// Calculate angular bounds
		vector offsetX, offsetY;
		float angularSize, distance, tolerance;		
		GetTargetAngularBounds(entity, aimPoint, offsetX, offsetY, angularSize, distance);

		// Correct aim point size based on factors
		float distanceFactor = GetDistanceFactor(distance);
		float offsetWeaponFactor = GetOffsetWeaponTypeFactor(weaponType);				
		float illuminationFactor = GetTargetIlluminationFactor(target);
		
		EAISkill currentSkill = m_CombatComponent.GetAISkill();
		offsetX = GetRandomFactor(currentSkill, 0) * offsetX * AIMING_ERROR_SCALE * distanceFactor * offsetWeaponFactor * illuminationFactor;
		offsetY = GetRandomFactor(currentSkill, 0) * offsetY * AIMING_ERROR_SCALE * distanceFactor * offsetWeaponFactor * illuminationFactor;
		
		tolerance = GetTolerance(entity, targetEntity, angularSize, distance, weaponType);
		
		SetVariableOut(PORT_ERROR_OFFSET, offsetX + offsetY);
		SetVariableOut(PORT_AIM_POINT, aimPoint);
		SetVariableOut(PORT_TOLERANCE, tolerance);
		
#ifdef WORKBENCH
		// PrintFormat("Target size - used in tolerance: %1 target aimpointPosition: %2", distance * Math.Tan(tolerance * Math.DEG2RAD), aimPoint.GetPosition());
#endif
		
		return ENodeResult.SUCCESS;
	}
	
	void ClearPorts()
	{
		ClearVariable(PORT_ERROR_OFFSET);
		ClearVariable(PORT_AIM_POINT);
		ClearVariable(PORT_TOLERANCE);
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	//! returns vectors of aimpoint volume projected onto shooter's BB center and the angular size of the target aimpoint
	void GetTargetAngularBounds(IEntity shooter, AimPoint targetAimpoint, out vector sizeVectorX, out vector sizeVectorY, out float angularSize, out float distance)
	{
		
		vector shooterCenter, joinNorm, join, min, max;
		float dimension = 2 * targetAimpoint.GetDimension();
		shooter.GetWorldBounds(min,max);
		
		// we wanted to use muzzle transform but that is unrelyable (when weapon switches, on init of fight, and so on)
		shooterCenter = (min + max) * 0.5;
		join = targetAimpoint.GetPosition() - shooterCenter;
		distance = join.Length();
		angularSize = Math.Atan2(dimension, distance) * Math.RAD2DEG;
		
		joinNorm = join.Normalized();
		sizeVectorX = (vector.Up * joinNorm).Normalized();
		sizeVectorY = (joinNorm * sizeVectorX).Normalized();
		
		sizeVectorX *= dimension;
		sizeVectorY *= dimension;
		
#ifdef AI_DEBUG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_TARGET_PROJECTED_SIZE))
		{
			m_aDebugShapes.Insert(Shape.CreateArrow(shooterCenter, shooterCenter + sizeVectorX, 0.1, COLOR_BLUE, ShapeFlags.NOZBUFFER));
			m_aDebugShapes.Insert(Shape.CreateArrow(shooterCenter, shooterCenter + sizeVectorY, 0.1, COLOR_RED, ShapeFlags.NOZBUFFER));
			m_aDebugShapes.Insert(Shape.CreateArrow(shooterCenter, shooterCenter + joinNorm * dimension, 0.1, COLOR_YELLOW, ShapeFlags.NOZBUFFER));
		}	
#endif
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------------
	//! returns factor of error scale based on distance to target 
	float GetDistanceFactor(float distance)
	{
		if (distance < CLOSE_RANGE_THRESHOLD)
			return Math.Map(distance, 0, CLOSE_RANGE_THRESHOLD, AIMING_ERROR_CLOSE_RANGE_FACTOR_MIN, AIMING_ERROR_FACTOR_MIN);

		float distanceCl = Math.Clamp((distance - CLOSE_RANGE_THRESHOLD) / LONG_RANGE_THRESHOLD, 0, 1);
		return Math.Lerp(AIMING_ERROR_FACTOR_MIN, AIMING_ERROR_FACTOR_MAX, distanceCl);
	}
	
	//------------------------------------------------------------------------------------------------
	//! returns 1.0 if target is well illuminated, and a bigger value if target is poorly illuminated.
	float GetTargetIlluminationFactor(BaseTarget tgt)
	{
		PerceivableComponent perceivable = tgt.GetPerceivableComponent();
		if (!perceivable)
			return 1.0;
		
		if (perceivable.GetIlluminationFactor() < 0.5)
			return 2.0;
		
		return 1.0;
	}
	
	//------------------------------------------------------------------------------------------------
	// returns random factor based on AI skill
	float GetRandomFactor(EAISkill skill,float mu)
	{
		float sigma;
		switch (skill)
		{
			case EAISkill.NOOB :
			{
				sigma = 4;
				break;
			}
			case EAISkill.ROOKIE :
			{
				sigma = 2;
				break;
			}
			case EAISkill.REGULAR :
			{
				sigma = 1;
				break;
			}
			case EAISkill.VETERAN :
			{
				sigma = 0.5;
				break;
			}
			case EAISkill.EXPERT :
			{
				sigma = 0.25;
				break;
			}
			case EAISkill.CYLON :
			{
				return 0;
			}
		}
		// PrintFormat("Gauss: %1, sigma: %2, skill: %3",result,sigma,typename.EnumToString(EAISkill,skill));
		return Math.RandomGaussFloat(sigma,mu);
	}
	
	//------------------------------------------------------------------------------------------------
	//! basic tolerance based on angular size of target in degrees
	float GetTolerance(IEntity observer, IEntity target, float angularSize, float distance, EWeaponType weaponType)
	{
		float tolerance;
		bool setMaxTolerance;
	
		// Always use max tolerance in close range
		if (distance < CLOSE_RANGE_THRESHOLD)
			return MAXIMAL_TOLERANCE;
			
		tolerance = angularSize / 2; // half of the size
		// angular speed
		tolerance *= GetAngularSpeedFactor(observer, target, setMaxTolerance);
				
		if (setMaxTolerance)
			tolerance = MAXIMAL_TOLERANCE;
		else 
		{
			// weapon type tolerance modifier
			tolerance *= GetWeaponTypeFactor(weaponType);
		};
		return Math.Clamp(tolerance, MINIMAL_TOLERANCE, MAXIMAL_TOLERANCE);
	}	
	
	//------------------------------------------------------------------------------------------------	
	float GetAngularSpeedFactor(IEntity observer, IEntity enemy, out bool setBigTolerance)
	{
		vector enemyVelocity;
		IEntity enemyRoot = enemy.GetRootParent();
		Physics enemyPhysics = enemyRoot.GetPhysics();
		if (enemyPhysics)
			enemyVelocity = enemyPhysics.GetVelocity();
		
		vector observerVelocity;
		vector observerAngularVelocity;
		IEntity observerRoot = observer.GetRootParent();
		Physics observerPhysics = observerRoot.GetPhysics();
		if (observerPhysics)
		{
			observerVelocity = observerPhysics.GetVelocity();
			observerAngularVelocity = observerPhysics.GetAngularVelocity();
		}
		
		vector relativeVelocity = enemyVelocity - observerVelocity;
		
		vector positionVector = enemy.GetOrigin() - observer.GetOrigin();
		vector targetLocalAngularVelocity = observerAngularVelocity + (positionVector * relativeVelocity / positionVector.LengthSq());  // omega = (r x v) / ||r||^2 
		float totalTargetLocalAngularVelocity = targetLocalAngularVelocity.Length();			
		
		if (totalTargetLocalAngularVelocity < 0.07) // rougly 4 degs in radians
			return 1.0;
		else if (totalTargetLocalAngularVelocity < 0.17) // roughly 10 degs in radians
			return 2;
	
		setBigTolerance = true;
		return 0;
	}

	//------------------------------------------------------------------------------------------------		
	float GetWeaponTypeFactor(EWeaponType weaponType)
	{
		switch(weaponType)
		{
			case EWeaponType.WT_RIFLE:
			{
				return 1.0;
			}
			case EWeaponType.WT_MACHINEGUN:
			{
				return 2.0;
			}
			case EWeaponType.WT_HANDGUN:
			{
				return 1.5;
			}
			case EWeaponType.WT_FRAGGRENADE:
			{
				return 3.0;
			}
			case EWeaponType.WT_SMOKEGRENADE:
			{
				return 4.0;
			}
			case EWeaponType.WT_ROCKETLAUNCHER:
			{
				return 0.5;
			}
			case EWeaponType.WT_SNIPERRIFLE:
			{
				return 0.5;
			}
		}
		return 1.0;
	}
	
	//------------------------------------------------------------------------------------------------		
	//! Scales offset depending on weapon type
	float GetOffsetWeaponTypeFactor(EWeaponType weaponType)
	{
		if (weaponType == EWeaponType.WT_ROCKETLAUNCHER)
			return 0;
		else
			return 1.0;
	}
		
	//------------------------------------------------------------------------------------------------
	AimPoint GetAimPoint(BaseTarget target, EAimPointType aimpointTypes[3])
	{
		PerceivableComponent targetPerceivable = target.GetPerceivableComponent();
		if (!targetPerceivable)
			return null;
		
		array<ref AimPoint> aimPoints = {};
		
		// Try to find an aimpoint
		for (int i = 0; i < 3; i++)
		{
			EAimPointType aimpointType = aimpointTypes[i];
			if (aimpointType == -1)
				continue;
				
			targetPerceivable.GetAimpointsOfType(aimPoints, aimpointType);
			if (!aimPoints.IsEmpty())
				break;
		}
		
		int index = aimPoints.GetRandomIndex(); // more aimpoints of same type -> pick one at random
		
		if (index != -1)
			return aimPoints[index];
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------
    static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription() {return "Get aiming error position from the center of the target";}
};

