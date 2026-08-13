class SCR_FeetLoadoutSlotInfo : LoadoutSlotInfo
{
	//------------------------------------------------------------------------------------------------
	//! Runs every time an entity is attached to the slot.
	override void OnAttachedEntity(IEntity entity)
	{
		super.OnAttachedEntity(entity);

		if (!Replication.IsServer())
			return;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;

		SCR_CharacterDamageManagerComponent dmgManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (dmgManager)
			dmgManager.TerminateDamageEffectsOfType(SCR_ExposedFeetDamageEffect);
	}

	//------------------------------------------------------------------------------------------------
	//! Runs every time an entity is detached from the slot.
	override void OnDetachedEntity(IEntity entity)
	{
		super.OnDetachedEntity(entity);

		if (!Replication.IsServer())
			return;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(GetOwner());
		if (!character || character.IsDeleted())
			return;

		SCR_CharacterDamageManagerComponent dmgManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!dmgManager)
			return;

		HitZone footHitZone = SCR_ExposedFeetDamageEffect.FindFootHitZone(dmgManager);
		if (!footHitZone)
			return;

		SCR_ExposedFeetDamageEffect dmgEffect = new SCR_ExposedFeetDamageEffect();
		dmgEffect.SetAffectedHitZone(footHitZone);
		dmgEffect.SetInstigator(Instigator.CreateInstigator(character));
		dmgManager.AddDamageEffect(dmgEffect);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSlotPostInit()
	{
		IEntity boots = GetAttachedEntity();
		if (boots)
			return;

		OnDetachedEntity(null);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_FeetLoadoutSlotInfo()
	{
		GetGame().GetCallqueue().CallLater(OnSlotPostInit, 1000);
	}
}