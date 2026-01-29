[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionAddScoreToCAHFaction : SCR_ScenarioFrameworkActionBase
{
	[Attribute()]
	FactionKey m_sFactionKey;
	
	[Attribute(defvalue: "0", desc: "The amount of score to be added to the specified faction.", params: "0 inf")]
	int m_iScoreToBeAdded;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		SCR_FactionAliasComponent factionAliasComponent = SCR_FactionAliasComponent.Cast(factionManager.FindComponent(SCR_FactionAliasComponent));
		if (factionAliasComponent) 
			m_sFactionKey = factionAliasComponent.ResolveFactionAlias(m_sFactionKey);
		
		SCR_BaseScoringSystemComponent scoringSystem = SCR_BaseGameMode.Cast(GetGame().GetGameMode()).GetScoringSystemComponent();
		if (!scoringSystem)
			return;
		
		Faction targetFaction = factionManager.GetFactionByKey(m_sFactionKey);
		if (!targetFaction)
			return;

		scoringSystem.AddFactionObjective(targetFaction, m_iScoreToBeAdded);
	}
}