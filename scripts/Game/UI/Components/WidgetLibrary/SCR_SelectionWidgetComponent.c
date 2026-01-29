class SCR_SelectionWidgetComponent : SCR_ChangeableComponentBase
{
	[Attribute()]
	ref array<string> m_aElementNames;

	protected ref array<ref Managed> m_aElementData = new array<ref Managed>;

	[Attribute("-1", UIWidgets.EditBox, "Element selected by default")]
	int m_iSelectedItem;

	//------------------------------------------------------------------------------------------------
	int AddItem(string item, bool last = false, Managed data = null)
	{
		if (!m_aElementNames)
			m_aElementNames = new array<string>;

		int id = m_aElementNames.Insert(item);

		m_aElementData.Insert(data);

		return id;
	}

	//------------------------------------------------------------------------------------------------
	void ClearAll()
	{
		m_iSelectedItem = -1;

		if (m_aElementNames)
			m_aElementNames.Clear();

		if (m_aElementData)
			m_aElementData.Clear();
	}

	//------------------------------------------------------------------------------------------------
  	void RemoveItem(int item, bool last = false)
	{
		if (!m_aElementNames || item < 0 || item >= m_aElementNames.Count())
			return;

		m_aElementNames.Remove(item);
		m_aElementData.Remove(item);

		m_iSelectedItem = -1;
	}

	//------------------------------------------------------------------------------------------------
	int GetCurrentIndex()
	{
		return m_iSelectedItem;
	}

	//------------------------------------------------------------------------------------------------
	string GetCurrentItem()
	{
		return GetItemName(m_iSelectedItem);
	}

	//------------------------------------------------------------------------------------------------
	Managed GetCurrentItemData()
	{
		if (!m_aElementData.IsIndexValid(m_iSelectedItem))
			return null;
		
		return m_aElementData[m_iSelectedItem];
	}

	//------------------------------------------------------------------------------------------------
	string GetItemName(int item)
	{
		if (!m_aElementNames || !m_aElementData || item < 0 || item >= m_aElementNames.Count())
			return string.Empty;

		return m_aElementNames[item];
	}

	//------------------------------------------------------------------------------------------------
	Managed GetItemData(int item)
	{
		if (!m_aElementNames || !m_aElementData || item < 0 || item >= m_aElementNames.Count() || item >= m_aElementData.Count())
			return null;

		return m_aElementData[item];
	}

	//------------------------------------------------------------------------------------------------
	int GetNumItems()
	{
		if (!m_aElementNames)
			return 0;

		return m_aElementNames.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool SetCurrentItem(int i, bool playSound = false, bool animate = false)
	{
		if (i < 0 || !m_aElementNames || !m_aElementData || i >= m_aElementNames.Count())
			return false;

		if (playSound)
			PlaySound(m_sSoundClicked);

		if (m_iSelectedItem == i)
			return false;

		m_iSelectedItem = i;
		
		return true;
	}
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// auto
	
	//------------------------------------------------------------------------------------------------
	static SCR_SelectionWidgetComponent GetSelectionComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_SelectionWidgetComponent.Cast(
			SCR_WLibComponentBase.GetComponent(SCR_SelectionWidgetComponent, name, parent, searchAllChildren)
		);
		return comp;
	}

//---- REFACTOR NOTE END ----
	
};