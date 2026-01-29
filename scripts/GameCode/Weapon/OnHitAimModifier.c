#define SCRIPTED_AIM_MODIFIER_DEBUG
//------------------------------------------------------------------------------------------------
//! ONHIT AIM MODIFIER
//------------------------------------------------------------------------------------------------
class OnHitAimModifier : ScriptedWeaponAimModifier
{
	private bool m_bIsSleeping = true;

	//	The maximum amount of damage this effect is scaled to.
	const float m_fEffectScale = 80;

	[Attribute("0.06", uiwidget: UIWidgets.Slider, params: "0 1 0.001")]
	protected float m_fLinearScale;

	[Attribute("25", uiwidget: UIWidgets.Slider, params: "0 100 0.1")]
	protected float m_fAngularScale;

	private vector m_vTarget;
	private vector m_vCurrent;
	private vector m_vVelocity;

	[Attribute("2.65", uiwidget: UIWidgets.Slider, desc: "How long is the spring effect", params: "0 10 0.001")]
	protected float m_fSpring;

	[Attribute("1.67", uiwidget: UIWidgets.Slider, desc: "High damping will make the spring more bouncy", params: "0 10 0.001")]
	protected float m_fDamping;

	[Attribute("10", uiwidget: UIWidgets.Slider, desc: "Speed of interpolation for spring", params: "0 20 0.001")]
	protected float m_fSpringSpeed;

	[Attribute("0.1", uiwidget: UIWidgets.Slider, desc: "Rate of speed reduction when spring reaches the target value", params: "0 1 0.001")]
	protected float m_fDecaySpeed;

	[Attribute("1", uiwidget: UIWidgets.Slider, params: "0 5 0.001")]
	protected float m_fRollRotationIntensity;

	[Attribute("0.75", uiwidget: UIWidgets.Slider, params: "0 5 0.001")]
	protected float m_fYawRotationIntensity;

	[Attribute("1", uiwidget: UIWidgets.Slider, params: "0 5 0.001")]
	protected float m_fPitchRotationIntensity;

	protected IEntity m_pOwner;
	#ifdef ENABLE_DIAG
	private ChimeraCharacter m_pCharacter;
	#endif

