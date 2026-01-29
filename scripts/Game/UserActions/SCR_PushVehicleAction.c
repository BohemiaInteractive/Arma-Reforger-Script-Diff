class SCR_PushVehicleAction : SCR_ScriptedUserAction
{
	protected const float FORCE_SCALE						= 1.0;
	protected const float COUNTERFORCE_SCALE				= -1.5;

	[Attribute(defvalue: "1", desc: "Mass to force\n[N per kg]")]
	protected float m_fMassToForce;

	[Attribute(defvalue: "1000", desc: "Force limit\n[N]", params: "1 100000 1")]
	protected float m_fForceLimit;

	[Attribute(defvalue: "3", desc: "Maximum speed\n[m/s]")]
	protected float m_fLinearSpeedLimit;

	[Attribute(defvalue: "2", desc: "Maximum angular speed\n[rad/s]", params: "0 inf")]
	protected float m_fAngularSpeedLimit;

	[Attribute(defvalue: "0", desc: "Pull if enabled, otherwise push", params: "0 inf")]
	protected bool m_bPull;

	[Attribute(defvalue: "5", desc: "How many times per second server should try to apply this force to the vehicle", params: "1 30 1")]
	protected int m_iForceApplicationFrequency;

	[Attribute(defvalue: "1.5", desc: "Multiplies the offset used for the position of the counter force.\nThis makes it easier to apply rotational force without pushing away the object from the user.", params: "0.1 inf")]
	protected float m_fCounterForceOffsetMultiplier;

	protected float m_fApplicationInterval;
	protected float m_fApplicationDeltaTime;
	protected RplComponent m_Rpl;
#ifdef ENABLE_DIAG
	protected ref array<ref Shape> m_aShapes = {};

	//------------------------------------------------------------------------------------------------
	void ClearDebug()
	{
		m_aShapes.Clear();
	}
#endif

	//------------------------------------------------------------------------------------------------
	//! Called when object is initialized and registered to actions manager
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_Rpl = RplComponent.Cast(pOwnerEntity.GetRootParent().FindComponent(RplComponent));
		m_fApplicationInterval = 1 / m_iForceApplicationFrequency;
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_VEHICLES_UNFLIP, "", "Visualize push force", "Vehicles");
#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Can this action be shown in the UI to the provided user entity?
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		if (character.IsInVehicle())
			return false;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;

		if (controller.IsSwimming())
			return false;

		if (controller.GetStance() == ECharacterStance.PRONE)
			return false;

		IEntity owner = GetOwner().GetRootParent();
		CharacterAnimationComponent characterAnimationComp = controller.GetAnimationComponent();
		if (!characterAnimationComp || characterAnimationComp.GetLinkedEntity() == owner)
			return false;

		Physics physics = owner.GetPhysics();
		if (!physics)
			return false;

		if (m_fLinearSpeedLimit > 0 && physics.GetVelocity().LengthSq() > m_fLinearSpeedLimit * m_fLinearSpeedLimit)
			return false;

		if (m_fAngularSpeedLimit > 0 && physics.GetAngularVelocity().LengthSq() > m_fAngularSpeedLimit * m_fAngularSpeedLimit)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyForce(IEntity rootEntity, IEntity pUserEntity, float timeSlice, vector forceOffset = vector.Zero)
	{
		// Only run where vehicle is being simulated
		if (!m_Rpl || m_Rpl.Role() != RplRole.Authority && !m_Rpl.IsOwnerProxy())
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		m_fApplicationDeltaTime += timeSlice;
		if (m_fApplicationDeltaTime < m_fApplicationInterval)
			return;

		// Get force origin, target and direction
		vector forceOrigin = character.EyePosition();
		Physics physics = rootEntity.GetPhysics();
		vector forceTarget = rootEntity.CoordToParent(physics.GetCenterOfMass());
		//equal Y to ensure that foce vector isnt going to have odd angle because of the distance between the player and the vehicle
		forceOrigin[1] = forceTarget[1];
		vector forceDirection = vector.Direction(forceTarget, forceOrigin);
		//normalized to prevent force scaling based on how far player is away from the vehicle
		forceDirection.Normalize();

		float force = Math.Min(m_fMassToForce * physics.GetMass(), m_fForceLimit);
		if (!m_bPull)
			force *= -1;

		float linearSpeedFactor = 1;
		if (m_fLinearSpeedLimit > 0)
			linearSpeedFactor -= Math.Clamp(physics.GetVelocity().LengthSq() / (m_fLinearSpeedLimit * m_fLinearSpeedLimit), 0, 1);

		float angularSpeedFactor = 1;
		if (m_fAngularSpeedLimit > 0)
			angularSpeedFactor -= Math.Clamp(physics.GetAngularVelocity().LengthSq() / (m_fAngularSpeedLimit * m_fAngularSpeedLimit), 0, 1);

		vector impulse = (m_fApplicationDeltaTime * force * linearSpeedFactor * angularSpeedFactor) * forceDirection;
		m_fApplicationDeltaTime = 0;
		// Apply force above and below center of mass for best efficiency
		physics.ApplyImpulseAt(forceTarget + forceOffset, impulse * FORCE_SCALE);

		if (forceOffset != vector.Zero)
			physics.ApplyImpulseAt(forceTarget - (forceOffset * m_fCounterForceOffsetMultiplier), impulse * COUNTERFORCE_SCALE);//lowered by another 50% to reduce sliding

#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_VEHICLES_UNFLIP))
		{
			m_aShapes.Clear();
			ShapeFlags shapeFlags = ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOOUTLINE;
			m_aShapes.Insert(Shape.CreateArrow(forceTarget, forceTarget + forceOffset, 0.1, Color.BLUE, shapeFlags));
			m_aShapes.Insert(Shape.CreateArrow(forceOrigin, forceOrigin - forceDirection, 0.1, Color.RED, shapeFlags));
			m_aShapes.Insert(Shape.CreateArrow(forceTarget + forceOffset, forceTarget + forceOffset + impulse, 1, Color.GREEN, shapeFlags));
			if (forceOffset != vector.Zero)
			{
				m_aShapes.Insert(Shape.CreateArrow(forceTarget, forceTarget - (forceOffset * m_fCounterForceOffsetMultiplier), 0.1, Color.PINK, shapeFlags));
				m_aShapes.Insert(Shape.CreateArrow(forceTarget - (forceOffset * m_fCounterForceOffsetMultiplier), forceTarget - (forceOffset * m_fCounterForceOffsetMultiplier) + (impulse * COUNTERFORCE_SCALE), 1, Color.BLACK, shapeFlags));
			}
		}
#endif
	}

	//------------------------------------------------------------------------------------------------
	//! Called when someone tries to perform the continuous action, user entity is typically character
	override void PerformContinuousAction(IEntity pOwnerEntity, IEntity pUserEntity, float timeSlice)
	{
		ApplyForce(pOwnerEntity.GetRootParent(), pUserEntity, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! Method called from scripted interaction handler when an action is started (progress bar appeared)
	//! \param pUserEntity The entity that started performing this action
	override void OnActionStart(IEntity pUserEntity)
	{
		super.OnActionStart(pUserEntity);
		m_fApplicationDeltaTime = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Method called when the action is interrupted/canceled.
	//! \param pUserEntity The entity that was performing this action prior to interruption
	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_VEHICLES_UNFLIP))
		{
			GetGame().GetCallqueue().CallLater(ClearDebug, 10000);
		}
#endif
		super.OnActionCanceled(pOwnerEntity, pUserEntity);
		m_fApplicationDeltaTime = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the progress of this action in seconds.
	override float GetActionProgressScript(float fProgress, float timeSlice)
	{
		return fProgress + timeSlice;
	}

	//------------------------------------------------------------------------------------------------
	//! If HasLocalEffectOnly() is false this method tells if the server is supposed to broadcast this action to clients.
	override bool CanBroadcastScript()
	{
		return false;
	}
}
