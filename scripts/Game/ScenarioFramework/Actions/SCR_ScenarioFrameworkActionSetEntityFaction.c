[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionSetEntityFaction : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity that faction will be set to.")]
	ref SCR_ScenarioFrameworkGet m_EntityGetter;

	[Attribute(desc: "Faction key")]
	string m_sFactionKey;
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_EntityGetter, entity))
			return;

		FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
		if (!factionComponent)
		{
			if (object)
				PrintFormat("ScenarioFramework Action: Faction component not found for Action %1 attached on %2.", this, object.GetName(), level: LogLevel.ERROR);
			else
				PrintFormat("ScenarioFramework Action: Faction component not found for Action %1.", this, level: LogLevel.ERROR);

			return;
		}

		// Resolve Alias
		SCR_FactionAliasComponent factionAliasComponent = SCR_FactionAliasComponent.Cast(GetGame().GetFactionManager().FindComponent(SCR_FactionAliasComponent));
		if (factionAliasComponent)
			m_sFactionKey = factionAliasComponent.ResolveFactionAlias(m_sFactionKey);

		factionComponent.SetAffiliatedFactionByKey(m_sFactionKey);
	}
}
