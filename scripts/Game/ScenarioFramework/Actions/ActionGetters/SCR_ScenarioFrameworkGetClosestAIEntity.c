[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetClosestAIEntity : SCR_ScenarioFrameworkGet
{
	[Attribute(desc: "Closest to what - use getter")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "(Optional) You can filter players by putting the Faction Key of desired faction(s). If not used, all factions will be eligible")]
	ref array<string> m_aFactionKeys;

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (!m_Getter)
		{
			Print("ScenarioFramework: GetClosestPlayerEntity - The object the distance is calculated from is missing!", LogLevel.ERROR);
			return null;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
			return null;

		IEntity entityFrom = entityWrapper.GetValue();
		if (!entityFrom)
			return null;

		array<AIAgent> agents = {};
		GetGame().GetAIWorld().GetAIAgents(agents);

		IEntity closestEntity;
		IEntity entityToBeChecked;
		SCR_ChimeraCharacter chimeraCharacter;
		float fDistance = float.MAX;
		bool factionKeyIsEmpty = m_aFactionKeys.IsEmpty();
		
		if (!factionKeyIsEmpty)
		{
			// Resolve Alias
			SCR_FactionAliasComponent factionAliasComponent = SCR_FactionAliasComponent.Cast(GetGame().GetFactionManager().FindComponent(SCR_FactionAliasComponent));
			if (factionAliasComponent) 
			{
				array<string> aliasedFactionKeys = {};
				foreach (string factionKey : m_aFactionKeys)
				{
					aliasedFactionKeys.Insert(factionAliasComponent.ResolveFactionAlias(factionKey))
				}
			
				if (!aliasedFactionKeys.IsEmpty())
				{
					m_aFactionKeys.Clear();
					m_aFactionKeys.InsertAll(aliasedFactionKeys);
				
				}
			}
		}
		
		foreach (AIAgent agent : agents)
		{
			entityToBeChecked = agent.GetControlledEntity();
			if (!entityToBeChecked)
				continue;
			
			if (!factionKeyIsEmpty)
			{
				chimeraCharacter = SCR_ChimeraCharacter.Cast(entityToBeChecked);
				if (!chimeraCharacter || !m_aFactionKeys.Contains(chimeraCharacter.GetFactionKey()))
					continue;
			}

			float fActualDistance = vector.DistanceSqXZ(entityFrom.GetOrigin(), entityToBeChecked.GetOrigin());

			if (fActualDistance < fDistance)
			{
				closestEntity = entityToBeChecked;
				fDistance = fActualDistance;
			}
		}

		if (!closestEntity)
			return null;

		return new SCR_ScenarioFrameworkParam<IEntity>(closestEntity);
	}
}