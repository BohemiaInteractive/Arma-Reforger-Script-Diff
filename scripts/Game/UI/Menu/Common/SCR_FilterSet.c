/*
Base classes for filters used in server browser, workshop content browser, scenario browser, etc.
*/

//-----------------------------------------------------------------------------------------------------------
//! Base class for one filter entry
//! If you need extra functionality from it, you can inherit from this class.
[BaseContainerProps(), SCR_FilterEntryTitle()]
class SCR_FilterEntry
{
	[Attribute()]
	string m_sDisplayName;
	
	[Attribute()]
	string m_sInternalName;
	
	[Attribute("false", UIWidgets.CheckBox)]
	bool m_bSelectedAtStart;

	// Main image
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Image - texture or imageset", "edds imageset")]
	ResourceName m_sImageTexture;

	[Attribute("{00FE3DBDFD15227B}UI/Textures/Icons/icons_wrapperUI-glow.imageset", UIWidgets.ResourcePickerThumbnail, "edds imageset")]
	ResourceName m_sGlowTexture;
	
	[Attribute("", UIWidgets.EditBox, "Image name if image set is used")]
	string m_sImageName;
	
	SCR_ListBoxElementComponent m_FilterComponent;
	
	protected bool m_bSelected;
	
	// Pointer back to category of this filter, initialized in constructor of Category class
	protected SCR_FilterCategory m_Category;
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns the category of this filter.
	SCR_FilterCategory GetCategory() { return m_Category; }
	
	//-----------------------------------------------------------------------------------------------------------
	//! Sets the selected flag, doesn't do anything special.
	void SetSelected(bool newValue) { m_bSelected = newValue; }
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns the selected flag.
	bool GetSelected(bool defaultValue = false) 
	{
		if (defaultValue)
			return m_bSelectedAtStart;
		
		return m_bSelected; 
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void Init(SCR_FilterCategory category)
	{
		m_Category = category;
	}
};


//-----------------------------------------------------------------------------------------------------------
//! Class for filter category
//! Each category has its own name, it is an array of individual filter entries.
[BaseContainerProps(), SCR_FilterEntryTitle()]
class SCR_FilterCategory
{
	[Attribute()]
	string m_sDisplayName;
	
	[Attribute()]
	string m_sInternalName;
	
	[Attribute("false", UIWidgets.Auto, "Only one filter can be enabled at a time.")]
	bool m_bMutuallyExclusive;
	
	[Attribute("true", UIWidgets.Auto, "When false, the filter panel will prohibit deselection of last selected entry.")];
	bool m_bAllowNothingSelected;
	
	[Attribute(uiwidget: UIWidgets.SearchComboBox, desc:"Category won't be created on selected platforms", enums: ParamEnumArray.FromEnum(EPlatform))]
	protected ref array<EPlatform> m_aHideOnPlatforms;
	
	//[Attribute("false", UIWidgets.Auto, "The category is hidden in release build.")]
	//bool m_bHiddenInReleaseBuild;
	
	//[Attribute("0", UIWidgets.ComboBox, "Type", "", ParamEnumArray.FromEnum(EContentBrowserFilterType) )]
	//EContentBrowserFilterType  m_eFilterType;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Common image set for individual filters. Each filter can override it though.", "imageset")]
	ResourceName m_sFilterImageSet;
	
	[Attribute("{00FE3DBDFD15227B}UI/Textures/Icons/icons_wrapperUI-glow.imageset", UIWidgets.ResourcePickerThumbnail, "imageset")]
	ResourceName m_sFilterGlowImageSet;
	
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref SCR_FilterEntry> m_aFilters;
	
	Widget m_wCategoryTitleWidget;
	
	
	//-----------------------------------------------------------------------------------------------------------
	protected void SCR_FilterCategory()
	{
		// Register the back-pointer of all child filters
		
		// !! This object is instantiated as a property of another scripted object,
		// So when constructor is called, all objects of m_aFilters are already created by the game
		
		foreach (auto f : m_aFilters)
			f.Init(this);		
	}
	
