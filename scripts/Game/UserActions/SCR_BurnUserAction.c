class BurnUserAction : ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		DamageManagerComponent dmgManager = DamageManagerComponent.Cast(pOwnerEntity.FindComponent(DamageManagerComponent));
		if (!dmgManager)
			return;
		
		HitZone mainHz = dmgManager.GetDefaultHitZone();
		mainHz.SetDamageOverTime(EDamageType.FIRE, 2.0);
		
		Print("Burn baby!");
	}
};
