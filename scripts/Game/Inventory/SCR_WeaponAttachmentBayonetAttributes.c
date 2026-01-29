class SCR_WeaponAttachmentBayonetAttributes : SCR_WeaponAttachmentAttributes
{
	[Attribute("1", UIWidgets.CheckBox, "Treat this attachment as a bayonet with regards to animation and sound.")]
	protected bool m_bIsBayonet;
	
	[Attribute("1.0", UIWidgets.EditBox, "Damage Modifier. If this value is greater than 1.0, weapon damage is increased with this attachment.")]
	protected float m_fDamageModificationFactor;
	
	[Attribute("1.0", UIWidgets.EditBox, "Range Modifier. Greater than 1 increases the range of the weapon.")]
	protected float m_fRangeModificationFactor;
	
	[Attribute("1.0", UIWidgets.EditBox, "Precision modifier. Values between 0 and 1 make the attack more precise.")]
	protected float m_fPrecisionModificationFactor;
	
	[Attribute("0.0", UIWidgets.EditBox, "Extra Obstruction length. Extra length for weapon obstruction test, in meters.")]
	protected float m_fExtraObstructionLength;
	
	override bool ApplyModifiers(BaseWeaponStatsManagerComponent statsManager, int muzzleIndex, IEntity attachedEntity)
	{
		if (!statsManager.SetIsBayonet(attachedEntity, m_bIsBayonet))
			return false;
		
		if (!statsManager.SetExtraObstructionLength(attachedEntity, m_fExtraObstructionLength))
			return false;
		
		if (!statsManager.SetMeleeDamageFactor(attachedEntity, m_fDamageModificationFactor))
			return false;
		
		if (!statsManager.SetMeleeRangeFactor(attachedEntity, m_fRangeModificationFactor))
			return false;
		
		if (!statsManager.SetMeleeAccuracyFactor(attachedEntity, m_fPrecisionModificationFactor))
			return false;	
		
		return true;
	}
	
	override void ClearModifiers(BaseWeaponStatsManagerComponent statsManager, int muzzleIndex, IEntity attachedEntity)
	{
		statsManager.ClearIsBayonet(attachedEntity);
		statsManager.ClearExtraObstructionLength(attachedEntity);
		statsManager.ClearMeleeDamageFactor(attachedEntity);
		statsManager.ClearMeleeRangeFactor(attachedEntity);
		statsManager.ClearMeleeAccuracyFactor(attachedEntity);
	} 
}