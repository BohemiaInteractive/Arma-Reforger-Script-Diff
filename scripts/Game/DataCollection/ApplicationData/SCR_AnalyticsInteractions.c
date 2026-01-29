// Class that tracks interactions in server browser, scenarios, and workshop
// Note: Not all of these section need to have all the exact same sections (e.g., scenarios do not have filters)
class SCR_AnalyticsInteractions
{
	protected static const int MIN_DELAY_BETWEEN_SEARCHES_MS = 100;
	protected ref SCR_AnalyticsDelayedSend m_DelayedSend = new SCR_AnalyticsDelayedSend();
	protected ref map<string, int> m_Tabs = new map<string, int>();
	protected ref map<string, int> m_Sortings = new map<string, int>();
	protected ref map<string, ref map<string, int>> m_Filters = new map<string, ref map<string, int>>();
	protected WorldTimestamp m_LastSearchUsage = null;
	protected int m_iSearchUsages = 0;
	protected int m_iFilterOnUsages = 0;
	protected int m_iFavoriteUsages = 0;	
	protected int m_iReopenTimes = 1;
	protected bool m_bIsOpened = false;
	
	//------------------------------------------------------------------------------------------------
	//! Start tracking interactions. This needs to be used BEFORE any other method and CLEARS all 
	//! current data if it is not openned soon enough from the last openned instance.
	void Start()
	{
		m_bIsOpened = true;
		m_DelayedSend.Start();
		if (m_DelayedSend.IsAdditive())
		{
			m_iReopenTimes += 1;
		}
		else
		{
			m_Tabs.Clear();
			m_Sortings.Clear();
			m_Filters.Clear();
			m_iSearchUsages = 0;
			m_iFilterOnUsages = 0;
			m_iFavoriteUsages = 0;
			m_iReopenTimes = 1;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Closes tracking interactions. Will start tracking delay before it will be ready to send to 
	//! analytics.
	void Close()
	{
		m_DelayedSend.Stop();
		m_bIsOpened = false;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Update the current timers with specified seconds that have elapsed since last update.
	//! \param[in] deltaSeconds
	void Update(float deltaSeconds)
	{
		m_DelayedSend.Update(deltaSeconds);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Can we send our current data to analytics already?
	//! \return bool
	bool IsReadyToSend()
	{
		return m_DelayedSend.IsReadyToSend();
	}
	
	//------------------------------------------------------------------------------------------------
	//! We sended the data, reset current state.
	void ResetReadyToSend()
	{
		m_DelayedSend.ResetReadyToSend();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return how many times we have re-openned the menu (1 = we have openned the menu only once).
	int GetReopenTimes()
	{
		return m_iReopenTimes;
	}

	//------------------------------------------------------------------------------------------------
	//! \return time in seconds which was tracked since Start().
	int GetTimeSpent()
	{
		return m_DelayedSend.GetTimeSpent();
	}

	//------------------------------------------------------------------------------------------------
	//! Set a different tab and save it to the interactions.
	//! \param[in] tab
	void SetTab(string tab)
	{
		if (!m_bIsOpened)
			return;

		int count = m_Tabs.Get(tab); // Get returns 0 if tab is not used
		m_Tabs.Set(tab, count + 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Set a different sorting and save it to the interactions.
	//! \param[in] sorting
	void SetSorting(string sorting)
	{
		if (!m_bIsOpened)
			return;

		int count = m_Sortings.Get(sorting); // Get returns 0 if sorting is not used
		m_Sortings.Set(sorting, count + 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Set a filter and save it to the interactions.
	//! \param[in] filterCategory
	//! \param[in] filterName
	void SetFilter(string filterCategory, string filterName)
	{		
		if (!m_bIsOpened)
			return;

		if (!m_Filters.Contains(filterCategory))
			m_Filters.Insert(filterCategory, new map<string, int>());

		map<string, int> filterNames = m_Filters.Get(filterCategory);
		int count = filterNames.Get(filterName);
		filterNames.Set(filterName, count + 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Use search box and save it to the interactions.
	void UseSearch()
	{		
		if (!m_bIsOpened)
			return;

		// Search box is broken as it sometimes sends two events when confirmed instead of one therefore 
		// we measure how much time has past between them, and if it is short enough we ignore the 
		// second event.
		WorldTimestamp current = GetGame().GetWorld().GetTimestamp();
		
		if (m_LastSearchUsage && current.DiffMilliseconds(m_LastSearchUsage) < MIN_DELAY_BETWEEN_SEARCHES_MS)
			return;
		
		m_iSearchUsages += 1;
		m_LastSearchUsage = current;
	}

	//------------------------------------------------------------------------------------------------
	//! Use filter toggle and save it to the interactions.
	void UseFilterOn()
	{		
		if (!m_bIsOpened)
			return;

		m_iFilterOnUsages += 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Use favorite button to the interactions.
	void UseFavorite()
	{		
		if (!m_bIsOpened)
			return;

		m_iFavoriteUsages += 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Get all saved interactions, in JSON-like format.
	//! \return string
	string GetInteractions()
	{
		array<string> arr = new array<string>();

		if (!m_Tabs.IsEmpty())
			arr.Insert(string.Format("'TabName':%1", SerializeMap(m_Tabs)));

		if (!m_Sortings.IsEmpty())
			arr.Insert(string.Format("'Sorting':%1", SerializeMap(m_Sortings)));

		if (!m_Filters.IsEmpty())
			arr.Insert(string.Format("'Filters':%1", SerializeFilters()));

		if (m_iSearchUsages > 0)
			arr.Insert(string.Format("'Searches':%1", m_iSearchUsages));

		if (m_iFilterOnUsages > 0)
			arr.Insert(string.Format("'FilterOn':%1", m_iFilterOnUsages));

		string result = "{";
		bool firstElement = true;
		foreach (string val : arr)
		{
			if (firstElement)
				result += val;
			else
				result += "," + val;

			firstElement = false;
		}

		return result + "}";
	}

	//------------------------------------------------------------------------------------------------
	protected string SerializeMap(map<string, int> mapp)
	{
		string result = "{";

		bool firstElement = true;
		foreach (string category, int count : mapp)
		{
			if (firstElement)
				result += string.Format("'%1':%2", category, count);
			else
				result += string.Format(",'%1':%2", category, count);

			firstElement = false;
		}

		return result + "}";
	}

	//------------------------------------------------------------------------------------------------
	protected string SerializeFilters()
	{
		string result = "{";

		bool firstElement = true;
		foreach (string category, map<string, int> names : m_Filters)
		{
			if (firstElement)
				result += string.Format("'%1':%2", category, SerializeMap(names));
			else
				result += string.Format(",'%1':%2", category, SerializeMap(names));

			firstElement = false;
		}

		return result + "}";
	}
}
