class SCR_ExposedFeetDamageEffect : SCR_PersistentDamageEffect
{
	//! Amount of HP below which we will no longer cause damage to the foot, as foot hit zone has 30hp by default
	protected const int FEET_DAMAGE_THRESHOLD = 20;
	//! Minimal amount of seconds before we try again to deal damage and trip the character
	protected const int DAMAGE_COOLDOWN_MIN = 45;
	protected const int DAMAGE_COOLDOWN_MAX = 95;
	//! Penalty applied when character is moving fast. It speeds up the next blister buildup
	protected const int FAST_MOVING_PENALTY_FACTOR = 10;
	protected const int FAST_MOVING_VELOCITY_SQ = 14;
	protected const int MIN_MOVING_VELOCITY_SQ = 2;
	//! Time after which character will transition back from ragdoll state
	protected const int RAGDOLL_TIME = 5;
	//! Total tilt in RADIANS which will allow for character to enter ragdoll
	protected const float RAGDOLL_SURFACE_TILT = 0.314159;
	//! Percentage chance for character to enter ragdoll state when cooldown is finished. Value 0 to 1
	protected const float RAGDOLL_CHANCE = 0.15;
	//! Multiplier applied to the ragdoll chance when both hit zones are below the value from FEET_DAMAGE_THRESHOLD
	protected const float RAGDOLL_CHANCE_MULTIPLIER_FROM_MAX_DMG = 2;
	protected const float MAX_SPEED_LIMIT = 0.80;

	protected static const string RIGHT_FOOT_HIT_ZONE_NAME = "RFoot";
	protected static const string LEFT_FOOT_HIT_ZONE_NAME = "LFoot";

	protected float m_fCooldownTimer;
	protected bool m_bSustainedMaxDamage;
	protected SCR_CharacterControllerComponent m_Controller;

