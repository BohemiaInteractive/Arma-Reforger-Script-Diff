//------------------------------------------------------------------------------------------------
/*!
	This InfoDisplay allows drawing of individual HUD scoring elements for individual Capture & Hold factions.
	In addition it draws all available objectives in the screen space.

	Note:
		This HUD element only supports two factions at a single given time.
		In addition only the US, USSR and FIA factions are supported.
		Using more than two factions will yield unexpected results.
		
		Similar approach can be taken, but a list of elements or any other
		dynamically filled element would be a much more preferable choice.
*/
class SCR_CaptureAndHoldTeamScoreDisplay : SCR_InfoDisplayExtended
{
	[Attribute("{56B174A06C7ECFAB}UI/Layouts/HUD/CaptureAndHold/CaptureandHoldObjectiveLayout.layout", params: "layout")]
	protected ResourceName m_rObjectiveHUDLayout;

	//! Parent frame that holds all area markers
	protected HorizontalLayoutWidget m_wAreaLayoutWidget;

	//! Array of all wrappers for the individual objective areas
	protected ref array<ref SCR_CaptureAndHoldObjectiveDisplayObject> m_aObjectiveElements = {};

	//! Array of all wrappers for the individual teams
	protected ref array<ref SCR_CaptureAndHoldTeamScoreDisplayObject> m_aScoringElements = {};

	//! Parent frame that holds all scoring widgets
	protected Widget m_wScoringFrame;

	//! Used to display remaining time (if any).
	protected TextWidget m_wRemainingTimeWidget;

	//! Area manager provides us with necessary API
	protected SCR_CaptureAndHoldManager m_pAreaManager;
	//! Game mode instance
	protected SCR_BaseGameMode m_pGameMode;

	//! Speed used to fade areas hud when hints are shown
	protected const float POINTS_LAYOUT_FADE_SPEED = 5.0;

	//------------------------------------------------------------------------------------------------
	//! Checks the prerequisites for this InfoDisplay.
	override bool DisplayStartDrawInit(IEntity owner)
	{
		// No ui can be drawn without necessary items
		m_pGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!m_pGameMode)
			return false;

		if (!m_pGameMode.GetScoringSystemComponent())
			return false;

