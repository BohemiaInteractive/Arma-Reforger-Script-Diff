//! This InfoDisplay allows drawing of individual HUD objective elements for individual
//! Capture & Hold areas by projecting their position from world to screen.
class SCR_CaptureAndHoldObjectiveDisplay : SCR_InfoDisplayExtended
{
	[Attribute("{56B174A06C7ECFAB}UI/Layouts/HUD/CaptureAndHold/CaptureandHoldObjectiveLayout.layout", params: "layout")]
	protected ResourceName m_rObjectiveHUDLayout;

	//! Individual element wrappers
	protected ref array<ref SCR_CaptureAndHoldObjectiveDisplayObject> m_aObjectiveElements = {};

	//! Area manager provides us with necessary API
	protected SCR_CaptureAndHoldManager m_AreaManager;
	//! Game mode instance.
	protected SCR_BaseGameMode m_GameMode;

	//------------------------------------------------------------------------------------------------
	//! Checks whether prerequisites for this InfoDisplay are met.
	override bool DisplayStartDrawInit(IEntity owner)
	{
		// No ui can be drawn without necessary items
		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!m_GameMode)
			return false;

		m_AreaManager = SCR_CaptureAndHoldManager.Cast(m_GameMode.FindComponent(SCR_CaptureAndHoldManager));
		if (!m_AreaManager)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Creates HUD elements for individual areas.
	override void DisplayStartDraw(IEntity owner)
	{
		// Create objective icons
		array<SCR_CaptureAndHoldArea> areas = {};
		m_AreaManager.GetAreas(areas);
		CreateObjectiveDisplays(areas);
	}

	//------------------------------------------------------------------------------------------------
	//! Destroys HUD elements of individual areas.
	override void DisplayStopDraw(IEntity owner)
	{
		// Clear ALL hud objects
		ClearObjectiveDisplays(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Updates and manages individual HUD elements.
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		// area markers
		array<SCR_CaptureAndHoldArea> areas = {};
		int count = m_AreaManager.GetAreas(areas);
		World world = GetGame().GetWorld();
		IEntity playerEntity = GetGame().GetPlayerController().GetControlledEntity();

		// Filter out entries which do not link to area anymore
		ClearObjectiveDisplays(true);
		// Redraw all elements
		if (areas.Count() != m_aObjectiveElements.Count())
		{
			ClearObjectiveDisplays(false);
			CreateObjectiveDisplays(areas);
		}

		for (int i = 0; i < count; i++)
		{
			m_aObjectiveElements[i].UpdateDynamic(playerEntity, timeSlice);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Creates and fills the objective collection using provided areas.
	protected void CreateObjectiveDisplays(array<SCR_CaptureAndHoldArea> areas)
	{
		Widget objective;
		SCR_CaptureAndHoldObjectiveDisplayObject displayObject;
		for (int i = 0, count = areas.Count(); i < count; i++)
		{
			objective = GetRootWidget().GetWorkspace().CreateWidgets(m_rObjectiveHUDLayout, GetRootWidget());
			if (!objective)
				continue;

			displayObject = new SCR_CaptureAndHoldObjectiveDisplayObject(objective, areas[i]);
			m_aObjectiveElements.Insert(displayObject);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Removes objective displays from the collection.
	//! \param emptyOnly If true, empty (not linking to area) objects are removed. If false, all objects are flushed immediately.
	protected void ClearObjectiveDisplays(bool emptyOnly)
	{
		// Clear previous elements
		Widget w;
		for (int i = m_aObjectiveElements.Count() - 1; i >= 0; i--)
		{
			if (emptyOnly && m_aObjectiveElements[i].GetArea())
				continue;

			w = m_aObjectiveElements[i].GetRootWidget();
			if (w)
			{
				w.RemoveFromHierarchy();
				delete w;
			}

			m_aObjectiveElements.Remove(i);
		}
	}
}