	//------------------------------------------------------------------------------------------------
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);

		evaluator.HandleEffectConsequences(this, dmgManager);
	}

	//------------------------------------------------------------------------------------------------
	override bool HijackDamageEffect(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		return !dmgManager.GetAllPersistentEffectsOfType(SCR_ExposedFeetDamageEffect, false).IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	override void OnEffectAdded(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(dmgManager.GetOwner());
		if (!character)
		{
			Terminate();
			return;
		}

		m_Controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		ApplyMobilityLimits(dmgManager);
		m_fCooldownTimer = Math.RandomInt(DAMAGE_COOLDOWN_MIN, DAMAGE_COOLDOWN_MAX);
	}

	//------------------------------------------------------------------------------------------------
	override void OnEffectRemoved(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		ApplyMobilityLimits(dmgManager);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(float timeSlice, SCR_ExtendedDamageManagerComponent dmgManager)
	{
		bool isMovingFast;
		if (!IsMoving(isMovingFast))
			return;

		if (isMovingFast)
			timeSlice *= FAST_MOVING_PENALTY_FACTOR;

		m_fCooldownTimer -= timeSlice;
		if (m_fCooldownTimer > 0)
			return;

		m_fCooldownTimer = Math.RandomInt(DAMAGE_COOLDOWN_MIN, DAMAGE_COOLDOWN_MAX);
		ApplyEffect(dmgManager);
		ChimeraCharacter character = ChimeraCharacter.Cast(dmgManager.GetOwner());
		if (character)
			TryTripCharacter(character);
	}

	//------------------------------------------------------------------------------------------------
	protected void TryTripCharacter(notnull ChimeraCharacter character)
	{
		float chance = RAGDOLL_CHANCE;
		if (m_bSustainedMaxDamage)
			chance *= RAGDOLL_CHANCE_MULTIPLIER_FROM_MAX_DMG;

		if (Math.RandomFloat01() > chance)
			return;

		if (m_Controller.IsFalling())
			return;

		TraceParam param();
		const vector characterPos = character.GetOrigin();
		param.Start = characterPos + vector.Up * 0.2;
		param.End = characterPos - vector.Up * 0.2;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.Exclude = character;
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		float traceDistance = character.GetWorld().TraceMove(param, TraceFiler);
		if (!param.TraceEnt)
			return;

		if (param.TraceNorm == vector.Zero)
			return;

		if (Math.Acos(vector.Dot(param.TraceNorm, vector.Up)) < RAGDOLL_SURFACE_TILT)
			return;

		m_Controller.MakeOwnerRagdoll_S(RAGDOLL_TIME);
	}

	//------------------------------------------------------------------------------------------------
	protected bool TraceFiler(notnull IEntity e, vector start = "0 0 0", vector dir = "0 0 0")
	{
		while (e)
		{
			if (ChimeraCharacter.Cast(e))
				return false;

			e = e.GetParent();
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsMoving(out bool isMovingFast = false)
	{
		if (m_Controller.GetStance() == ECharacterStance.PRONE || m_Controller.IsSwimming())
			return false;

		float velocity = m_Controller.GetVelocity().LengthSq();
		if (velocity > MIN_MOVING_VELOCITY_SQ)
		{
			isMovingFast = velocity > FAST_MOVING_VELOCITY_SQ || m_Controller.IsSprinting();
			return true;
		}

		isMovingFast = m_Controller.IsFalling();
		if (isMovingFast)
			return true;

		if (m_Controller.IsClimbing())
			return true;

		const CharacterInputContext context = m_Controller.GetInputContext();
		isMovingFast = context.GetJump();
		if (isMovingFast)
			return true;

		return context.IsMoving();
	}

	//------------------------------------------------------------------------------------------------
	float GetResultingDamage(notnull SCR_CharacterDamageManagerComponent dmgManager)
	{
		HitZone hZ = GetAffectedHitZone();
		float remainingAffectedHealth = hZ.GetHealth() - FEET_DAMAGE_THRESHOLD;
		if (remainingAffectedHealth <= 0)
		{
			ECharacterHitZoneGroup group = ECharacterHitZoneGroup.RIGHTLEG;
			if (hZ.GetName() == RIGHT_FOOT_HIT_ZONE_NAME)
				group = ECharacterHitZoneGroup.LEFTLEG;

			m_bSustainedMaxDamage = true;
			hZ = FindFootHitZone(dmgManager, group);
			if (!hZ)
				return 0;

			remainingAffectedHealth = hZ.GetHealth() - FEET_DAMAGE_THRESHOLD;
			if (remainingAffectedHealth < 0)
				return 0;

			SetAffectedHitZone(hZ);
		}

		m_bSustainedMaxDamage = false;
		return Math.Max(remainingAffectedHealth * 0.3, 1);
	}

	//------------------------------------------------------------------------------------------------
	static HitZone FindFootHitZone(notnull SCR_CharacterDamageManagerComponent dmgManager, ECharacterHitZoneGroup group = ECharacterHitZoneGroup.RIGHTLEG)
	{
		array<HitZone> groupHitZones = {};
		dmgManager.GetHitZonesOfGroup(group, groupHitZones, false);
		bool searchingForRightFoot = group == ECharacterHitZoneGroup.RIGHTLEG;
		foreach (HitZone hZ : groupHitZones)
		{
			if (searchingForRightFoot)
			{
				if (hZ.GetName() != RIGHT_FOOT_HIT_ZONE_NAME)
					continue;
			}
			else
			{
				if (hZ.GetName() != LEFT_FOOT_HIT_ZONE_NAME)
					continue;
			}

			return hZ;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyMobilityLimits(notnull SCR_ExtendedDamageManagerComponent dmgManager)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(dmgManager.GetOwner());
		if (!character || !m_Controller)
		{
			Terminate();
			return;
		}

		if (dmgManager.GetAllPersistentEffectsOfType(SCR_ExposedFeetDamageEffect, false).Contains(this))
		{
			character.SetSpeedLimit(this, MAX_SPEED_LIMIT);
			return;
		}

		character.SetSpeedLimit(this, 1);
	}

	//------------------------------------------------------------------------------------------------
	override EDamageType GetDefaultDamageType()
	{
		return EDamageType.MELEE;
	}
}