		m_pAreaManager = SCR_CaptureAndHoldManager.Cast(m_pGameMode.FindComponent(SCR_CaptureAndHoldManager));
		if (!m_pAreaManager)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Sorts input array of areas by their name.
	//! Names must be unique!
	protected void SortAreas(array<SCR_CaptureAndHoldArea> areas)
	{
		// This is not the most ideal solution, but it's simple
		// and done only when the display starts drawing, so it should not be that bad
		map<string, SCR_CaptureAndHoldArea> lut = new map<string, SCR_CaptureAndHoldArea>();
		array<string> names = {};
		for (int i = 0, count = areas.Count(); i < count; i++)
		{
			SCR_CaptureAndHoldArea area = areas[i];
			if (!area)
				continue;

			lut.Insert(area.GetAreaSymbol(), area);
			names.Insert(area.GetAreaSymbol());
		}

		names.Sort();
		areas.Clear();
		for (int i = 0, count = names.Count(); i < count; i++)
		{
			areas.Insert(lut[names[i]]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Removes objective displays from the collection.
	//! \param emptyOnly If true, empty (not linking to area) objects are removed. If false, all objects are flushed immediately.
	protected void ClearObjectiveDisplays(bool emptyOnly)
	{
		// Clear previous elements
		for (int i = m_aObjectiveElements.Count() - 1; i >= 0; i--)
		{
			if (emptyOnly && m_aObjectiveElements[i].GetArea())
				continue;

			Widget w = m_aObjectiveElements[i].GetRootWidget();
			if (w)
			{
				w.RemoveFromHierarchy();
				delete w;
			}

			m_aObjectiveElements.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Creates and fills the objective collection using provided areas.
	protected void CreateObjectiveDisplays(array<SCR_CaptureAndHoldArea> areas)
	{
		if (!m_wRoot)
			return;

		// Sort the areas by name
		SortAreas(areas);

		// Create objective icons
		m_wAreaLayoutWidget = HorizontalLayoutWidget.Cast(m_wRoot.FindAnyWidget("Layout_Areas"));
		if (!m_wAreaLayoutWidget)
			return;

		// Create new elements
		foreach (SCR_CaptureAndHoldArea area : areas)
		{
			Widget objective = GetRootWidget().GetWorkspace().CreateWidgets(m_rObjectiveHUDLayout, m_wAreaLayoutWidget);
			if (!objective)
				break;
			
			// Add some padding to all icons
			GridSlot.SetPadding(objective, 4, 0, 4, 0);

			SCR_CaptureAndHoldObjectiveDisplayObject displayObject = new SCR_CaptureAndHoldObjectiveDisplayObject(objective, area);
			m_aObjectiveElements.Insert(displayObject);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Creates individual hud elements.
	override void DisplayStartDraw(IEntity owner)
	{
		if (m_wRoot)
		{
			// Root frame
			m_wScoringFrame = m_wRoot.FindAnyWidget("Score_Root");

			// Draw elements initially
			array<SCR_CaptureAndHoldArea> areas = {};
			m_pAreaManager.GetAreas(areas);
			CreateObjectiveDisplays(areas);
			
			array<Faction> factions = {};
			int count = GetGame().GetFactionManager().GetFactionsList(factions);
			
			SCR_Faction faction;
			for (int i = count - 1; i >= 0; i--)
			{
				faction = SCR_Faction.Cast(factions[i]);
				if (!faction.IsPlayable())
					factions.RemoveOrdered(i);
			}
			
			if (factions.Count() != 2)
			{
				Print("SCR_CaptureAndHoldTeamScoreDisplay: faction count for C&H must be 2 and is: '" +  factions.Count() + "' instead", LogLevel.ERROR);
				return;
			}

			// Left "side"
			Widget leftParent = m_wRoot.FindAnyWidget("Score_Left");
			if (leftParent)
			{
				ref SCR_CaptureAndHoldTeamScoreDisplayObject usObj = new SCR_CaptureAndHoldTeamScoreDisplayObject(leftParent, factions[0]);
				m_aScoringElements.Insert(usObj);
			}

			// Right "side"
			Widget rightParent = m_wRoot.FindAnyWidget("Score_Right");
			if (rightParent)
			{
				ref SCR_CaptureAndHoldTeamScoreDisplayObject ussrObj = new SCR_CaptureAndHoldTeamScoreDisplayObject(rightParent,  factions[1]);
				m_aScoringElements.Insert(ussrObj);
			}

			// Remaining time text in middle
			m_wRemainingTimeWidget = TextWidget.Cast(m_wRoot.FindAnyWidget("RemainingTime"));
		}
		else
		{
			Print("SCR_CaptureAndHoldDisplay could not create scoring layout!");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Clears all hud elements.
	override void DisplayStopDraw(IEntity owner)
	{
		// Release all wrappers for objectives
		ClearObjectiveDisplays(false);

		// Clear all scoring elements
		for (int i = m_aScoringElements.Count() - 1; i >= 0; i--)
		{
			m_aScoringElements.Remove(i);
		}
		
		m_aScoringElements.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates the progress and state of all available elements.
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		SCR_BaseScoringSystemComponent scoringSystem = m_pGameMode.GetScoringSystemComponent();

		// Reposition scoring UI based on whether it is in a map or not
		if (m_wScoringFrame)
		{
			SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
			if (editorManager && editorManager.IsOpened())
			{
				FrameSlot.SetPos(m_wScoringFrame, 0.0, 72.0);
			}
			else
			{
				FrameSlot.SetPos(m_wScoringFrame, 0.0, 32.0);
			}
		}

		// Fade out points when a hint is shown to prevent clipping
		if (m_wAreaLayoutWidget)
		{
			SCR_PopUpNotification notifications = SCR_PopUpNotification.GetInstance();
			float targetOpacity = 1.0;
			if (notifications && notifications.GetCurrentMsg())
				targetOpacity = 0.0;

			if (m_wAreaLayoutWidget.GetOpacity() != targetOpacity)
				m_wAreaLayoutWidget.SetOpacity(Math.Lerp(m_wAreaLayoutWidget.GetOpacity(), targetOpacity, timeSlice * POINTS_LAYOUT_FADE_SPEED));
		}

		// Update scoring
		SCR_CaptureAndHoldTeamScoreDisplayObject scoringObject;
		for (int i = 0, count = m_aScoringElements.Count(); i < count; i++)
		{
			scoringObject = m_aScoringElements[i];
			if (!scoringObject)
				continue;

			int score = 0;
			int maxScore = 0;
			if (scoringSystem)
			{
				score = scoringSystem.GetFactionScore(scoringObject.GetFaction());
				maxScore = scoringSystem.GetScoreLimit();
			}

			scoringObject.Update(score, maxScore);
		}

		// Filter out objective HUD items that are no longer linked to an area
		ClearObjectiveDisplays(true);

		// Make sure that there is enough elements
		array<SCR_CaptureAndHoldArea> areas = {};
		m_pAreaManager.GetAreas(areas);

		// If the count has changed, redraw elements (adding or removing widgets)
		if (m_aObjectiveElements.Count() != areas.Count())
		{
			ClearObjectiveDisplays(false);
			CreateObjectiveDisplays(areas);
		}

		// Update objective markers
		if (m_aObjectiveElements && m_aObjectiveElements.Count() > 0)
		{
			foreach (SCR_CaptureAndHoldObjectiveDisplayObject objective : m_aObjectiveElements)
			{
				if (!objective)
					continue;

				objective.UpdateStatic(timeSlice);
			}
		}

		// Update remaining time widget if any
		if (m_wRemainingTimeWidget)
		{
			if (m_pGameMode.GetTimeLimit() <= 0.0) // Disable showing if no time limit is set
			{
				m_wRemainingTimeWidget.SetVisible(false);
			}
			else
			{
				// Enable if time limit is set, clamp to 0
				float remainingTime = m_pGameMode.GetRemainingTime();
				if (remainingTime < 0.0)
					remainingTime = 0.0;

				m_wRemainingTimeWidget.SetVisible(true);
				m_wRemainingTimeWidget.SetText(SCR_FormatHelper.FormatTime(remainingTime));
			}
		}
	}
}
