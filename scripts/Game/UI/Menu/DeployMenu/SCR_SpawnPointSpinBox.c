class SCR_SpawnPointSpinBox : SCR_SpinBoxComponent
{
	protected ref array<RplId> m_aSpawnPointIds = {};
	protected bool m_bFocused;

	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		m_bFocused = true;
		
		return false;
	}	
	
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		m_bFocused = false;
		
		return false;
	}
	
	int AddItem(string item, RplId id)
	{
		if (!id.IsValid() || m_aSpawnPointIds.Find(id) > -1)
			return -1;
		int i = AddItem(item);
		m_aSpawnPointIds.Insert(id);
		SetInitialState();

		return i;
	}

	override void RemoveItem(int item, bool last = false)
	{
		if (m_aSpawnPointIds.IsIndexValid(item))
			m_aSpawnPointIds.Remove(item);

		super.RemoveItem(item, true);
	}

	override void ClearAll()
	{
		super.ClearAll();
		m_aSpawnPointIds.Clear();
	}

	RplId GetSpawnPointId(int itemId)
	{
		if (m_aSpawnPointIds.IsEmpty())
			return RplId.Invalid();
		return m_aSpawnPointIds.Get(itemId);
	}

	int GetItemId(RplId id)
	{
		return m_aSpawnPointIds.Find(id);
	}

	bool IsEmpty()
	{
		return m_aSpawnPointIds.IsEmpty();
	}

	void SetItemName(int id, string name)
	{
		if (m_aElementNames.IsIndexValid(id))
		{
			m_aElementNames[id] = name;
			SetInitialState(false);
		}
	}

	array<SCR_SpawnPoint> GetSpawnPointsInList()
	{
		array<SCR_SpawnPoint> result = {};
		foreach (RplId id : m_aSpawnPointIds)
		{
			SCR_SpawnPoint sp = SCR_SpawnPoint.GetSpawnPointByRplId(id);
			if (sp)
				result.Insert(sp);
		}

		return result;
	}
	
	bool IsFocused()
	{
		return m_bFocused && GetGame().GetInputManager().IsUsingMouseAndKeyboard();
	}
};