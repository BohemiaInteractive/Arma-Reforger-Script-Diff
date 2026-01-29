class SCR_ResourceEncapsulator : SCR_ResourceInteractor
{	
	protected SCR_ResourceContainer m_ContainerRepresentative;
	
	[Attribute(uiwidget: UIWidgets.Object)]
	protected ref SCR_ResourceEncapsulatorContainerQueue m_ContainerQueue;
	
	[Attribute(uiwidget: UIWidgets.Object)]
	protected ref array<ref SCR_ResourceEncapsulatorActionBase> m_aActions;
	
	//------------------------------------------------------------------------------------------------
	override float GetAggregatedResourceValue()
	{
		if (!m_ContainerQueue)
			return m_fAggregatedResourceValue;
		
		return m_ContainerQueue.GetAggregatedResourceValue();
	}
	
	//------------------------------------------------------------------------------------------------
	override float GetAggregatedMaxResourceValue()
	{
		if (!m_ContainerQueue)
			return m_fAggregatedMaxResourceValue;
		
		return m_ContainerQueue.GetAggregatedMaxResourceValue();
	}
	
	//------------------------------------------------------------------------------------------------
	override int GetContainerCount()
	{
		if (!m_ContainerQueue)
			return 0.0;
		
		return m_ContainerQueue.GetContainerCount();
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_ResourceContainerQueueBase GetContainerQueue()
	{
		return m_ContainerQueue;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ResourceContainer GetContainerRepresentative()
	{
		return m_ContainerRepresentative;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_ResourceEncapsulatorActionBase> GetActions()
	{
		return m_aActions;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsAllowed(notnull SCR_ResourceContainer container)
	{
		return container.GetResourceType() == m_eResourceType;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsContainerRepresentative(notnull SCR_ResourceContainer container)
	{
		return container == m_ContainerRepresentative;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool ShouldUpdate()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override int FindContainer(notnull SCR_ResourceContainer container)
	{
		if (!m_ContainerQueue)
			return super.FindContainer(container);
		
		return m_ContainerQueue.FindContainer(container);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetContainerRepresentative(notnull SCR_ResourceContainer container)
	{
		m_ContainerRepresentative = container;
		
		if (m_ContainerRepresentative)
			m_ContainerRepresentative.SetResourceEncapsulator(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RegisterContainer(notnull SCR_ResourceContainer container)
	{		
		if (CanInteractWith(container) 
		&&	m_ContainerQueue 
		&&	m_ContainerQueue.RegisterContainer(container) != SCR_ResourceContainerQueueBase.INVALID_CONTAINER_INDEX)
		{
			OnContainerRegistered(container);
			
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RegisterContainerForced(notnull SCR_ResourceContainer container)
	{		
		if (m_ContainerQueue &&	m_ContainerQueue.RegisterContainer(container) != SCR_ResourceContainerQueueBase.INVALID_CONTAINER_INDEX)
		{
			OnContainerRegistered(container);
			
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool UnregisterContainer(int containerIndex)
	{
		return m_ContainerQueue && m_ContainerQueue.PopContainerAt(containerIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool UnregisterContainer(notnull SCR_ResourceContainer container)
	{	
		return m_ContainerQueue && m_ContainerQueue.PopContainerAt(m_ContainerQueue.FindContainer(container));
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestConsumtion(float resourceCost, bool notifyChange = true)
	{
		float resourceUsed;
		SCR_ResourceContainer container;
		
		for (int i =  m_ContainerQueue.GetContainerCount() - 1; i >= 0 && resourceCost > 0.0; --i)
		{
			container		= m_ContainerQueue.GetContainerAt(i);
			resourceUsed	= Math.Min(resourceCost, container.GetResourceValue());
			
			if (container.GetOnEmptyBehavior() == EResourceContainerOnEmptyBehavior.DELETE && resourceUsed < resourceCost)
				i--;
			
			resourceCost -= resourceUsed;
			
			container.DecreaseResourceValue(resourceUsed, notifyChange);
		}
		
		if (m_ResourceComponent)
			m_ResourceComponent.Replicate();
	}
	
	//------------------------------------------------------------------------------------------------
	void RequestGeneration(float resourceAmount, bool notifyChange = true)
	{
		float resourceUsed
		SCR_ResourceContainer container;
		int containerCount = m_ContainerQueue.GetContainerCount();
		
		for (int i = 0; i < containerCount && resourceAmount > 0.0; ++i)
		{
			container		= m_ContainerQueue.GetContainerAt(i);
			resourceUsed	= Math.Min(container.ComputeResourceDifference(), resourceAmount);
			
			if (resourceUsed <= SCR_ResourceActor.RESOURCES_LOWER_LIMIT)
				continue;
			
			resourceAmount -= resourceUsed;
			
			container.IncreaseResourceValue(resourceUsed, notifyChange);
		}
		
		if (m_ResourceComponent)
			m_ResourceComponent.Replicate();
	}
	
	//------------------------------------------------------------------------------------------------
	void DebugDraw()
	{
		vector origin	= GetOwnerOrigin();
		Color color		= m_ResourceComponent.GetDebugColor();
		
		color.Scale(0.2);
		color.SetA(1.0);
		//Shape.CreateSphere(m_ResourceComponent.GetDebugColor().PackToInt(), ShapeFlags.TRANSP | ShapeFlags.ONCE | ShapeFlags.DOUBLESIDE | ShapeFlags.NOZWRITE, origin, 15.0);
		DebugTextWorldSpace.Create(GetGame().GetWorld(), string.Format("  %1  \n  %2 containers  \n  %3 / %4 resources  \n  %5 m  ", m_sDebugName, GetContainerCount(), GetAggregatedResourceValue(), GetAggregatedMaxResourceValue(), 15.0), DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA, origin[0], origin[1] + 3, origin[2], 10.0, 0xFFFFFFFF, color.PackToInt());
		
		if (m_ContainerQueue)
			m_ContainerQueue.DebugDraw();
	}

	//------------------------------------------------------------------------------------------------
	override void UpdateContainerResourceValue(SCR_ResourceContainer container, float previousValue)
	{
		OnResourcesChanged(
			m_ContainerQueue.UpdateContainerResourceValue(container.GetResourceValue(), previousValue)
		);
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateContainerMaxResourceValue(SCR_ResourceContainer container, float previousValue)
	{
		OnMaxResourcesChanged(
			m_ContainerQueue.UpdateContainerMaxResourceValue(container.GetMaxResourceValue(), previousValue)
		);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Initialize(notnull IEntity owner)
	{
		super.Initialize(owner);
		
		SetContainerRepresentative(m_ResourceComponent.GetContainer(m_eResourceType));
		
		if (m_ContainerQueue)
			m_ContainerQueue.Initialize(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Clear()
	{
		super.Clear();
		
		if (m_ContainerQueue)
			m_ContainerQueue.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnResourcesChanged(float previousValue)
	{
		super.OnResourcesChanged(previousValue);
		
		if (!m_ContainerRepresentative)
			return;
		
		m_ContainerRepresentative.SetResourceValueUnsafe(GetAggregatedResourceValue());
		
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMaxResourcesChanged(float previousValue)
	{
		super.OnResourcesChanged(previousValue);
		
		if (!m_ContainerRepresentative)
			return;
		
		m_ContainerRepresentative.SetMaxResourceValue(GetAggregatedMaxResourceValue());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnContainerRegistered(notnull SCR_ResourceContainer container)
	{
		super.OnContainerRegistered(container);
		
		container.SetIsEncapsulated(true);
		
		if (!m_ContainerRepresentative)
			return;
		
		if (m_aActions)
		{
			foreach (SCR_ResourceEncapsulatorActionBase action : m_aActions)
			{
				if (action)
					action.PerformAction(m_ContainerRepresentative, container);
			}
		}

		m_ContainerRepresentative.SetMaxResourceValue(GetAggregatedMaxResourceValue());
		m_ContainerRepresentative.SetResourceValueUnsafe(GetAggregatedResourceValue());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnContainerUnregistered(notnull SCR_ResourceContainer container)
	{
		super.OnContainerUnregistered(container);
		
		container.SetIsEncapsulated(false);
		
		if (!m_ContainerRepresentative)
			return;
		
		m_ContainerRepresentative.SetMaxResourceValue(GetAggregatedMaxResourceValue());
		m_ContainerRepresentative.SetResourceValueUnsafe(GetAggregatedResourceValue());
	}
}

class SCR_ResourceEncapsulatorAssimilated : SCR_ResourceEncapsulator
{
	
}

class SCR_ResourceEncapsulatorDelegated : SCR_ResourceEncapsulator
{
	
}
