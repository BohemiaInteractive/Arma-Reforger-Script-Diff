[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionShowHint : SCR_ScenarioFrameworkActionBase
{
	[Attribute()]
	string m_sTitle;

	[Attribute()]
	string m_sText;

	[Attribute()]
	string m_sActionName;
	
	[Attribute()]
	string m_sActionText;

	[Attribute(defvalue: "15")]
	int	m_iTimeout;

	[Attribute()]
	FactionKey m_sFactionKey;

	[Attribute(desc: "Getter to get either a specific player or array of player entities")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	protected static const string DOUBLE_LINE_SPACE = "<br/><br/>";

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
		if (EntityUtils.IsPlayer(object))
			playerID = playerManager.GetPlayerIdFromControlledEntity(object);

		array<IEntity> aEntities;

		if (m_Getter)
		{
			// Getter takes the priority. We set it back to -1 in case that object was player.
			playerID = -1;

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

		ComposeHintText();
			
		if (!aEntities)
		{
			ShowHint(playerID);
		}
		else
		{
			foreach (IEntity entity : aEntities)
			{
				if (!EntityUtils.IsPlayer(entity))
					continue;

				playerID = playerManager.GetPlayerIdFromControlledEntity(entity);
				ShowHint(playerID);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowHint(int playerID)
	{
		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;

		// Resolve Alias
		SCR_FactionAliasComponent factionAliasComponent = SCR_FactionAliasComponent.Cast(GetGame().GetFactionManager().FindComponent(SCR_FactionAliasComponent));
		if (factionAliasComponent) 
			m_sFactionKey = factionAliasComponent.ResolveFactionAlias(m_sFactionKey);	
		
		scenarioFrameworkSystem.ShowHint(m_sText, m_sTitle, m_iTimeout, m_sFactionKey, playerID);
	}
	
	//------------------------------------------------------------------------------------------------
	void ComposeHintText()
	{
		if (!SCR_StringHelper.IsEmptyOrWhiteSpace(m_sActionName))
			m_sText = string.Format("%1%2%3%2<action name='%4' scale='1.7'/><br>", m_sText, DOUBLE_LINE_SPACE, m_sActionText, m_sActionName);
	}
	
}
