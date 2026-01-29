//------------------------------------------------------------------------------------------------
class SCR_DataCollectorUI
{
	protected ResourceName m_sBaseLayout = "{76EA77A3A4A16485}UI/layouts/Menus/CareerProfile/DataCollectorUI/StatsEntryBase.layout";

	protected ResourceName m_sEntryLayout = "{EE1BCC8E5072F4C5}UI/layouts/Menus/CareerProfile/DataCollectorUI/DataStatEntry.layout";

	protected Widget m_wParentWidget;

	//------------------------------------------------------------------------------------------------
	Widget CreateEntry()
	{
		if (!GetGame() || !GetGame().GetWorkspace())
			return null;

		if (!m_wParentWidget)
		{
			m_wParentWidget = GetGame().GetWorkspace().CreateWidgets(m_sBaseLayout);
			if (!m_wParentWidget)
				return null;
		}

		Widget entriesWidget = m_wParentWidget.FindAnyWidget("Entries");
		if (!entriesWidget)
			return null;

		return GetGame().GetWorkspace().CreateWidgets(m_sEntryLayout, entriesWidget);
	}

	//------------------------------------------------------------------------------------------------
	void SetVisible(bool visible)
	{
		m_wParentWidget.SetVisible(visible);
	}

	//------------------------------------------------------------------------------------------------
	Widget GetParentWidget()
	{
		return m_wParentWidget;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_DataCollectorUI ()
	{
		m_wParentWidget = GetGame().GetWorkspace().CreateWidgets(m_sBaseLayout);
	}

	void ~SCR_DataCollectorUI()
	{
		m_wParentWidget.RemoveFromHierarchy();
	}
};
