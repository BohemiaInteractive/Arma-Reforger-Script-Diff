[BaseContainerProps()]
class SCR_ResourceGeneratorActionStore : SCR_ResourceGeneratorActionBase
{
	//------------------------------------------------------------------------------------------------
	override float ComputeGeneratedResources(notnull SCR_ResourceGenerator generator, float resourceValue)
	{
		return Math.Min(resourceValue, generator.GetAggregatedMaxResourceValue() - generator.GetAggregatedResourceValue());
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(notnull SCR_ResourceGenerator generator, inout float resourceValue)
	{
		SCR_ResourceContainer containerTo;
		SCR_ResourceContainerQueueBase containerQueue = generator.GetContainerQueue();
		int containerCount = containerQueue.GetContainerCount();
		
		for (int i = 0; i < containerCount; i++)
		{
			if (resourceValue <= 0)
				break;
			
			containerTo = containerQueue.GetContainerAt(i);
			
			if (!containerTo)
				continue;
			
			float usedResources	= Math.Min(containerTo.ComputeResourceDifference(), resourceValue);
			resourceValue		-= usedResources;
			
			containerTo.IncreaseResourceValue(usedResources);
		}
	}
}
