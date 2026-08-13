#define SCRIPTED_AIM_MODIFIER_DEBUG
class SCR_RecoilForceAimModifier : ScriptedWeaponAimModifier
{
	[Attribute(desc: "Maximum amount of rotation that can be applied after each shot.\nX - Left/Right rotation\nY - Up/Down rotation", params: "0 inf", precision: 5)]
	protected vector m_vRotationOffset;

	[Attribute(desc: "Positive X means bias towards RIGHT direction, while negative means that it will more likely rotate to the LEFT.\n Positive Y value means that it will more likely go UP, while negative DOWN.\nZero means no bias.\nOne or minus one will ensure that recoil will be only in certain direction", params: "-1 1")]
	protected vector m_vDirectionBias;

	[Attribute(desc: "If game should check the ground under the gun to determine how much recoil should be applied")]
	protected bool m_bCheckRestingSurfaceDensity;

	protected float m_fRemainingTime;
	protected AimingComponent m_AimingComp;
	protected int m_iSeed;

	protected const int PRIME = 2137;

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity weaponEnt)
	{
		if (m_vRotationOffset == vector.Zero)
			return;
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_TURRET_RECOIL_DISABLE, "", "Disable turret recoil", "Projectiles");
#endif
		m_AimingComp = AimingComponent.Cast(weaponEnt.FindComponent(AimingComponent));
		if (!m_AimingComp)
		{
			IEntity parent = weaponEnt.GetParent();
			if (!parent)
				return;

			m_AimingComp = AimingComponent.Cast(parent.FindComponent(AimingComponent));
			if (!m_AimingComp)
				return;
		}

		SCR_MuzzleEffectComponent muzzleEffectComp = SCR_MuzzleEffectComponent.Cast(weaponEnt.FindComponent(SCR_MuzzleEffectComponent));
		if (muzzleEffectComp)
			muzzleEffectComp.GetOnWeaponFired().Insert(OnProjectileFired);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to trigger mortar recoil animation
	//! \param[in] effectEntity
	//! \param[in] muzzle
	//! \param[in] projectileEntity
	void OnProjectileFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_TURRET_RECOIL_DISABLE))
			return;
#endif
		RplComponent seedSource = SCR_EntityHelper.GetEntityRplComponent(projectileEntity);
		if (seedSource)
		{
			m_iSeed = seedSource.Id().ToString().ToInt(); // needed to have the same seed for all clients to adjust the weapon in a same way
			return;
		}

		BaseMagazineComponent magazineComp = muzzle.GetMagazine();
		if (!magazineComp)
			return;

		// if projectile isn't replicated, then we need to check the magazine for something that can use as replicated seed
		seedSource = SCR_EntityHelper.GetEntityRplComponent(magazineComp.GetOwner());
		if (!seedSource)
			return;

		m_iSeed = seedSource.Id().ToString().ToInt();

		// to ensure different recoil amount for each shot, seed will need to be modified based on the number of the bullet which was fired, as that is also replicated
		int numberOfBullets = magazineComp.GetAmmoCount() + 1;
		int offset = PRIME % numberOfBullets;
		if (numberOfBullets % 2 == 0)
			offset = -offset; // negate for odd bullet to make seed more 'unpredictable'

		offset *= PRIME;
		m_iSeed += offset; // modify the seed by the calculated offset
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnWeaponFired()
	{
#ifdef ENABLE_DIAG
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_WEAPONS_TURRET_RECOIL_DISABLE))
			return;
#endif
		if (m_vRotationOffset == vector.Zero)
			return;

		IEntity owner = m_AimingComp.GetOwner();
		if (!owner)
			return;

		vector newRotation = m_AimingComp.GetAimingRotation();
		float surfaceDensityMultiplier = 1;
		if (m_bCheckRestingSurfaceDensity)
		{
			TraceParam paramGround = new TraceParam();
			paramGround.Start = owner.GetOrigin() + (vector.Up * 0.1);
			paramGround.End = paramGround.Start - vector.Up;
			paramGround.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
			paramGround.Exclude = owner;
			paramGround.LayerMask = EPhysicsLayerPresets.Projectile;
			owner.GetWorld().TraceMove(paramGround, FilterCallback);
			GameMaterial material = paramGround.SurfaceProps;
			BallisticInfo ballisticInfo;
			if (material)
				ballisticInfo = material.GetBallisticInfo();

			if (ballisticInfo)
				surfaceDensityMultiplier = Math.AbsFloat(ballisticInfo.GetDensity() - 1) * 0.2 + 1;
		}

		RandomGenerator randomValueGenerator = new RandomGenerator();
		randomValueGenerator.SetSeed(m_iSeed);
		for (int i; i < 3; i++) // do it for all 3 axis to support odd guns, which may be firing on Right or Up axis, instead of Forward axis
		{
			float max = m_vRotationOffset[i];
			if (float.AlmostEqual(max, 0))
				continue;

			float min = -max;
			float bias = m_vDirectionBias[i] * max;
			min += bias;
			max += bias;
			newRotation[i] = newRotation[i] + (randomValueGenerator.RandFloatXY(min, max) * surfaceDensityMultiplier);
		}

		newRotation *= Math.DEG2RAD;
		m_AimingComp.SetAimingRotation(newRotation);
	}

	//------------------------------------------------------------------------------------------------
	protected bool FilterCallback(IEntity e)
	{
		if (ChimeraCharacter.Cast(e))
			return false;

		return true;
	}
}