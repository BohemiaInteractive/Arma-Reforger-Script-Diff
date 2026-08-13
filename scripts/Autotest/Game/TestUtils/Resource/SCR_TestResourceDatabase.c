class SCR_TestResourceDatabase
{
	//------------------------------------------------------------------------------------------------
	//! Searches for resources in specified path.
	//! \param[in] path
	//! \param[in] extensions
	//! \param[in] ignoredTypes
	//! \param[in] allowedTypes
	//! \return Set of resurce names matching the criteria
	static set<ResourceName> Search(
		string path,
		array<string> extensions = null,
		array<string> tags = null,
		array<typename> ignoredTypes = null,
		array<typename> allowedTypes = null
	) {

		SearchResourcesFilter filter = new SearchResourcesFilter();
		filter.rootPath = path;
		filter.fileExtensions = extensions;
		filter.tags = tags;

		set<ResourceName> tempResult = new set<ResourceName>();
		ResourceDatabase.SearchResources(filter, tempResult.Insert);

		// remove unregistered entries
		tempResult.RemoveItem(ResourceName.Empty);

		// these filters are bit slow but should be fine as it's called infrequently
		if (ignoredTypes)
			FilterIgnoredTypes(tempResult, ignoredTypes);
		// TODO consolidate both filters into single method to speed it up
		if (allowedTypes)
			FilterAllowedTypes(tempResult, allowedTypes);

		return tempResult;
	}

	//------------------------------------------------------------------------------------------------
	protected static void FilterIgnoredTypes(notnull set<ResourceName> resourcesSet, notnull array<typename> ignoredTypes)
	{
		array<ResourceName> resourcesToRemove = {};
		foreach (ResourceName resourceName : resourcesSet)
		{
			foreach (typename ignoredType : ignoredTypes)
			{
				Resource resource = Resource.Load(resourceName);
				typename resourceClassName = resource.GetResource().ToBaseContainer().GetClassName().ToType();

				if (resourceClassName.IsInherited(ignoredType))
					resourcesToRemove.Insert(resourceName);
			}
		}

		foreach (ResourceName resourceName : resourcesToRemove)
		{
			resourcesSet.RemoveItem(resourceName);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected static void FilterAllowedTypes(notnull set<ResourceName> resourcesSet, notnull array<typename> allowedTypes)
	{
	    array<ResourceName> resourcesToRemove = {};
	
	    foreach (ResourceName resourceName : resourcesSet)
	    {
	        Resource resource = Resource.Load(resourceName);
	        typename resourceClassName = resource.GetResource().ToBaseContainer().GetClassName().ToType();
	
	        bool matchesAny = false;
	        foreach (typename allowedType : allowedTypes)
	        {
	            if (resourceClassName.IsInherited(allowedType))
	            {
	                matchesAny = true;
	                break; 									// one match is enough (OR)
	            }
	        }
	
	        if (!matchesAny)
	            resourcesToRemove.Insert(resourceName);	// remove only if it matched NONE
	    }
	
	    foreach (ResourceName resourceName : resourcesToRemove)
		{
	        resourcesSet.RemoveItem(resourceName);
		}
	}
}