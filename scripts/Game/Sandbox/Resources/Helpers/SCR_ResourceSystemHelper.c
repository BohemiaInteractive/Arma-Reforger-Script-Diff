class SCR_ResourceSystemHelper
{
	//~ How many decimals are displayed when showing supplies. Use SCR_ResourceSystemHelper.SuppliesToString(supplies) to ignore any decimals ending in 0
	static const int DECIMALS_SUPPLIES = 1;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] resourceType Resource type
	//! \return If supplies are enabled or not
	static bool IsGlobalResourceTypeEnabled(EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_BaseGameMode baseGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!baseGameMode)
			return true;
		
		return baseGameMode.IsResourceTypeEnabled(resourceType);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! Get the amount of resources stored in the resource component
	//! \param[in] resourceComponent ResourceComponent to get stored resources from
	//! \param[out] storedRescources How many resources were found
	//! \param[in] resourceType Type of resources to find
	//! \return Returns true if resource storage were found (even if stored resources is 0) returns false if no valid ResourceComponent or ResourceComponent setup
	//!
	static bool GetStoredResources(notnull SCR_ResourceComponent resourceComponent, out float storedRescources, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_ResourceConsumer consumer; 
		storedRescources = 0;
		
		if (resourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, resourceType, consumer))
		{
			storedRescources = consumer.GetAggregatedResourceValue();
			return true;
		}
		
		SCR_ResourceGenerator generator;
		if (resourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, resourceType, generator))
		{
			storedRescources = generator.GetAggregatedResourceValue();
			return true;
		}
		
		if (resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, resourceType, consumer))
		{
			storedRescources = consumer.GetAggregatedResourceValue();
			return true;
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! Call this to consume resources
	//! \param[in] resourceComponent ResourceComponent to get consume resources from
	//! \param[in] resourcesToConsume How much needs to be consumed
	//! \param[in] failIfNotEnoughResources If true it will never remove resources if there aren't enough. If false it will remove either the given amount or, if not enough resources, it will remove all the resources it can
	//! \param[in] resourceType Type of resources to find
	//! \return EResourceReason, if it succeeded or not and the reason why
	//!
	static EResourceReason ConsumeResources(notnull SCR_ResourceComponent resourceComponent, float resourcesToConsume, bool failIfNotEnoughResources, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		//~ If it does not fail if there are not enough resources than make sure to remove the max amount possible
		if (!failIfNotEnoughResources)
		{
			float available;
			GetAvailableResources(resourceComponent, available, resourceType);
			
			if (available < resourcesToConsume)
				resourcesToConsume = available;
		}

		SCR_ResourceConsumer consumer = GetAvailableResourceConsumer(resourceComponent, resourceType);
		if (!consumer)
			return EResourceReason.UNAVAILABLE;
		
		return consumer.RequestConsumtion(resourcesToConsume);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! Function to unify the way supplies are displayed in UI
	//! \param[in] supplies Supplies float to be converted to string
	//! \return Float converted to supplies with the correct decimals
	//!
	static string SuppliesToString(float supplies)
	{
		return SCR_FormatHelper.FloatToStringNoZeroDecimalEndings(supplies, DECIMALS_SUPPLIES);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! Get the amount of resources stored as well as the max amount that can be stored in the resource component
	//! \param[in] resourceComponent ResourceComponent to get stored resources from
	//! \param[out] Current stored resources (0 if no valid resources are found)
	//! \param[out] Max stored resources (0 if no valid resources are found)
	//! \param[in] resourceType Type of resources to find
	//! \return Returns true if resource storage were found (even if stored resources is 0) returns false if no valid ResourceComponent or ResourceComponent setup or if max resource amount is 0
	//!
	static bool GetStoredAndMaxResources(notnull SCR_ResourceComponent resourceComponent, out float totalResources, out float maxResources, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_ResourceConsumer consumer; 
		
		totalResources = 0;
		maxResources = 0;
			
		if (resourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, resourceType, consumer))
		{
			maxResources = consumer.GetAggregatedMaxResourceValue();
			totalResources = consumer.GetAggregatedResourceValue();
		
			if (maxResources > 0)
				return true;
		}
		
		SCR_ResourceGenerator generator;
		if (resourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, resourceType, generator))
		{
			maxResources = generator.GetAggregatedMaxResourceValue();
			totalResources = generator.GetAggregatedResourceValue();
			
			if (maxResources > 0)
				return true;
		}
		
		if (resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, resourceType, consumer))
		{
			maxResources = consumer.GetAggregatedMaxResourceValue();
			totalResources = consumer.GetAggregatedResourceValue();
			
			if (maxResources > 0)
				return true;
		}
		
		//~ Reset again
		totalResources = 0;
		maxResources = 0;

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! Get the amount of resources that are available to use for the given resource component
	//! \param[in] resourceComponent ResourceComponent to get available resources from
	//! \param[out] availableResources How many resources are available
	//! \param[in] resourceID Type of resources it checks. By default this is DEFAULT
	//! \param[in] resourceType Type of resources to find
	//! \return Returns true if availabe resources were found (even if available resources is 0) returns false if no valid ResourceComponent or ResourceComponent setup
	//!
	static bool GetAvailableResources(notnull SCR_ResourceComponent resourceComponent, out float availableResources, EResourceGeneratorID resourceID = EResourceGeneratorID.DEFAULT, EResourceType resourceType = EResourceType.SUPPLIES)
	{		
		SCR_ResourceConsumer consumer;
		availableResources = 0;
		
		if (resourceComponent.GetConsumer(resourceID, resourceType, consumer))
		{
			availableResources = consumer.GetAggregatedResourceValue();
			return true;
		}
			
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! Get the storage consumer from the given ResourceCompoment
	//! \param[in] resourceComponent ResourceComponent to get consumer from
	//! \param[in] resourceType Type of resources the consumer has
	//! \return Found consumer
	//!
	static SCR_ResourceConsumer GetStorageConsumer(notnull SCR_ResourceComponent resourceComponent, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, resourceType);
		if (consumer)
			return consumer;
		
		consumer = resourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, resourceType);
		if (consumer)
			return consumer;
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! Get the consumer for that will grant you access to the available resources
	//! \param[in] resourceComponent ResourceComponent to get consumer from
	//! \param[in] resourceType Type of resources the consumer has
	//! \return Found consumer
	//!
	static SCR_ResourceConsumer GetAvailableResourceConsumer(notnull SCR_ResourceComponent resourceComponent, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		return resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, resourceType);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! Get the first valid consumer that could be found
	//! \param[in] resourceComponent ResourceComponent to get consumer from
	//! \param[in] resourceType Type of resources the consumer has
	//! \return Found consumer
	//!
	static SCR_ResourceConsumer GetFirstValidConsumer(notnull SCR_ResourceComponent resourceComponent, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_ResourceConsumer consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, resourceType);
		if (consumer)
			return consumer;
		
		consumer = resourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, resourceType);
		if (consumer)
			return consumer;
		
		consumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, resourceType);
		if (consumer)
			return consumer;
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! Get the first valid generator that could be found
	//! \param[in] resourceComponent ResourceComponent to get generator from
	//! \param[in] resourceType Type of resources the generator has
	//! \return Found generator
	//!
	static SCR_ResourceGenerator GetFirstValidGenerator(notnull SCR_ResourceComponent resourceComponent, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_ResourceGenerator generator = resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT_STORAGE, resourceType);
		if (generator)
			return generator;
		
		generator = resourceComponent.GetGenerator(EResourceGeneratorID.VEHICLE_LOAD, resourceType);
		if (generator)
			return generator;
		
		generator = resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, resourceType);
		if (generator)
			return generator;
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Static function to make sure that all refunds are treated the same way
	//! return Correctly rounded refund amount
	static float RoundRefundSupplyAmount(float refundCost)
	{
		return Math.Ceil(refundCost);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Do simple transfer of resources between two components
	//! \param[in] resourceComponentFrom ResourceComponent from where resources should be taken
	//! \param[in] resourceComponentTo ResourceComponent to where resources should be transfered
	//! \param[in] transferAmount Amound of resources to be transfered
	//! \param[in] allowPartialTransfer Allow transfering only partial amount of desired resources, should there be less resources, or space in target ResourceComponent
	//! \param[in] resourceType Type of resources to be transfered
	static EResourceReason SimpleResourceTransfer(notnull SCR_ResourceComponent resourceComponentFrom, notnull SCR_ResourceComponent resourceComponentTo, float transferAmount, bool allowPartialTransfer = true, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_ResourceConsumer resourceConsumerFrom = GetFirstValidConsumer(resourceComponentFrom, resourceType);
		if (!resourceConsumerFrom)
			return EResourceReason.UNAVAILABLE;
		
		SCR_ResourceGenerator resourceGeneratorTo = GetFirstValidGenerator(resourceComponentTo, resourceType);
		if (!resourceGeneratorTo)
			return EResourceReason.UNAVAILABLE;
		
		float availableResources = resourceConsumerFrom.GetAggregatedResourceValue();
		float actualTransferAmount = transferAmount;
		if (actualTransferAmount > availableResources)
		{
			if (!allowPartialTransfer)
				return EResourceReason.INSUFICIENT;
			
			actualTransferAmount = transferAmount - availableResources;
		}
		
		float availableStorage = resourceGeneratorTo.GetAggregatedMaxResourceValue() - resourceGeneratorTo.GetAggregatedResourceValue();
		if (availableStorage < actualTransferAmount)
		{
			if (!allowPartialTransfer)
				return EResourceReason.INSUFICIENT;
			
			actualTransferAmount = availableStorage;
		}

		resourceConsumerFrom.RequestConsumtion(actualTransferAmount);
		resourceGeneratorTo.RequestGeneration(actualTransferAmount);
		return EResourceReason.SUFFICIENT;
	}
}
