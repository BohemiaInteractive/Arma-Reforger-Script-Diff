class SCR_AIGetAllowedLookRange : AITaskScripted
{
	protected static const string RANGE_PORT = "AngularRangeDeg";
	protected static const string LOOK_AXIS_PORT = "LookAxis";
	protected static const string IS_FREE_LOOK = "IsFreeLook";
	protected static const float FREE_LOOK_ANGLE = 30.0;	
	protected static const float MOVING_IN_VEHICLE_ANGLE = 15;

	// Used to test whether our vehicle is moving or not
	protected const float MIN_SPEED_MPS_SQ = 0.5 * 0.5;
		
	[Attribute("20", UIWidgets.EditBox, "Distance for looking" )]
	protected float m_fDistance;
	
	
	
	protected static ref TStringArray s_aVarsOut = { LOOK_AXIS_PORT, RANGE_PORT, IS_FREE_LOOK };
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(owner.GetControlledEntity());
		if (!character)
			return ENodeResult.FAIL;
		if (!character.IsInVehicle())
			return ENodeResult.FAIL;
		CompartmentAccessComponent compAcc = character.GetCompartmentAccessComponent();
		if (!compAcc)
			return ENodeResult.FAIL;
		BaseCompartmentSlot comp = compAcc.GetCompartment();
		if (!comp)
			return ENodeResult.FAIL;
		vector forwardVec;
		IEntity vehicle = comp.GetVehicle();
		forwardVec = vehicle.GetTransformAxis(2).Normalized();
		float angleBound;
		bool isFreeLook = true;
		
		bool isMoving = false;
		Physics phy = vehicle.GetPhysics();
		bool phyActive = phy.IsActive(); // delete
		vector phyVelo = phy.GetVelocity(); // delete
		if (phy && phy.IsActive())
			isMoving = phy.GetVelocity().LengthSq() > MIN_SPEED_MPS_SQ;
		
		TurretCompartmentSlot turret = TurretCompartmentSlot.Cast(comp);
		if (turret)
		{
			TurretControllerComponent turretController = TurretControllerComponent.Cast(turret.GetController());
			if (!turretController)
				return ENodeResult.FAIL;		
			TurretComponent turretComponent = turretController.GetTurretComponent();
			if (!turretComponent)
				return ENodeResult.FAIL;
			
			
			vector aimingLimitHoriz, aimingLimitVert;
			turretComponent.GetAimingLimits(aimingLimitHoriz,aimingLimitVert);
			
			if (turretComponent.HasMoveableBase())
				aimingLimitHoriz = Vector(-180, 180, 0);
			
			if (isMoving)
			{
				// When moving, look forward
				angleBound = Math.Min(aimingLimitHoriz[1], MOVING_IN_VEHICLE_ANGLE);
			}
			else
				angleBound = aimingLimitHoriz[1];
			
			isFreeLook = false;
		}	
		else
		{
			angleBound = FREE_LOOK_ANGLE;		
		}
		
		SetVariableOut(LOOK_AXIS_PORT, comp.GetPosition() + forwardVec * m_fDistance);
		SetVariableOut(RANGE_PORT, angleBound);
		SetVariableOut(IS_FREE_LOOK, isFreeLook);
				
		return ENodeResult.SUCCESS;
	}
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() { return true; }
	
	//----------------------------------------------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "GetAllowedLookRange: Gets the range of look for character in vehicle";
	}
};