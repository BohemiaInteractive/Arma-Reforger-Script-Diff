#define SCRIPTED_AIM_MODIFIER_DEBUG
class SCR_RecoilForceAimModifier : ScriptedWeaponAimModifier
{
	[Attribute(desc: "How much rotational force should be applied", params: "0 inf")]
	protected vector m_vRotationOffset;

	[Attribute(desc: "If game should check the ground under the gun to determine how much recoil should be applied")]
	protected bool m_bCheckRestingSurfaceDensity;

	protected float m_fRemainingTime;
	protected AimingComponent m_AimingComp;
	protected int m_iSeed;

	//------------------------------------------------------------------------------------------------
	override void OnInit(IEntity weaponEnt)
	{
		if (m_vRotationOffset == vector.Zero)
			return;

		m_AimingComp = AimingComponent.Cast(weaponEnt.FindComponent(AimingComponent));
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
		RplComponent projectileRpl = RplComponent.Cast(projectileEntity.FindComponent(RplComponent));
		if (!projectileRpl)
			return;

		//needed to have the same seed for all clients to adjust the weapon in a same way
		m_iSeed = projectileRpl.Id().ToString().ToInt();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnWeaponFired()
	{
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
		if (!float.AlmostEqual(m_vRotationOffset[0], 0))
			newRotation[0] = newRotation[0] + (randomValueGenerator.RandFloatXY(-m_vRotationOffset[0], m_vRotationOffset[0]) * surfaceDensityMultiplier);

		if (!float.AlmostEqual(m_vRotationOffset[1], 0))
			newRotation[1] = newRotation[1] + (randomValueGenerator.RandFloatXY(-m_vRotationOffset[1], m_vRotationOffset[1]) * surfaceDensityMultiplier);

		if (!float.AlmostEqual(m_vRotationOffset[2], 0))
			newRotation[2] = newRotation[2] + (randomValueGenerator.RandFloatXY(-m_vRotationOffset[2], m_vRotationOffset[2]) * surfaceDensityMultiplier);

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