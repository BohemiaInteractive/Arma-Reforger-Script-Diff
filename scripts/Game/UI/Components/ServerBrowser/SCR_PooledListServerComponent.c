//! Pooled scrollable list with server entries handling.
class SCR_PooledServerListComponent : SCR_PooledListComponent
{
	// Server arrays
	protected ref array<SCR_ServerBrowserEntryComponent> m_aRoomEntries = {};
	protected ref array<Room> m_aRooms = {};

	protected int m_iLoadedPage;

	//------------------------------------------------------------------------------------------------
	// Override functions
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override protected Widget CreateEntry(Widget parent)
	{
		Widget entry = super.CreateEntry(parent);

		// Check entry
		if (!entry)
			return null;

		// Access server entry component
		SCR_ServerBrowserEntryComponent serverEntry = SCR_ServerBrowserEntryComponent.Cast(entry.FindHandler(SCR_ServerBrowserEntryComponent));

		if (!serverEntry)
			return null;

		// Setup server entry
		serverEntry.GetOnFocus().Insert(OnServerEntryFocusEnter);

		// Save server entry
		m_aRoomEntries.Insert(serverEntry);
		return entry;
	}

	//------------------------------------------------------------------------------------------------
	//! Set server entry it's room data to display server info
	//! \param[in] w
	override protected void FillEntry(Widget w)
	{
		int entryId = -1;
		int roomId = -1;

		if (!w)
			return;

		SCR_ServerBrowserEntryComponent serverEntry = ServerEntryByWidget(w, entryId);
		if (!serverEntry)
			return;

		// Check if can be displayed
		if (!IsPageLoaded(w.GetParent()))
			return;

		// Setup room id
		roomId = entryId;

		if (m_bPagesInverted)
		{
			// Modify count if pages are inverted
			if (w.GetParent() == m_wPage0)
				roomId = entryId + m_iPageEntriesCount;
			if (w.GetParent() == m_wPage1)
				roomId = entryId - m_iPageEntriesCount;
		}

		// Get addons manager
		SCR_AddonManager mgr = SCR_AddonManager.GetInstance();
		ClientLobbyApi lobby = GetGame().GetBackendApi().GetClientLobby();

		// Fill room

		if (roomId < 0 || roomId >= m_aRooms.Count())
			return;

		if (IsEntryVisible(w))
		{
			serverEntry.SetRoomInfo(m_aRooms[roomId]);
			m_aRoomEntries[entryId].EmptyVisuals(false);
		}
		else
		{
			serverEntry.SetRoomInfo(null);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Move list scroll to top
	override void MoveToTop()
	{
		//UpdateLoadedPage();
		super.MoveToTop();
		UpdateLoadedPage();
	}

	//------------------------------------------------------------------------------------------------
	override protected void SetCurrentPage(int page)
	{
		// Update base and invoke
		SetCurrentPageBase(page);

		// Display unloaded
		if (m_iLoadedPage == m_iCurrentPage)
			return;

		// Muliple pages
		if (Math.AbsInt(m_iLoadedPage - m_iCurrentPage) > 1)
		{
			foreach (SCR_ServerBrowserEntryComponent entry : m_aRoomEntries)
			{
				entry.EmptyVisuals(true);
			}

			return;
		}

		// Single page change
		int start = 0;

		// + 1 page - go down
		if (m_iLoadedPage + 1 == m_iCurrentPage)
		{
			if (!m_bPagesInverted)
				start = m_iPageEntriesCount;

			// Loading
			for (int i = m_iPageEntriesCount + start - 1; i >= start; i--)
			{
				m_aRoomEntries[i].EmptyVisuals(true);
			}

			return;
		}

		// -1 page - go up
		if (m_iLoadedPage - 1 == m_iCurrentPage)
		{
			if (m_bPagesInverted)
				start = m_iPageEntriesCount;

			// Loading
			for (int i = start, max = m_iPageEntriesCount + start; i < max; i++)
			{
				m_aRoomEntries[i].EmptyVisuals(true);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsPageLoaded(Widget wPage)
	{
		// Multiple pages scroll
		if (Math.AbsInt(m_iLoadedPage - m_iCurrentPage) > 1)
			return false;

		// Down
		if (m_iLoadedPage + 1 == m_iCurrentPage)
		{
			if (m_bPagesInverted && wPage == m_wPage1)
				return true;

			if (!m_bPagesInverted && wPage == m_wPage0)
				return true;

			return false;
		}

		// Up
		if (m_iLoadedPage - 1 == m_iCurrentPage)
		{
			if (m_bPagesInverted && wPage == m_wPage1)
				return true;

			if (!m_bPagesInverted && wPage == m_wPage0)
				return true;

			return false;
		}

		// Current page loaded
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Invoker actions
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Call this when focusing on server entry
	//! Show data about server
	//! \param[in] entry
	protected void OnServerEntryFocusEnter(SCR_ScriptedWidgetComponent entry)
	{
		SCR_ServerBrowserEntryComponent serverEntry = SCR_ServerBrowserEntryComponent.Cast(entry);

		// Set current focused id
		int entryId = m_aRoomEntries.Find(serverEntry);
		entryId += m_iCurrentPage * m_iPageEntriesCount;

		if (m_bPagesInverted)
		{
			// Modify count if pages are inverted
			if (serverEntry.GetRootWidget().GetParent() == m_wPage0)
				entryId += m_iPageEntriesCount;
			else if (serverEntry.GetRootWidget().GetParent() == m_wPage1)
				entryId -= m_iPageEntriesCount;
		}
	}

	//------------------------------------------------------------------------------------------------
	// Protectted functions
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! \param[in] entryWidget
	//! \param[out] id
	//! \return server entry owned by given entry widget root
	protected SCR_ServerBrowserEntryComponent ServerEntryByWidget(Widget entryWidget, out int id)
	{
		// Check widget
		if (!entryWidget)
			return null;

		// Find server entry by id
		id = m_aEntryWidgets.Find(entryWidget);

		if (0 <= id && id < m_aRoomEntries.Count())
			return m_aRoomEntries[id];

		return null;
	}

	//------------------------------------------------------------------------------------------------
	// Public functions
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! \param[in] rooms
	//! \param[in] allRoomsCount
	//! \param[in] animate
	void SetRooms(array<Room> rooms, int allRoomsCount = -1, bool animate = false)
	{
		m_wRoot.SetVisible(true);

		m_aRooms = rooms;

		int roomsCount = rooms.Count();

		// Update data
		int dataCount = allRoomsCount;
		if (allRoomsCount == -1)
			dataCount = m_aRooms.Count();

		SetDataEntries(allRoomsCount);

		UpdateEntries(animate);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void ShowEmptyRooms()
	{
		m_aRooms.Clear();
		UpdateEntries(false);

		// Hide entries
		foreach (Widget widget : m_aEntryWidgets)
		{
			widget.SetVisible(false);
		}

		if (m_bIsMeasured)
		{
			m_wRoot.ClearFlags(WidgetFlags.CLIPCHILDREN);
			m_wRoot.ClearFlags(WidgetFlags.INHERIT_CLIPPING);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Call this to verify currently displayed data are loaded
	void UpdateLoadedPage()
	{
		m_iLoadedPage = m_iCurrentPage;
	}

	//------------------------------------------------------------------------------------------------
	//! Return true if room of given entry is loaded
	//! Base on widget and current id check if it's in loaded servers batch
	//! \param[in] entry
	//! \return
	bool IsRoomLoaded(notnull SCR_ServerBrowserEntryComponent entry)
	{
		if (m_aRoomEntries.IsEmpty())
			return false;

		int id = m_aRoomEntries.Find(entry);
		if (id == -1)
			return false;

		return m_iLoadedPage == m_iCurrentPage;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] entry
	//! \return true if entry not pass whole found server list
	protected bool IsEntryVisible(Widget entry)
	{
		// Current id - based on current page
		int id = m_aEntryWidgets.Find(entry);
		id += m_iCurrentPage * m_iPageEntriesCount;

		// Is over count?
		if (id > m_iAllEntriesCount)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	array<SCR_ServerBrowserEntryComponent> GetRoomEntries()
	{
		return m_aRoomEntries;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetLoadedPage()
	{
		return m_iLoadedPage;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetCurrentPage()
	{
		return m_iCurrentPage;
	}
}
