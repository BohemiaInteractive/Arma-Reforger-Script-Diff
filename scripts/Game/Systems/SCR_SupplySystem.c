class SCR_SupplySystem : SCR_ResourceSystem
{
	override static void InitInfo(WorldSystemInfo outInfo)
	{
		super.InitInfo(outInfo);
		outInfo.AddPoint(ESystemPoint.FixedFrame);
	}

	[Attribute()]
	protected ResourceName m_sSupplyEffectsConfig;
	protected ref SCR_SupplyEffectsConfig m_SupplyEffects;

	//------------------------------------------------------------------------------------------------
	void ActivateEffects(EResourcePlayerInteractionType interactionType, SCR_ResourceComponent resourceComponentFrom, SCR_ResourceComponent resourceComponentTo, EResourceType resourceType, float resourceValue)
	{
		Rpc_ActivateEffects(interactionType, Replication.FindId(resourceComponentFrom), Replication.FindId(resourceComponentTo), resourceType, resourceValue);
		Rpc(Rpc_ActivateEffects, interactionType, Replication.FindId(resourceComponentFrom), Replication.FindId(resourceComponentTo), resourceType, resourceValue);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_ActivateEffects(EResourcePlayerInteractionType interactionType, RplId rplIdResourceComponentFrom, RplId rplIdResourceComponentTo, EResourceType resourceType, float resourceValue)
	{
		if (!m_SupplyEffects)
			return;

		SCR_ResourceComponent resourceComponentFrom = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdResourceComponentFrom));
		if (!resourceComponentFrom)
			return;

		SCR_ResourceComponent resourceComponentTo = SCR_ResourceComponent.Cast(Replication.FindItem(rplIdResourceComponentTo));
		if (resourceComponentTo)
			m_SupplyEffects.ActivateEffect(interactionType, resourceComponentFrom, resourceComponentTo, resourceType, resourceValue);
	}

	//------------------------------------------------------------------------------------------------
	override event protected void OnStarted()
	{
		super.OnStarted();
		
		if (m_sSupplyEffectsConfig.GetPath().IsEmpty())
			return;

		Resource holder = BaseContainerTools.LoadContainer(m_sSupplyEffectsConfig);
		if (!holder || !holder.IsValid())
			return;

		BaseContainer container = holder.GetResource().ToBaseContainer();
		if (!container)
			return;

		m_SupplyEffects = SCR_SupplyEffectsConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));		
	}
}