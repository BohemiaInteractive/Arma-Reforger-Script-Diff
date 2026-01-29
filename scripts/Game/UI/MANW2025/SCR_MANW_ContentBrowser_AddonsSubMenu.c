class SCR_MANW_ContentBrowser_AddonsSubMenu : SCR_ContentBrowser_AddonsSubMenu
{
	protected const string MANW_FILTER = "MANW_2025";
	protected const string EXLUCDE_TEST = "TESTDEV";
	
	//------------------------------------------------------------------------------------------------
	// Override to skip DEVTEST
	override protected void PackTagFilterData(SCR_ContentBrowser_GetAssetListParams params, bool defaultValues)
	{
		SCR_FilterSet filterSet = m_Widgets.m_FilterPanelComponent.GetFilter();
		SCR_FilterCategory otherCategory = filterSet.FindFilterCategory("other");
		
		// Make arrays with tags to include or exclude
		array<ref array<string>> tagArraysInclude = {};
		array<string> tagsExclude = {};

		// Tags from type filters
		SCR_FilterCategory manwCategory = filterSet.FindFilterCategory("manw_winners");
		if (manwCategory.GetAnySelected() && !defaultValues)
		{
			array<string> typeTags = {};
			foreach (SCR_FilterEntry filter : manwCategory.GetFilters())
			{
				if (filter.GetSelected(defaultValues))
					typeTags.Insert(filter.m_sInternalName);
			}
			tagArraysInclude.Insert(typeTags);
		}
		
		// Tags from type filters
		SCR_FilterCategory typeCategory = filterSet.FindFilterCategory("type");
		if (!defaultValues && typeCategory.GetAnySelected())
		{
			array<string> typeTags = {};
			foreach (SCR_FilterEntry filter : typeCategory.GetFilters())
			{
				if (filter.GetSelected(defaultValues))
					typeTags.Insert(filter.m_sInternalName);
			}
			tagArraysInclude.Insert(typeTags);
		}
		
		tagArraysInclude.Insert({MANW_FILTER});
		tagsExclude.Insert(EXLUCDE_TEST);

		// Pack the tags
		if (!tagArraysInclude.IsEmpty() || !tagsExclude.IsEmpty())
		{
			params.StartObject("tags");							// <- Start "tags" object

			if (!tagArraysInclude.IsEmpty())
			{
				params.StartArray("include");					// <- Start "include"  array

				foreach (array<string> tagArray : tagArraysInclude)
				{
					params.ItemArray();							// <- Start tags array
					foreach (string tag : tagArray)
					{
						params.ItemString(tag);					// <- Array element
					}
					params.EndArray();							// <- End tags array
				}

				params.EndArray();								// <- End "include" array
			}

			if (!tagsExclude.IsEmpty())
			{
				params.StartArray("exclude");					// <- Start "exclude" array
				foreach (string tag : tagsExclude)
				{
					params.ItemString(tag);						// <- Array element
				}
				params.EndArray();								// <- End "exclude" array
			}

			params.EndObject();									// <- End "tags" object
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// Override to skip reported check
	override void PackGetAssetListParams(SCR_ContentBrowser_GetAssetListParams p, bool defaultValues = false)
	{
		SCR_FilterSet filterSet = m_Widgets.m_FilterPanelComponent.GetFilter();
		SCR_FilterCategory otherCategory = filterSet.FindFilterCategory("other");
		
		// Pack search string
		string searchStr = m_Widgets.m_FilterPanelComponent.GetWidgets().m_FilterSearchComponent.GetValue();
		if (!defaultValues && !searchStr.IsEmpty())
			p.StoreString("search", searchStr);				// <- "search": "..."
		
		// Pack sorting and filters
		PackSortingData(p);
		PackQuickFilterData(p, defaultValues);
		PackTagFilterData(p, defaultValues);
	}
}
