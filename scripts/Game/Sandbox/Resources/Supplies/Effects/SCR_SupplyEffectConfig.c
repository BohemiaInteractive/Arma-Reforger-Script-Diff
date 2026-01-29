[BaseContainerProps(configRoot: true)]
class SCR_SupplyEffectsConfig
{
	[Attribute()]
	protected ref array<ref SCR_SupplyEffectBase> m_aSupplyEffects;

	void ActivateEffect(EResourcePlayerInteractionType interactionType, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue)
	{
		foreach (SCR_SupplyEffectBase effect : m_aSupplyEffects)
		{
			effect.ActivateEffect(interactionType, resourceComponentFrom, resourceComponentTo, resourceType, resourceValue);
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_SupplyEffectBase
{
	void ActivateEffect(EResourcePlayerInteractionType interactionType, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue);
}