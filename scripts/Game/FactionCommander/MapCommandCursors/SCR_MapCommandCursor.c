class SCR_MapCommandCursor
{
	protected bool m_bIsDestinationEnabled;
	protected vector m_vStartPosition;
	protected SCR_MapCursorModule m_MapCursorModule;

	protected ref ScriptInvokerVector m_OnCommandExecuted;
	protected ref ScriptInvokerVector m_OnCommandNotExecuted;

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerVector GetOnCommandExecuted()
	{
		if (!m_OnCommandExecuted)
			m_OnCommandExecuted = new ScriptInvokerVector();

		return m_OnCommandExecuted;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerVector GetOnCommandNotExecuted()
	{
		if (!m_OnCommandNotExecuted)
			m_OnCommandNotExecuted = new ScriptInvokerVector();

		return m_OnCommandNotExecuted;
	}

	//------------------------------------------------------------------------------------------------
	void ShowCursor(vector startCursorPosition)
	{
		ToggleNegativeCursor(true);
		m_bIsDestinationEnabled = false;

		SCR_MapEntity.GetOnSelection().Insert(OnSelection);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);

		m_vStartPosition = startCursorPosition;

		GetGame().GetCallqueue().CallLater(UpdateCursor, 100, true);
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanExecuteCommand(vector position)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommandExecuted(vector position)
	{
		if (m_OnCommandExecuted)
			m_OnCommandExecuted.Invoke(position);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommandNotExecuted(vector position)
	{
		if (m_OnCommandNotExecuted)
			m_OnCommandNotExecuted.Invoke(position);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSelection(vector coords)
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;

		float x, y;
		mapEntity.ScreenToWorld(coords[0], coords[2], x, y);
		coords[0] = x;
		coords[2] = y;

		if (CanExecuteCommand(coords))
		{
			OnCommandExecuted(coords);
			DisableSelection();
		}
		else
		{
			OnCommandNotExecuted(coords);
		}

		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_MAP_CLICK_POINT_ON);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCursor()
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;

		float cursorX = GetGame().GetWorkspace().DPIScale(SCR_MapCursorInfo.x);
		float cursorY = GetGame().GetWorkspace().DPIScale(SCR_MapCursorInfo.y);

		float x, y;
		vector worldPosition;
		mapEntity.ScreenToWorld(cursorX, cursorY, x, y);
		worldPosition[0] = x;
		worldPosition[2] = y;

		// set cursor type and color
		if (CanExecuteCommand(worldPosition))
		{
			if (m_bIsDestinationEnabled)
				return;

			TogglePositiveCursor(true);
			ToggleNegativeCursor(false);
			m_bIsDestinationEnabled = true;
		}
		else
		{
			if (!m_bIsDestinationEnabled)
				return;

			TogglePositiveCursor(false);
			ToggleNegativeCursor(true);
			m_bIsDestinationEnabled = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapClose(MapConfiguration config)
	{
		DisableSelection(false);// toggleOffCursors param is disabled here, because cursors are reset in the SCR_MapCursorModule
	}

	//------------------------------------------------------------------------------------------------
	void DisableSelection(bool toggleOffCursors = true)
	{
		SCR_MapEntity.GetOnSelection().Remove(OnSelection);
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);

		if (toggleOffCursors)
		{
			TogglePositiveCursor(false);
			ToggleNegativeCursor(false);
		}

		GetGame().GetCallqueue().Remove(UpdateCursor);
	}

	//------------------------------------------------------------------------------------------------
	protected void TogglePositiveCursor(bool enable)
	{
		if (!m_MapCursorModule)
			return;

		m_MapCursorModule.ToggleMapCommandPositiveCursor(enable);
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleNegativeCursor(bool enable)
	{
		if (!m_MapCursorModule)
			return;

		m_MapCursorModule.ToggleMapCommandNegativeCursor(enable);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_MapCommandCursor()
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;

		m_MapCursorModule = SCR_MapCursorModule.Cast(mapEntity.GetMapModule(SCR_MapCursorModule));
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MapCommandCursor()
	{
		DisableSelection();
	}
}
