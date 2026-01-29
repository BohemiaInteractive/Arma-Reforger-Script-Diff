[BaseContainerProps()]
class SCR_ResourceConsumerContainerStorageQueueExtended : SCR_ResourceConsumerContainerStorageQueue
{
	//------------------------------------------------------------------------------------------------
	override bool ShouldContainerChangeCauseUpdate()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override int ComputePosition(SCR_ResourceContainer container, SCR_ResourceConsumer actor)
	{
		int containerComparePosition;
		float containerCompareResource;
		float containerCompareMaxResource;
		float containerCompareNorm;
		SCR_ResourceContainer containerCompare;
		int position				= m_iOffsetPosition;
		int maxPosition				= GetUpperOffsetPosition();
		float containerResource		= container.GetResourceValue();
		
		//! Empty containers at the last of the policy queue saves performance on consumption.
		if (containerResource <= SCR_ResourceActor.RESOURCES_LOWER_LIMIT)
			return maxPosition - m_iOffsetPosition;
		
		float containerMaxResource	= container.GetMaxResourceValue();
		float containerNorm			= vector.DistanceSq(actor.GetOwnerOrigin(), container.GetOwnerOrigin());
		
		while (position < maxPosition)
		{
			containerComparePosition	= position + ((maxPosition - position) >> 1);
			containerCompare			= GetContainerAt(containerComparePosition);
			
			if (!containerCompare)
			{
				maxPosition = containerComparePosition;
				continue;
			}
			
			containerCompareResource	= containerCompare.GetResourceValue();
			containerCompareMaxResource	= containerCompare.GetMaxResourceValue();
			containerCompareNorm		= vector.DistanceSq(actor.GetOwnerOrigin(), containerCompare.GetOwnerOrigin());
			
			if (containerNorm > containerCompareNorm)
				position = containerComparePosition + 1;
			
			else if (containerNorm < containerCompareNorm)
				maxPosition = containerComparePosition;
			
			else if (containerResource > containerCompareResource)
				position = containerComparePosition + 1;
			
			else if (containerResource < containerCompareResource)
				maxPosition = containerComparePosition;
			
			else if (containerMaxResource > containerCompareMaxResource)
				position = containerComparePosition + 1;
			
			else if (containerMaxResource < containerCompareMaxResource)
				maxPosition = containerComparePosition;
			
			else 
				break;
		}
		
		return position - m_iOffsetPosition;
	}
};
