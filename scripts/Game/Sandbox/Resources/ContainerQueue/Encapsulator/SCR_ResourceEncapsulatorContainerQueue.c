[BaseContainerProps()]
class SCR_ResourceEncapsulatorContainerQueue : SCR_ResourceContainerQueue<SCR_ResourceEncapsulator>
{	
	//------------------------------------------------------------------------------------------------
	//! Registers a container into the queue.
	//! \param container The container to be registered into the queue.
	//! \return Returns the position of the container in the queue.
	override int RegisterContainer(notnull SCR_ResourceContainer container)
	{
		int position = SCR_ResourceContainerQueueBase.INVALID_CONTAINER_INDEX;
		bool shouldIncrementOffset;
		SCR_ResourceContainerStorageQueue<SCR_ResourceEncapsulator> storageQueue;
		
		EResourceContainerStorageType storageType = container.GetStorageType();
		
		foreach (SCR_ResourceStoragePolicyBase<SCR_ResourceEncapsulator> policy: m_StoragePolicies)
		{
			storageQueue = policy.GetStorageQueue();
		
			if (shouldIncrementOffset)
			{
				storageQueue.IncrementOffset(1);
				continue;
			}
			
			if (!policy.IsStorageTypeValid(storageType))
				continue;
			
			array<ref SCR_ResourceEncapsulatorActionBase> actions = m_Interactor.GetActions();
			SCR_ResourceContainer containerRepresentative = m_Interactor.GetContainerRepresentative();

			m_fAggregatedResourceValue		+= container.GetResourceValue();
			m_fAggregatedMaxResourceValue	+= container.GetMaxResourceValue();
			
			shouldIncrementOffset = true;
			position = storageQueue.RegisterContainer(container, m_Interactor);
		}
		
		return position;
	}
	
	//------------------------------------------------------------------------------------------------
	override void DebugDraw()
	{	
		if (!m_Interactor)
			return;
		
		int consumerIdx;
		float heightDiff;
		vector origin;
		Color color1 = Color.FromInt(m_Interactor.GetDebugColor().PackToInt());
		Color color2 = Color.FromInt(m_Interactor.GetDebugColor().PackToInt());
		
		color1.Scale(0.2);
		color1.SetA(1.0);
		color2.Lerp(Color.FromInt(Color.WHITE), 0.0);
		color2.SetA(1.0);
		
		foreach (int idx, SCR_ResourceContainer container: m_aRegisteredContainers)
		{
			if (!container)
				continue;
				
			string infoText = string.Format("   ENC   Pos: %1  Distance: %2 m   ", idx, vector.Distance(m_Interactor.GetOwnerOrigin(), container.GetOwnerOrigin()));
			origin 		= container.GetOwnerOrigin();
			consumerIdx	= container.GetLinkedInteractorIndex(m_Interactor);
			
			if (container.IsResourceGainEnabled())
				consumerIdx++;
			
			if (container.IsResourceDecayEnabled())
				consumerIdx++;
			
			heightDiff = container.debugControlOffset * consumerIdx + container.debugControlOffset * 3;
			
			Shape.CreateArrow(origin + Vector(0.0, heightDiff, 0.0), origin, 0.0, 0xFFFFFFFF, ShapeFlags.ONCE | ShapeFlags.NOZBUFFER);
			
			Shape.CreateArrow(m_Interactor.GetOwnerOrigin(), (origin + Vector(0.0, heightDiff, 0.0)), 0.1, color1.PackToInt(), ShapeFlags.ONCE | ShapeFlags.NOZBUFFER |ShapeFlags.DEPTH_DITHER);
			
			DebugTextWorldSpace.Create(m_Interactor.GetOwner().GetWorld(), infoText, DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA | DebugTextFlags.ONCE, origin[0], origin[1] + heightDiff, origin[2], 10, color1.PackToInt(), color2.PackToInt(), consumerIdx + 1);
			
			if (!container.GetComponent().IsDebugVisualizationEnabled())
				container.DebugDraw(false);
		}
	}
}
