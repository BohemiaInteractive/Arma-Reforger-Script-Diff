[BaseContainerProps()]
class SCR_AIDangerReaction_StartedBleeding : SCR_AIDangerReaction
{
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount)
	{
		return true;
	}
};