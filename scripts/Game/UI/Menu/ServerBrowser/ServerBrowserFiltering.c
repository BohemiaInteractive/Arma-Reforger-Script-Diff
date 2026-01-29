/*
File for server browser search filtering parameters class.
*/

//------------------------------------------------------------------------------------------------ 
//! Server search filtering
class FilteredServerParams : RoomFilterBase
{
	// Constant filter parameters 
	const int RECENT_SERVERS_SECONDS = 60 * 60 * 24 * 7;
	
	// Filter names 
	const string FILTER_FAVORITES = "favorites";
	const string FILTER_RECENT_SERVERS = "oldestJoinInSeconds";
	const string FILTER_MODDED = "modded";
	const string FILTER_OFFICIAL = "official";
	const string FILTER_COMMUNITY = "community";
	const string FILTER_CROSS_PLAY = "gameClientFilter";
	const string FILTER_VERSION = "gameVersion";
	
	const string SORT_ASCENDENT = "ascendent";
	const string SORT_NAME = "SessionName";
	const string SORT_PING = "Ping";
	
	// Categories 
	const string CATEGORY_PLAYERS = "Players";
	const string CATEGORY_VERSION = "Version";
	
	// Filter values 
	const string VALUE_PLAYERS_MIN = "minPlayersPercent";
	const string VALUE_PLAYERS_MAX = "maxPlayersPercent";
	const string VALUE_PLAYERS_MIN_COUNT = "minPlayersCount";
	const string VALUE_PLAYERS_MAX_COUNT = "maxPlayersCount";
	
	// Filters	
	protected ref SCR_FilterSet m_Filter;
	protected ref array<SCR_FilterEntryRoom> m_aFiltersUncategorized = {};
	protected ref array<ref SCR_FilterEntryRoom> m_aDefaultFilters = {};

	protected ref array<string> modIds = new array<string>;

	// Direct search
	protected string text = "";
	
	protected string hostAddress = "";
	protected string directJoinCode = "";
	
	// Scenario filter 
	protected string scenarioId = "";
	protected string hostedScenarioModId = "";
	
	// Sorting order
	protected string order = SORT_NAME;
	
	// Tabs filter
	protected bool m_bOfficialOn = false;
	protected bool m_bCommunityOn = false;
	
	protected bool m_bFavoriteFilterOn = false;
	protected bool m_bRecentlyPlayedOn = false;
	
	// Mutually exclusive filters
	protected bool m_bModdedFilterSelected;
	protected bool m_bCrossPlayFilterSelected;
	
	// Custom filter specific 
	protected bool m_bUsePlayerLimit = true;
	
