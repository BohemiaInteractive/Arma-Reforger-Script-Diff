[BaseContainerProps(configRoot: true)]
class SCR_ResourceGeneratorServicePoint : SCR_ResourceGenerator
{
	protected SCR_ServicePointComponent m_ServicePointComponent;
	protected ref set<SCR_ResourceGeneratorMilitaryBase> m_aBaseGenerators;
	
	//------------------------------------------------------------------------------------------------
	override bool CanInteractWith(notnull SCR_ResourceContainer container)
	{
		if (m_aBaseGenerators)
		{
			foreach (SCR_ResourceGeneratorMilitaryBase baseGenerator : m_aBaseGenerators)
			{
				if (baseGenerator && container.IsInteractorLinked(baseGenerator))
					return super.CanInteractWith(container);
			}
		}
		
		return super.CanInteractWith(container);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnResourceGridUpdated(notnull SCR_ResourceGrid grid)
	{
		if (m_aBaseGenerators)
		{
			foreach (SCR_ResourceGeneratorMilitaryBase baseGenerator : m_aBaseGenerators)
			{
				if (baseGenerator)
					grid.UpdateInteractor(baseGenerator);
			}
		}
		
		super.OnResourceGridUpdated(grid);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Event that gets called upon a base registering the service.
	//! \param[in] servicePoint The service point that was registered, in this case it is expected to
	//!		be the service point tied to this generator.
	//! \param[in] base The base that registered the service.
	void OnBaseRegistered(notnull SCR_ServicePointComponent servicePoint, notnull SCR_MilitaryBaseComponent base)
	{
		SCR_ResourceComponent baseResourceComponent = SCR_ResourceComponent.Cast(base.GetOwner().FindComponent(SCR_ResourceComponent));
		
		if (!baseResourceComponent)
			return;
		
		SCR_ResourceGeneratorMilitaryBase baseGenerator = SCR_ResourceGeneratorMilitaryBase.Cast(baseResourceComponent.GetGenerator(m_eIdentifier, m_eResourceType));
		
		if (!baseGenerator)
			return;
		
		if (!m_aBaseGenerators)
			m_aBaseGenerators = new set<SCR_ResourceGeneratorMilitaryBase>();
		
		m_aBaseGenerators.Insert(baseGenerator);
		baseGenerator.GetOnBaseContainerRegistered().Insert(OnBaseContainerRegistered);
		baseGenerator.GetOnBaseContainerUnregistered().Insert(OnBaseContainerUnregistered);
		
		SCR_ResourceContainerQueueBase containerQueue = baseGenerator.GetContainerQueue();
		SCR_ResourceContainer container;
		
		for (int idx = containerQueue.GetContainerCount() - 1; idx >= 0; --idx)
		{
			container = containerQueue.GetContainerAt(idx);
			
			if (!container)
				continue;
			
			OnBaseContainerRegistered(baseGenerator, container);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Event that gets called upon a base unregistering the service.
	//! \param[in] servicePoint The service point that was unregistered, in this case it is expected to
	//!		be the service point tied to this generator.
	//! \param[in] base The base that unregistered the service.
	void OnBaseUnregistered(notnull SCR_ServicePointComponent servicePoint, notnull SCR_MilitaryBaseComponent base)
	{
		SCR_ResourceComponent baseResourceComponent = SCR_ResourceComponent.Cast(base.GetOwner().FindComponent(SCR_ResourceComponent));
		
		if (!baseResourceComponent)
			return;
		
		SCR_ResourceGeneratorMilitaryBase baseGenerator = SCR_ResourceGeneratorMilitaryBase.Cast(baseResourceComponent.GetGenerator(m_eIdentifier, m_eResourceType));
		
		if (!baseGenerator)
			return;
		
		if (m_aBaseGenerators)
			m_aBaseGenerators.RemoveItem(baseGenerator);
		
		baseGenerator.GetOnBaseContainerRegistered().Remove(OnBaseContainerRegistered);
		baseGenerator.GetOnBaseContainerUnregistered().Remove(OnBaseContainerUnregistered);
		
		SCR_ResourceContainerQueueBase containerQueue = baseGenerator.GetContainerQueue();
		SCR_ResourceContainer container;
		
		for (int idx = containerQueue.GetContainerCount() - 1; idx >= 0; --idx)
		{
			container = containerQueue.GetContainerAt(idx);
			
			if (!container)
				continue;
			
			OnBaseContainerUnregistered(baseGenerator, container);
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Event that gets called upon a base registering a resource container.
	//! \param[in] generator The generator that registered the container, in this case it is expected
	//!		to be the base that registered the container.
	//! \param[in] container The container that was registered into the base.
	protected void OnBaseContainerRegistered(notnull SCR_ResourceGeneratorMilitaryBase generator, notnull SCR_ResourceContainer container)
	{
		if (!m_aBaseGenerators)
			return;
		
		if (!container.IsInteractorLinked(this))
			RegisterContainer(container);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Event that gets called upon a base unregistering a resource container.
	//! \param[in] generator The generator that unregistered the container, in this case it is expected
	//!		to be the base that unregistered the container.
	//! \param[in] container The container that was unregistered into the base.
	protected void OnBaseContainerUnregistered(notnull SCR_ResourceGeneratorMilitaryBase generator, notnull SCR_ResourceContainer container)
	{
		if (!m_aBaseGenerators)
			return;
		
		foreach (SCR_ResourceGeneratorMilitaryBase baseGenerator : m_aBaseGenerators)
		{
			if (baseGenerator && container.IsInteractorLinked(baseGenerator))
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