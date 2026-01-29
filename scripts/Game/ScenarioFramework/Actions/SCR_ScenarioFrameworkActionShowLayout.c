[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionShowLayout : SCR_ScenarioFrameworkActionBase
{
	[Attribute("", UIWidgets.ResourceNamePicker, "", "layout")]
	protected ResourceName m_sLayout;
	
	[Attribute(desc: "Fade in time", params: "0 inf 0.01")]
	float m_fFadeIn;
	
	[Attribute(desc: "Fade out time", params: "0 inf 0.01")]
	float m_fFadeOut;
	
	[Attribute(desc: "For how long the layout should stay visible.", params: "0 inf 0.001")]
	float m_fVisibilityTime;
	
	[Attribute(desc: "Target value for opacity.", params: "0 1")]
	float m_fOpacityValue;
	
	[Attribute(desc: "Unique layout ID")]
	int m_iID;

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

		int playerId = -1;
		if (EntityUtils.IsPlayer(object))
			playerId = playerManager.GetPlayerIdFromControlledEntity(object);

		array<IEntity> aEntities;

		if (m_Getter)
		{
			// Getter takes the priority. We set it back to -1 in case that object was player.
			playerId = -1;

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
					playerId = playerManager.GetPlayerIdFromControlledEntity(entityFrom);
			}
		}
		
		if (!aEntities)
		{
			scenarioFrameworkSystem.ShowLayout(playerId, object.GetID(), m_iID, m_fFadeIn, m_fFadeOut, m_fVisibilityTime, m_fOpacityValue);
		}
		else
		{
			foreach (IEntity entity : aEntities)
			{
				if (!EntityUtils.IsPlayer(entity))
					continue;

				playerId = playerManager.GetPlayerIdFromControlledEntity(entity);
				scenarioFrameworkSystem.ShowLayout(playerId, object.GetID(), m_iID, m_fFadeIn, m_fFadeOut, m_fVisibilityTime, m_fOpacityValue);
			}
		}
	}
			
	//------------------------------------------------------------------------------------------------
	ResourceName GetLayoutResource()
	{
		return m_sLayout;
	}
}