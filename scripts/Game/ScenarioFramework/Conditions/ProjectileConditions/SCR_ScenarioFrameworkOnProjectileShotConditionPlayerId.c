[BaseContainerProps()]
class SCR_ScenarioFrameworkOnProjectileShotConditionPlayerId : SCR_ScenarioFrameworkOnProjectileShotConditionBase
{
	[Attribute(desc: "Player IDs")]
	ref array<int> m_aPlayerIDs;
	//------------------------------------------------------------------------------------------------
	override bool ConditionCheck(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkOnProjectileShotConditionPlayerId.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		return m_aPlayerIDs && m_aPlayerIDs.Contains(playerID);
	}
}