[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionHideHint : SCR_ScenarioFrameworkActionBase
{
	[Attribute()]
	FactionKey m_sFactionKey;

	[Attribute(desc: "Getter to get either a specific player or array of player entities")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		int playerID = -1;
		array<IEntity> aEntities;

		if (m_Getter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (!entityWrapper)
			{
				SCR_ScenarioFrameworkParam<array<IEntity>> arrayOfEntitiesWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_Getter.Get());
				if (!arrayOfEntitiesWrapper)
					return;

				aEntities = arrayOfEntitiesWrapper.GetValue();
				if (!aEntities)
					return;
			}
			else
			{
				IEntity entityFrom = entityWrapper.GetValue();
				if (entityFrom)
					playerID = playerManager.GetPlayerIdFromControlledEntity(entityFrom);
			}
		}
		else
		{
			if (EntityUtils.IsPlayer(object))
				playerID = playerManager.GetPlayerIdFromControlledEntity(object);
		}

		if (!aEntities)
		{
			if (playerID > -1)
				HideHint(playerID);
		}
		else
		{
			foreach (IEntity entity : aEntities)
			{
				if (!EntityUtils.IsPlayer(entity))
					continue;

				playerID = playerManager.GetPlayerIdFromControlledEntity(entity);
				if (playerID > -1)
					HideHint(playerID);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void HideHint(int playerID)
	{
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;

		// Resolve Alias
		SCR_FactionAliasComponent factionAliasComponent = SCR_FactionAliasComponent.Cast(GetGame().GetFactionManager().FindComponent(SCR_FactionAliasComponent));
		if (factionAliasComponent)
			m_sFactionKey = factionAliasComponent.ResolveFactionAlias(m_sFactionKey);

		scenarioFrameworkSystem.HideHint(m_sFactionKey, playerID);
	}
}
