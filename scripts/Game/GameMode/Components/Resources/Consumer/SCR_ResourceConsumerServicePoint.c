[BaseContainerProps(configRoot: true)]
class SCR_ResourceConsumerServicePoint : SCR_ResourceConsumer
{
	protected SCR_ServicePointComponent m_ServicePointComponent;
	protected ref set<SCR_ResourceConsumerMilitaryBase> m_aBaseConsumers;
	
	//------------------------------------------------------------------------------------------------
	override bool CanInteractWith(notnull SCR_ResourceContainer container)
	{
		if (m_aBaseConsumers)
		{
			foreach (SCR_ResourceConsumerMilitaryBase baseConsumer : m_aBaseConsumers)
			{
				if (baseConsumer && container.IsInteractorLinked(baseConsumer))
					return super.CanInteractWith(container);
			}
		}
		
		return super.CanInteractWith(container);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnResourceGridUpdated(notnull SCR_ResourceGrid grid)
	{
		if (m_aBaseConsumers)
		{
			foreach (SCR_ResourceConsumerMilitaryBase baseConsumer : m_aBaseConsumers)
			{
				if (baseConsumer)
					grid.UpdateInteractor(baseConsumer);
			}
		}
		
		super.OnResourceGridUpdated(grid);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Event that gets called upon a base registering the service.
	//! \param[in] servicePoint The service point that was registered, in this case it is expected to
	//!		be the service point tied to this consumer.
	//! \param[in] base The base that registered the service.
	void OnBaseRegistered(notnull SCR_ServicePointComponent servicePoint, notnull SCR_MilitaryBaseComponent base)
	{
		SCR_ResourceComponent baseResourceComponent = SCR_ResourceComponent.Cast(base.GetOwner().FindComponent(SCR_ResourceComponent));
		
		if (!baseResourceComponent)
			return;
		
		SCR_ResourceConsumerMilitaryBase baseConsumer = SCR_ResourceConsumerMilitaryBase.Cast(baseResourceComponent.GetConsumer(m_eGeneratorIdentifier, m_eResourceType));
		
		if (!baseConsumer)
			return;
		
		if (!m_aBaseConsumers)
			m_aBaseConsumers = new set<SCR_ResourceConsumerMilitaryBase>();
		
		m_aBaseConsumers.Insert(baseConsumer);
		baseConsumer.GetOnBaseContainerRegistered().Insert(OnBaseContainerRegistered);
		baseConsumer.GetOnBaseContainerUnregistered().Insert(OnBaseContainerUnregistered);
		
		SCR_ResourceContainerQueueBase containerQueue = baseConsumer.GetContainerQueue();
		SCR_ResourceContainer container;
		
		for (int idx = containerQueue.GetContainerCount() - 1; idx >= 0; --idx)
		{
			container = containerQueue.GetContainerAt(idx);
			
			if (!container)
				continue;
			
			OnBaseContainerRegistered(baseConsumer, container);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Event that gets called upon a base unregistering the service.
	//! \param[in] servicePoint The service point that was unregistered, in this case it is expected to
	//!		be the service point tied to this consumer.
	//! \param[in] base The base that unregistered the service.
	void OnBaseUnregistered(notnull SCR_ServicePointComponent servicePoint, notnull SCR_MilitaryBaseComponent base)
	{
		SCR_ResourceComponent baseResourceComponent = SCR_ResourceComponent.Cast(base.GetOwner().FindComponent(SCR_ResourceComponent));
		
		if (!baseResourceComponent)
			return;
		
		SCR_ResourceConsumerMilitaryBase baseConsumer = SCR_ResourceConsumerMilitaryBase.Cast(baseResourceComponent.GetConsumer(m_eGeneratorIdentifier, m_eResourceType));
		
		if (!baseConsumer)
			return;
		
		if (m_aBaseConsumers)
			m_aBaseConsumers.RemoveItem(baseConsumer);
		
		baseConsumer.GetOnBaseContainerRegistered().Remove(OnBaseContainerRegistered);
		baseConsumer.GetOnBaseContainerUnregistered().Remove(OnBaseContainerUnregistered);
		
		SCR_ResourceContainerQueueBase containerQueue = baseConsumer.GetContainerQueue();
		SCR_ResourceContainer container;
		
		for (int idx = containerQueue.GetContainerCount() - 1; idx >= 0; --idx)
		{
			container = containerQueue.GetContainerAt(idx);
			
			if (!container)
				continue;
			
			OnBaseContainerUnregistered(baseConsumer, container);
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Event that gets called upon a base registering a resource container.
	//! \param[in] consumer The consumer that registered the container, in this case it is expected
	//!		to be the base that registered the container.
	//! \param[in] container The container that was registered into the base.
	protected void OnBaseContainerRegistered(notnull SCR_ResourceConsumerMilitaryBase consumer, notnull SCR_ResourceContainer container)
	{
		if (!m_aBaseConsumers)
			return;
		
		if (!container.IsInteractorLinked(this))
			RegisterContainer(container);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Event that gets called upon a base unregistering a resource container.
	//! \param[in] consumer The consumer that unregistered the container, in this case it is expected
	//!		to be the base that unregistered the container.
	//! \param[in] container The container that was unregistered into the base.
	protected void OnBaseContainerUnregistered(notnull SCR_ResourceConsumerMilitaryBase consumer, notnull SCR_ResourceContainer container)
	{
		if (!m_aBaseConsumers)
			return;
		
		foreach (SCR_ResourceConsumerMilitaryBase baseConsumer : m_aBaseConsumers)
		{
			if (baseConsumer && container.IsInteractorLinked(baseConsumer))
				return;
		}
		
		UnregisterContainer(container);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Initialize(notnull IEntity owner)
	{
		super.Initialize(owner);
		
		m_ServicePointComponent = SCR_ServicePointComponent.Cast(GetOwner().FindComponent(SCR_ServicePointComponent));
		
		if (!m_ServicePointComponent)
			return;
		
		m_ServicePointComponent.GetOnBaseRegistered().Insert(OnBaseRegistered);
		m_ServicePointComponent.GetOnBaseUnregistered().Insert(OnBaseUnregistered);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Clear()
	{
		super.Clear();
		
		if (!m_ServicePointComponent)
			return;
		
		m_ServicePointComponent.GetOnBaseRegistered().Remove(OnBaseRegistered);
		m_ServicePointComponent.GetOnBaseUnregistered().Remove(OnBaseUnregistered);
	}
}