	// ------------ Public API ----------------
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns array with filter entries.
	array<ref SCR_FilterEntry> GetFilters()
	{
		return m_aFilters;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Finds a filter by internal name
	SCR_FilterEntry FindFilter(string internalName)
	{
		foreach (SCR_FilterEntry e : m_aFilters)
		{
			if (e.m_sInternalName == internalName)
				return e;
		}
		
		return null;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	bool GetAllSelected()
	{
		foreach (auto f : m_aFilters)
		{
			if (!f.GetSelected())
				return false;
		}
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	bool GetAnySelected()
	{
		foreach (auto f : m_aFilters)
		{
			if (f.GetSelected())
				return true;
		}
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	bool GetAllDeselected()
	{
		foreach (auto f : m_aFilters)
		{
			if (f.GetSelected())
				return false;
		}
		return true;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns count of selected entries
	int GetSelectedCount()
	{
		int n = 0;
		foreach (auto f : m_aFilters)
		{
			if (f.GetSelected())
				n++;
		}
		return n;
	}
	
	//-----------------------------------------------------------------------------------------------------------	
	void GetHideOnPlaforms(out array<EPlatform> platforms)
	{
		platforms.Copy(m_aHideOnPlatforms);
	}
};



//-----------------------------------------------------------------------------------------------------------
//! Config class for whole filter set.
//! Filter set is an array of filter categories.
//! Inherit from it if you need to store some more data - don't forget to mark it with BaseContainerProps attribute too!
[BaseContainerProps(configRoot : true)]
class SCR_FilterSet
{
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref SCR_FilterCategory> m_aFilterCategories;
	
	//-----------------------------------------------------------------------------------------------------------
	array<ref SCR_FilterCategory> GetFilterCategories()
	{
		return m_aFilterCategories;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Finds a filter category by its internal name.
	SCR_FilterCategory FindFilterCategory(string internalName)
	{
		foreach (SCR_FilterCategory cat : m_aFilterCategories)
		{
			if (cat.m_sInternalName == internalName)
				return cat;
		}
		return null;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Finds a filter entry within categories 
	ref SCR_FilterEntry FindFilter(string internalName)
	{
		foreach (SCR_FilterCategory cat : m_aFilterCategories)
		{
			SCR_FilterEntry filter = cat.FindFilter(internalName);
			if (filter)
				return filter;
		}
		return null;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	bool AnyFilterSelected()
	{
		bool selected;
		foreach (SCR_FilterCategory category : m_aFilterCategories)
		{
			selected = category.GetAnySelected();
			if (selected)
				return true;
		}
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Initializes all filters to their default values
	void ResetToDefaultValues()
	{
		// Select filters which must be selected at start according to SelectedAtStart flag
		foreach (SCR_FilterCategory category : m_aFilterCategories)
		{
			foreach (SCR_FilterEntry filter : category.GetFilters())
				filter.SetSelected(filter.m_bSelectedAtStart);
		}
	}
};

//------------------------------------------------------------------------------------------------
//! Set filter title in config by internal name for better readability
class SCR_FilterEntryTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{		
		source.Get("m_sInternalName", title);
		return true;
	}
};


// ----------------- Saving / Loading --------------------

//------------------------------------------------------------------------------------------------
//! Storage of one filter set
[BaseContainerProps()]
class SCR_FilterSetStorage
{
	const int CURRENT_VERSION = 1;
	
	[Attribute()]
	string m_sTag;							// Filter sets are identified by this tag, tag should be unique
	
	[Attribute()]
	ref array<string> m_aSelectedFilters;	// Array of selected filters
	
	[Attribute()]
	ref array<string> m_aSelectedCategories;// Categories of those selected filters

	[Attribute()]
	int m_iVersion;							// Version. In the future filters will change, we will have to detect old filters.
};

//------------------------------------------------------------------------------------------------
class SCR_AllFilterSetsStorage : ModuleGameSettings
{
	[Attribute()]
	ref array<ref SCR_FilterSetStorage> m_aFilterSets;
	
	
	//------------------------------------------------------------------------------------------------
	static void SaveFilterSet(string tag, SCR_FilterSet filterSet)
	{
		SCR_AllFilterSetsStorage allFilterSets = new SCR_AllFilterSetsStorage;
		BaseContainer allFilterSetsContainer = GetGame().GetGameUserSettings().GetModule("SCR_AllFilterSetsStorage");
		BaseContainerTools.WriteToInstance(allFilterSets, allFilterSetsContainer);
		
		// Delete the old data for this tag, if found
		for (int i = allFilterSets.m_aFilterSets.Count()-1; i >= 0; i--)
		{
			SCR_FilterSetStorage storedFilterSet = allFilterSets.m_aFilterSets[i];
			if (storedFilterSet.m_sTag == tag)
			{
				allFilterSets.m_aFilterSets.Remove(i);
			}
		}
		
		// Create a new entry for this filter set
		SCR_FilterSetStorage filterSetStorage = new SCR_FilterSetStorage;
		filterSetStorage.m_iVersion = SCR_FilterSetStorage.CURRENT_VERSION;
		filterSetStorage.m_sTag = tag;
		if (!filterSetStorage.m_aSelectedFilters)
			filterSetStorage.m_aSelectedFilters = new array<string>;
		if (!filterSetStorage.m_aSelectedCategories)
			filterSetStorage.m_aSelectedCategories = new array<string>;
		
		foreach (SCR_FilterCategory category : filterSet.GetFilterCategories())
		{
			foreach (SCR_FilterEntry filter : category.GetFilters())
			{
				// Save filters 
				if (filter.GetSelected() && !filter.m_sInternalName.IsEmpty() && !category.m_sInternalName.IsEmpty() )
				{
					filterSetStorage.m_aSelectedCategories.Insert(category.m_sInternalName);
					filterSetStorage.m_aSelectedFilters.Insert(filter.m_sInternalName);
				}
			}
		}
		
		allFilterSets.m_aFilterSets.Insert(filterSetStorage);
		
		BaseContainerTools.ReadFromInstance(allFilterSets, allFilterSetsContainer);
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	bool IsFilterSetSaved(string tag)
	{
		SCR_AllFilterSetsStorage allFilterSets = new SCR_AllFilterSetsStorage;
		BaseContainer allFilterSetsContainer = GetGame().GetGameUserSettings().GetModule("SCR_AllFilterSetsStorage");
		BaseContainerTools.WriteToInstance(allFilterSets, allFilterSetsContainer);
		
		// Find the old data for this tag
		SCR_FilterSetStorage storedFilterSet;
		for (int i = 0; i < allFilterSets.m_aFilterSets.Count(); i++)
		{
			SCR_FilterSetStorage iFilterSet = allFilterSets.m_aFilterSets[i];
			if (iFilterSet.m_sTag == tag)
				return true;
		}
		
		return false;
	}
	
	
	
	//------------------------------------------------------------------------------------------------
	static bool TryLoadFilterSet(string tag, SCR_FilterSet filterSet)
	{
		SCR_AllFilterSetsStorage allFilterSets = new SCR_AllFilterSetsStorage;
		BaseContainer allFilterSetsContainer = GetGame().GetGameUserSettings().GetModule("SCR_AllFilterSetsStorage");
		BaseContainerTools.WriteToInstance(allFilterSets, allFilterSetsContainer);

		// Find the old data for this tag
		SCR_FilterSetStorage storedFilterSet;
		for (int i = 0; i < allFilterSets.m_aFilterSets.Count(); i++)
		{
			SCR_FilterSetStorage iFilterSet = allFilterSets.m_aFilterSets[i];
			if (iFilterSet.m_sTag == tag)
			{
				storedFilterSet = iFilterSet;
				break;
			}
		}
		
		// Bail if not found
		if (!storedFilterSet)
			return false;
		
		// Bail if version is invalid
		if (storedFilterSet.m_iVersion > SCR_FilterSetStorage.CURRENT_VERSION)
		{
			return false;
		}
		
		// Restore selected filters
		
		// Clear up first
		foreach (SCR_FilterCategory category : filterSet.GetFilterCategories())
		{
			foreach (SCR_FilterEntry filter : category.GetFilters())
			{
				filter.SetSelected(false);
			}
		}
		
		// Select filters which are selected in saved data
		for (int i = 0; i < storedFilterSet.m_aSelectedFilters.Count(); i++)
		{
			string filterName = storedFilterSet.m_aSelectedFilters[i];
			string categoryName = storedFilterSet.m_aSelectedCategories[i];
			
			SCR_FilterCategory category = filterSet.FindFilterCategory(categoryName);
			
			if (category)
			{
				SCR_FilterEntry filter = category.FindFilter(filterName);
				if (filter)
					filter.SetSelected(true);
			}
		}
		
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Set all saved filter values to default
	static void ResetAllToDefault()
	{
		// Find storage
		SCR_AllFilterSetsStorage allFilterSets = new SCR_AllFilterSetsStorage;
		allFilterSets.m_aFilterSets = {};
		BaseContainer allFilterSetsContainer = GetGame().GetGameUserSettings().GetModule("SCR_AllFilterSetsStorage");

		// Save settings
		BaseContainerTools.ReadFromInstance(allFilterSets, allFilterSetsContainer);
		GetGame().UserSettingsChanged();
		GetGame().SaveUserSettings();
	}
};