	//--------------------------------------------
	// Overrided filter functions 
	//--------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void OnPack()
	{	
		SCR_FilterEntryRoom official = FindFilterByInternalName(m_aDefaultFilters, FILTER_OFFICIAL);
		SCR_FilterEntryRoom community = FindFilterByInternalName(m_aDefaultFilters, FILTER_COMMUNITY);

		// Setup tab filters
		official.SetSelected(m_bOfficialOn);
		community.SetSelected(m_bCommunityOn);
		 
		SCR_FilterEntryRoom favorite = FindFilterByInternalName(m_aDefaultFilters, FILTER_FAVORITES);
		favorite.SetSelected(m_bFavoriteFilterOn);
		
		SCR_FilterEntryRoom recentlyPlayed = FindFilterByInternalName(m_aDefaultFilters, FILTER_RECENT_SERVERS);
		recentlyPlayed.SetSelected(m_bRecentlyPlayedOn);
		
		// Modded
		m_bModdedFilterSelected = false;
		m_bCrossPlayFilterSelected = false;
		
		// Search bar
		StoreString("text", text);

		// Handle players filters 
		if (m_bUsePlayerLimit)
		{
			//int min, max, minCount, maxCount = -1;
			int min = -1;
			int max = -1;
			int minCount = -1;
			int maxCount = -1;

			FormatePlayersFilters(min, max, minCount, maxCount);
		
			if (min != -1)
				StoreInteger(VALUE_PLAYERS_MIN, min);
			if (max != -1)
				StoreInteger(VALUE_PLAYERS_MAX, max);
			if (minCount != -1)
				StoreInteger(VALUE_PLAYERS_MIN_COUNT, minCount);
			if (maxCount != -1)
				StoreInteger(VALUE_PLAYERS_MAX_COUNT, maxCount);
		}
		
		// Game version
		// Works together with "versionMatch"
		SCR_FilterCategory version;
		if (m_Filter)
			version = m_Filter.FindFilterCategory(CATEGORY_VERSION);
		
		if (version && version.GetAnySelected())
			StoreString(FILTER_VERSION, GetGame().GetBuildVersion());
		
		// Register filters
		foreach (SCR_FilterEntryRoom filter : m_aFiltersUncategorized)
		{		
			if (!filter)
				continue;
			
			// Store boolean of selected 
			if (filter.GetSelected())
			{			
				// Separated handling for players 
				if (m_Filter && filter.GetCategory() == m_Filter.FindFilterCategory(CATEGORY_PLAYERS))
					continue;
				
				ActivateFilterValues(filter);
			}
		}
		
		if (!m_bCrossPlayFilterSelected)
			StoreString(FILTER_CROSS_PLAY, "AnyCompatible");
		
		// Set search by mod id 
		if (modIds.IsEmpty())
			UnregV("modIds");
		else
			RegV("modIds");
	}
	
