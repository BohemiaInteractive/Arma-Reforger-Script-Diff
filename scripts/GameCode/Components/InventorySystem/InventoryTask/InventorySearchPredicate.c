class InventorySearchPredicate: BaseInventorySearchPredicate
{
	// Fill this array with required GenericComponent types
	ref array<typename> QueryComponentTypes = {};
	// Fill this array with required BaseItemAttributeData types
	ref array<typename> QueryAttributeTypes = {};
	
	override protected sealed ref array<typename> GetQueryComponents()
	{
		if (QueryComponentTypes.Count() == 0)
			return null;
		return QueryComponentTypes;
	}
	override protected sealed ref array<typename> GetQueryAttributes()
	{
		if (QueryAttributeTypes.Count() == 0)
			return null;
		return QueryAttributeTypes;
	}
};
