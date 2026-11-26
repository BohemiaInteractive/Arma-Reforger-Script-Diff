[BaseContainerProps()]
class SCR_AIDangerReaction_DamageTaken : SCR_AIDangerReaction
{
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount)
	{
		if (dangerEvent.GetVictim() != utility.m_OwnerEntity)
			return false;

		// Healing is reported as damage taken, it is not a danger
		if (threatSystem.IsDamageTakenOnlyHealing())
			return false;

		IEntity shooter = dangerEvent.GetObject();
		
		if (!shooter)
			return false;
		
		vector shooterPos = shooter.GetOrigin();
		
		utility.m_SectorThreatFilter.OnDamageTaken(shooterPos);
		
		return true;
	}
};