	//------------------------------------------------------------------------------------------------
	protected override void OnInit(IEntity weaponEnt)
	{
		m_pOwner = weaponEnt;
		#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_HITREACTION_AIMMODIFIER,"","AimModifier Debug", "Character");
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CHARACTER_HITREACTION_AIMMODIFIER,0);
		m_fDiagSpring = m_fSpring * m_fDiagMul;
		m_fDiagDamping = m_fDamping * m_fDiagMul;
		m_fDiagLinearScale = m_fLinearScale * m_fDiagMul;
		m_fDiagAngularScale = m_fAngularScale * m_fDiagMul;
		m_fDiagSpeed = m_fSpringSpeed * m_fDiagMul;
		#endif
	}
	//------------------------------------------------------------------------------------------------
	protected override void OnActivated(IEntity weaponOwner)
	{
		if (!weaponOwner)
			return;
		ChimeraCharacter parentCharacter = ChimeraCharacter.Cast(weaponOwner);
		if (!parentCharacter)
			return;

		SCR_CharacterDamageManagerComponent dmgManager = SCR_CharacterDamageManagerComponent.Cast(parentCharacter.GetDamageManager());
		if (dmgManager)
			dmgManager.GetOnDamage().Insert(OnDamage);


		#ifdef ENABLE_DIAG
		m_pCharacter = parentCharacter;
		#endif
	}
	//------------------------------------------------------------------------------------------------
	protected override void OnDeactivated(IEntity weaponOwner)
	{
		if (!weaponOwner)
			return;

		ChimeraCharacter parentCharacter = ChimeraCharacter.Cast(weaponOwner);
		if (!parentCharacter)
			return;

		SCR_CharacterDamageManagerComponent dmgManager = SCR_CharacterDamageManagerComponent.Cast(parentCharacter.GetDamageManager());
		if (dmgManager)
			dmgManager.GetOnDamage().Remove(OnDamage);


		#ifdef ENABLE_DIAG
		m_pCharacter = null;
		#endif
	}

	//------------------------------------------------------------------------------------------------
	void OnDamage(BaseDamageContext damageContext)
	{
		float div;
		if (m_fEffectScale > 0.0)
			div = 1.0 / m_fEffectScale;

		float scaleFactor = damageContext.damageValue * div;
		scaleFactor = Math.Clamp(scaleFactor, 0.0, 1.0);

		if (scaleFactor > 0)
			m_bIsSleeping = false;


		vector direction = damageContext.hitDirection;

		// Scale the input so it's slightly larger at lower values,
		// and not linear, that would mean almost no visible effect at low damage values
		const float POW = 1.8 / 2.0;
		float finalPower = Math.Pow(scaleFactor, POW);
		m_vTarget = direction * finalPower;
		#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_HITREACTION_AIMMODIFIER))
			Print("New target = " + m_vTarget);
		#endif

	}
	//------------------------------------------------------------------------------------------------
	// called every frame when aim offset is calculated
	override void OnCalculate(IEntity owner, WeaponAimModifierContext context, float timeSlice, out vector translation, out vector rotation, out vector turnOffset)
	{
		#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_HITREACTION_AIMMODIFIER))
			DrawDiagWindow(timeSlice);
		#endif

		if (!m_bIsSleeping)
		{
			const float threshold = 0.001;
			if (m_vTarget.LengthSq() < threshold)
				m_bIsSleeping = true;
		}

		if (m_bIsSleeping)
		{
			translation = vector.Zero;
			rotation = vector.Zero;
			turnOffset = vector.Zero;
			return;
		}

		m_vCurrent = VectorSpring(m_vCurrent, m_vTarget, m_vVelocity, m_fSpring, m_fDamping, timeSlice * m_fSpringSpeed);
		m_vTarget = vector.Lerp(m_vTarget, vector.Zero, m_fDecaySpeed);

		translation = m_vCurrent * m_fLinearScale;
		vector linToRot;
		linToRot[1] = Math.AbsFloat(m_vCurrent[2]) * m_fPitchRotationIntensity;
		linToRot[0] = m_vCurrent[0] * m_fYawRotationIntensity;
		linToRot[2] = -linToRot[0] * m_fRollRotationIntensity;

		rotation = linToRot * m_fAngularScale;
		turnOffset = vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	protected vector VectorSpring(vector current, vector target, inout vector velocity, float spring, float damping, float dt)
	{
		float vx = velocity[0];
		float vy = velocity[1];
		float vz = velocity[2];

		current[0] = Math.SmoothSpring(current[0], target[0], vx, spring, damping, dt);
		current[1] = Math.SmoothSpring(current[1], target[1], vy, spring, damping, dt);
		current[2] = Math.SmoothSpring(current[2], target[2], vz, spring, damping, dt);

		velocity = Vector(vx, vy, vz);
		return current;
	}

	//------------------------------------------------------------------------------------------------
	#ifdef ENABLE_DIAG
	private void ResetEffect()
	{
		m_vCurrent = vector.Zero;
		m_vVelocity = vector.Zero;
		m_vTarget = vector.Zero;
	}

	int m_iDiagDamage = 50;
	float m_fDiagSpring;
	float m_fDiagDamping;
	float m_fDiagLinearScale;
	float m_fDiagAngularScale;
	float m_fDiagSpeed;
	const float m_fDiagMul = 100.0;

	//------------------------------------------------------------------------------------------------
	void DrawDiagWindow( float timeSlice )
	{
		DbgUI.Begin("On Hit Diag");
		{
			if (DbgUI.Button("Reset"))
				ResetEffect();

			vector _[3];
			DbgUI.InputInt("Damage", m_iDiagDamage);
			if (DbgUI.Button("Apply damage") || Debug.KeyState(KeyCode.KC_B))
			{
				BaseDamageContext damageContext = new BaseDamageContext();
				damageContext.damageValue = m_iDiagDamage;
				damageContext.damageType = EDamageType.KINETIC;
				OnDamage(damageContext);
				Debug.ClearKey(KeyCode.KC_B);
			}

			DbgUI.SliderFloat("Spring = " + m_fDiagSpring / m_fDiagMul, m_fDiagSpring, 0.0, 3.0 * m_fDiagMul);
			DbgUI.SliderFloat("Damping = " + m_fDiagDamping / m_fDiagMul, m_fDiagDamping, 0.0, 3.0 * m_fDiagMul);
			DbgUI.Spacer(16);
			DbgUI.SliderFloat("Spring Speed = " + m_fDiagSpeed / m_fDiagMul, m_fDiagSpeed, 0.0, 10.0 * m_fDiagMul);
			DbgUI.SliderFloat("Linear Scale = " + m_fDiagLinearScale / m_fDiagMul, m_fDiagLinearScale, 0.0, 1.0 * m_fDiagMul);
			DbgUI.SliderFloat("Angular Scale = " + m_fDiagAngularScale / m_fDiagMul, m_fDiagAngularScale, 0.0, 90.0 * m_fDiagMul);
			m_fSpring = m_fDiagSpring / m_fDiagMul;
			m_fDamping = m_fDiagDamping / m_fDiagMul;
			m_fLinearScale = m_fDiagLinearScale / m_fDiagMul;
			m_fAngularScale = m_fDiagAngularScale / m_fDiagMul;
			m_fSpringSpeed = m_fDiagSpeed / m_fDiagMul;


			DbgUI.Text("Target = " + m_vTarget);

		}
		DbgUI.End();
	}

	#endif

};