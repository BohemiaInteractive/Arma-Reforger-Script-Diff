class SCR_WeaponAttachmentSuppressorAttributes : SCR_WeaponAttachmentAttributes
{
	[Attribute("1.0", UIWidgets.EditBox, "Muzzle Velocity Coefficient. Values >1 speed up the bullet, values below slow it down")]
	protected float m_fMuzzleSpeedCoefficient;
	[Attribute("1", UIWidgets.CheckBox, "Override sound for shots")]
	protected bool m_bOverrideShot;
	[Attribute("1", UIWidgets.CheckBox, "Override muzzle effects")]
	protected bool m_bOverrideMuzzleEffects;
	[Attribute("1.0", UIWidgets.EditBox, "Muzzle Dispersion Factor. Affects the radius of impact at the given range in the weapon. Smaller than 1.0 makes the weapon more accurate")]
	protected float m_fMuzzleDispersionFactor;

	[Attribute("0.0", UIWidgets.EditBox, "Extra Obstruction Length")]
	protected float m_fExtraObstructionLength;

			
	[Attribute("1 1 1", "Linear Recoil Factors for X/Y/Z Direction. Smaller values reduce recoil.")]
	protected vector m_vLinearFactors;
	[Attribute("1 1 1", "Angular Recoil Factors for X/Y/Z Direction. Smaller values reduce recoil.")]
	protected vector m_vAngularFactors;
	[Attribute("1 1 1", "Turning Recoil Factors for X/Y/Z Direction. Smaller values reduce recoil.")]
	protected vector m_vTurnFactors;
		
	override bool ApplyModifiers(BaseWeaponStatsManagerComponent statsManager, int muzzleIndex, IEntity attachedEntity)
	{
		if (muzzleIndex == -1)
			return false; // Should not happen
		
		if (!statsManager.SetMuzzleVelocityCoefficient(attachedEntity, muzzleIndex, m_fMuzzleSpeedCoefficient))
			return false;
		
		if (!statsManager.SetMuzzleDispersionFactor(attachedEntity, muzzleIndex, m_fMuzzleDispersionFactor))
			return false;
		
		if (m_bOverrideShot)
			if (!statsManager.SetShotSoundOverride(attachedEntity, muzzleIndex, true))
				return false;
		
		if (m_bOverrideMuzzleEffects)
		{
			if (!statsManager.SetMuzzleEffectOverride(attachedEntity, muzzleIndex, true))
				return false;
			array<Managed> comArray = {};
			attachedEntity.FindComponents(MuzzleEffectComponent, comArray);
			foreach (Managed component : comArray)
			{
				MuzzleEffectComponent muzzleEffect = MuzzleEffectComponent.Cast(component);
				if (muzzleEffect)
					statsManager.AddMuzzleEffectOverride(attachedEntity, muzzleIndex, muzzleEffect);
			}
		}
		
		if (!statsManager.SetExtraObstructionLength(attachedEntity, m_fExtraObstructionLength))
			return false;
		
		if (!statsManager.SetRecoilLinearFactors(attachedEntity, muzzleIndex, m_vLinearFactors))
			return false;
		
		if (!statsManager.SetRecoilAngularFactors(attachedEntity, muzzleIndex, m_vAngularFactors))
			return false;
		
		if (!statsManager.SetRecoilTurnFactors(attachedEntity, muzzleIndex, m_vTurnFactors))
			return false;
		
		return true;
	}
	
	override void ClearModifiers(BaseWeaponStatsManagerComponent statsManager, int muzzleIndex, IEntity attachedEntity)
	{
		if (muzzleIndex == -1)
			return;
		statsManager.ClearMuzzleVelocityCoefficient(attachedEntity, muzzleIndex);
		statsManager.ClearMuzzleDispersionFactor(attachedEntity, muzzleIndex);
		
		if (m_bOverrideShot)
			statsManager.ClearShotSoundOverride(attachedEntity, muzzleIndex);
		
		if (m_bOverrideMuzzleEffects)
			statsManager.ClearMuzzleEffectOverride(attachedEntity, muzzleIndex);
		
		statsManager.ClearExtraObstructionLength(attachedEntity);
		statsManager.ClearRecoilLinearFactors(attachedEntity, muzzleIndex);
		statsManager.ClearRecoilAngularFactors(attachedEntity, muzzleIndex);
		statsManager.ClearRecoilTurnFactors(attachedEntity, muzzleIndex);
	}
}