	//--------------------------------------------
	// Public functions 
	//--------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void AddModId(string sId)
	{
		modIds.Insert(sId);
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearModIds()
	{
		modIds.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup host address param and clear join code 
	void SetHostAddress(string address) 
	{ 
		hostAddress = address; 
		directJoinCode = string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup join code param and clear host address 
	void SetJoinCode(string joinCode) 
	{ 
		directJoinCode = joinCode;
		hostAddress = string.Empty;	 
	}
	
	//------------------------------------------------------------------------------------------------
	void FilteredServerParams()
	{	
		includePing = true;
		
		// Apply filter 
		RegV("directJoinCode");
		
		RegV("hostAddress");
		
		// Sort 
		RegV("order");
		
		// Room content 
		RegV("modIds");
		RegV("scenarioId");
		
		RegV("includePing");
		
		// Sort ascendence 
		SCR_FilterEntryRoom asc = new SCR_FilterEntryRoom;
		asc.m_sInternalName = SORT_ASCENDENT;
		asc.AddFilterValue(SORT_ASCENDENT, "1", EFilterType.TYPE_BOOL);
		asc.SetSelected(true);
		m_aDefaultFilters.Insert(asc);
		
		// Official-community 
		SCR_FilterEntryRoom filterOfficial = new SCR_FilterEntryRoom;
		filterOfficial.m_sInternalName = FILTER_OFFICIAL;
		filterOfficial.AddFilterValue(FILTER_OFFICIAL, "1", EFilterType.TYPE_BOOL);
		filterOfficial.SetSelected(false);
		m_aDefaultFilters.Insert(filterOfficial);
		
		SCR_FilterEntryRoom filterCommunity = new SCR_FilterEntryRoom;
		filterCommunity.m_sInternalName = FILTER_COMMUNITY;
		filterCommunity.AddFilterValue(FILTER_OFFICIAL, "0", EFilterType.TYPE_BOOL);
		filterCommunity.SetSelected(false);
		m_aDefaultFilters.Insert(filterCommunity);
		
		// Favorite 
		SCR_FilterEntryRoom filterFavorite = new SCR_FilterEntryRoom;
		filterFavorite.m_sInternalName = FILTER_FAVORITES;
		filterFavorite.AddFilterValue(FILTER_FAVORITES, "1", EFilterType.TYPE_BOOL);
		filterFavorite.SetSelected(true);
		m_aDefaultFilters.Insert(filterFavorite);
		
		// Recent servers 
		SCR_FilterEntryRoom filterRecent = new SCR_FilterEntryRoom;
		filterRecent.m_sInternalName = FILTER_RECENT_SERVERS;
		filterRecent.AddFilterValue(FILTER_RECENT_SERVERS, RECENT_SERVERS_SECONDS.ToString(), EFilterType.TYPE_INT);
		filterRecent.SetSelected(true);
		m_aDefaultFilters.Insert(filterRecent);
		
		// Add default filters 
		foreach (SCR_FilterEntryRoom filter : m_aDefaultFilters)
			m_aFiltersUncategorized.Insert(filter);
	}
	
	//--------------------------------------------
	// Protected functions 
	//--------------------------------------------

	//-----------------------------------------------------------------------------------------------------------
	//! Return filter with given internal name. Find filter within categories
	protected SCR_FilterEntryRoom FindFilterByInternalName(array<ref SCR_FilterEntryRoom> filters, string internalName)
	{
		if (!filters || filters.IsEmpty())
			return null;
		
		// Go throught filtres 
		foreach (SCR_FilterEntryRoom roomFilter : filters)
		{
			// check name and value 
			if (roomFilter.m_sInternalName == internalName) 
				return roomFilter;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Store all velues in filter array
	protected void ActivateFilterValues(SCR_FilterEntryRoom filter)
	{
		// Check filter 
		if (!filter)
			return;
		
		// Check values 
		array<ref SCR_FilterEntryRoomValue> values = filter.GetValues();
		
		if (!values || values.IsEmpty())
			return;
		
		// Go through values 
		foreach (SCR_FilterEntryRoomValue value : values)
		{
			// Values 
			string name = value.GetName();
			
			// Track modded filter 
			if (name == FILTER_MODDED && value.GetBoolValue() == true)
				m_bModdedFilterSelected = filter.GetSelected();
			
			// Track cross play filter 
			if (name == FILTER_CROSS_PLAY)
				m_bCrossPlayFilterSelected = filter.GetSelected();
			
			// Check type 
			EFilterType filterType = value.GetType();
				
			switch(filterType)
			{
				// Booleans
				case EFilterType.TYPE_BOOL:
				StoreBoolean(name, value.GetBoolValue());
				break;
				
				// Integers 
				case EFilterType.TYPE_INT:
				StoreInteger(name, value.GetIntNumberValue());
				break;
				
				// Floating decimals 
				case EFilterType.TYPE_FLOAT:
				StoreFloat(name, value.GetFloatNumberValue());
				break;
				
				// Strings
				case EFilterType.TYPE_STRING:
				StoreString(name, value.GetStringValue());
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Agregate combination of player filter to gain player count range 
	protected void FormatePlayersFilters(out int min, out int max, out int minCount, out int maxCount)
	{
		if (!m_Filter)
			return;
		
		// Get players 
		SCR_FilterCategory playersCat = m_Filter.FindFilterCategory(CATEGORY_PLAYERS);
		if (!playersCat)
			return;
		
		// Checkc filters 
		array<ref SCR_FilterEntry> filters = playersCat.GetFilters();
		if(filters.IsEmpty())
			return;
		
		// Default range values 
		min = -1;
		max = -1;
		minCount = -1;
		maxCount = -1;
		
		for (int i = 0, count = filters.Count(); i < count; i++)
		{
			// Active?
			if (!filters[i].GetSelected())
				continue;
			
			SCR_FilterEntryRoom roomFilter = SCR_FilterEntryRoom.Cast(filters[i]);
			if (!roomFilter)
				return;
			
			// Min count - smallest min 
			SCR_FilterEntryRoomValue valueMinCount = roomFilter.FindValue(VALUE_PLAYERS_MIN_COUNT);
			if (valueMinCount)
			{
				if (minCount == -1 || valueMinCount.GetIntNumberValue() < minCount)
					minCount = valueMinCount.GetIntNumberValue();
			}
			
			// Max count - biggest max 
			SCR_FilterEntryRoomValue valueMaxCount = roomFilter.FindValue(VALUE_PLAYERS_MAX_COUNT);
			if (valueMaxCount)
			{
				if (maxCount == -1 || valueMaxCount.GetIntNumberValue() < maxCount)
					maxCount = valueMaxCount.GetIntNumberValue();
			}
			
			// Min range - smallest min 
			SCR_FilterEntryRoomValue valueMin = roomFilter.FindValue(VALUE_PLAYERS_MIN);
			if (valueMin)
			{
				if (min == -1 || valueMin.GetIntNumberValue() < min)
					min = valueMin.GetIntNumberValue();
			}
			
			// Max range - biggest max 
			SCR_FilterEntryRoomValue valueMax = roomFilter.FindValue(VALUE_PLAYERS_MAX);
			if (valueMax)
			{
				if (max == -1 || valueMax.GetIntNumberValue() > max)
					max = valueMax.GetIntNumberValue();
			}
		}
	}
	
	
	//--------------------------------------------
	// Get & set
	//--------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Go through filters in categories and create uncategorized filter list 
	void SetFilters(SCR_FilterSet filterSet) 
	{ 
		// Filter list clear up
		if (!m_aFiltersUncategorized.IsEmpty())
			m_aFiltersUncategorized.Clear();
		
		// Check filter set 
		if (!filterSet)
			return; 
		
		m_Filter = filterSet;
		
		// Check categories 
		array<ref SCR_FilterCategory> categories = filterSet.GetFilterCategories();
		if (!categories || categories.IsEmpty())
			return;
		
		// Go through filter set categories 
		foreach (SCR_FilterCategory category : categories)
		{
			// Add filters to uncategorized 
			foreach (SCR_FilterEntry filter : category.GetFilters())
			{
				SCR_FilterEntryRoom roomFilter = SCR_FilterEntryRoom.Cast(filter);
				if (roomFilter)
				{
					m_aFiltersUncategorized.Insert(roomFilter); 
				}
			}
		}
		
		// Add default filters 
		foreach (SCR_FilterEntryRoom filter : m_aDefaultFilters)
		{
			m_aFiltersUncategorized.Insert(filter);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFavoriteFilter(bool isAllowed)
	{	
		m_bFavoriteFilterOn = isAllowed;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetFavoriteFilter()
	{
		return m_bFavoriteFilterOn;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRecentlyPlayedFilter(bool isAllowed)
	{
		m_bRecentlyPlayedOn = isAllowed;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetRecentlyPlayedFilter()
	{
		return m_bRecentlyPlayedOn;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOfficialFilter(bool displayOfficial)
	{
		m_bOfficialOn = displayOfficial;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCommunityFilter(bool displayCommunity)
	{
		m_bCommunityOn = displayCommunity;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSorting(string mOrder, bool bAscendent)
	{
		if (mOrder.IsEmpty())
			order = SORT_NAME;
		else
			order = mOrder;

		// find order filter 
		SCR_FilterEntryRoom orderFilter = FindFilterByInternalName(m_aDefaultFilters, SORT_ASCENDENT);

		// Set order 
		if (orderFilter)
		{
			orderFilter.GetValues()[0].SetValue("0");
			
			if (bAscendent)
				orderFilter.GetValues()[0].SetValue("1");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	string GetSortOrder() { return order; }
	
	//------------------------------------------------------------------------------------------------
	void SetSearch(string sInput) 
	{ 
		text = sInput; 
	}
	
	//------------------------------------------------------------------------------------------------
	string GetSearchText() { return text; }

	// Scenario id 
	//------------------------------------------------------------------------------------------------
	string GetScenarioId() { return scenarioId; }
	
	//------------------------------------------------------------------------------------------------
	void SetScenarioId(string id)
	{
		scenarioId = id;
	}

	// Scenario mod id 
	//------------------------------------------------------------------------------------------------
	string GetHostedScenarioModId() { return hostedScenarioModId; }
	
	//------------------------------------------------------------------------------------------------
	void SetHostedScenarioModId(string id) 
	{ 
		hostedScenarioModId = id;
		
		// Registration
		if (!hostedScenarioModId.IsEmpty())
			RegV("hostedScenarioModId");
		else 
			UnregV("hostedScenarioModId");
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsModdedFilterSelected()
	{
		return m_bModdedFilterSelected;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOwnedOnly(bool showOwned)
	{
		ownedOnly = showOwned;
	}

	//------------------------------------------------------------------------------------------------
	void SetUsePlayerLimit(bool use)
	{
		m_bUsePlayerLimit = use;
	}
};


//------------------------------------------------------------------------------------------------
// Filter entry class extended with value 
[BaseContainerProps(), SCR_FilterEntryTitle()]
class SCR_FilterEntryRoom : SCR_FilterEntry
{
	// Filter values array 
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref SCR_FilterEntryRoomValue> m_aValues;
	
	//------------------------------------------------------------------------------------------------
	void AddFilterValue(string name, string value, EFilterType type)
	{
		// Check and initiate filter values
		if (!m_aValues)
			m_aValues = new array<ref SCR_FilterEntryRoomValue>;
		
		// Setup filter value
		SCR_FilterEntryRoomValue filterValue = new SCR_FilterEntryRoomValue;
		filterValue.SetName(name);
		filterValue.SetValue(value);
		filterValue.SetType(type);
		
		// Add value 
		m_aValues.Insert(filterValue);
	}

	//------------------------------------------------------------------------------------------------
	array<ref SCR_FilterEntryRoomValue> GetValues() { return m_aValues; }
	
	//------------------------------------------------------------------------------------------------
	SCR_FilterEntryRoomValue FindValue(string name)
	{
		for (int i = 0, count = m_aValues.Count(); i < count; i++)
		{
			if (m_aValues[i].GetName() == name)
				return m_aValues[i];
		}
		
		return null; 
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_FilterEntryTitle()]
class SCR_FilterEntryRoomValue
{
	[Attribute("", UIWidgets.EditBox, "Name of used filter")]
	protected string m_sInternalName;
	
	[Attribute("", UIWidgets.EditBox, "Filter value for any type")]
	protected string m_sValue;
	
	[Attribute("0", UIWidgets.ComboBox, desc: "Filter variable type", category: "", ParamEnumArray.FromEnum(EFilterType))]
	protected EFilterType m_iType;
	
	//------------------------------------------------------------------------------------------------
	void SetName(string name) { m_sInternalName = name; }
	
	//------------------------------------------------------------------------------------------------
	string GetName() { return m_sInternalName; }
	
	//------------------------------------------------------------------------------------------------
	void SetType(EFilterType type) { m_iType = type; }
	
	//------------------------------------------------------------------------------------------------
	EFilterType GetType() { return m_iType; }
	
	// Setting and returning value 
	
	//------------------------------------------------------------------------------------------------
	void SetValue(string value) { m_sValue = value; }
	
	//------------------------------------------------------------------------------------------------
	string GetStringValue() { return m_sValue; }
	
	//------------------------------------------------------------------------------------------------
	bool GetBoolValue() { return m_sValue == "1"; }
	
	//------------------------------------------------------------------------------------------------
	float GetIntNumberValue() { return m_sValue.ToInt(); }
	
	//------------------------------------------------------------------------------------------------
	float GetFloatNumberValue() { return m_sValue.ToFloat(); } 
};

//------------------------------------------------------------------------------------------------
enum EFilterType
{
	TYPE_BOOL,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRING,
};