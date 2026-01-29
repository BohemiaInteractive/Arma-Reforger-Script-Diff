[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_PauseGameTimeToolbarAction : SCR_BaseToggleToolbarAction
{
	protected ChimeraWorld m_World;
	protected SCR_BaseGameMode m_GameMode; 

	//---------------------------------------------------------------------------------------------
	override bool IsServer()
	{
		return false;
	}
	
	//---------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (!m_GameMode)
			m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		
		if (!m_GameMode)
			return false;

		return m_GameMode.CanBePaused();
	}
	
	//---------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (!m_GameMode)
			m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());

		if (!m_GameMode)
			return false;

		return m_GameMode.CanBePaused();
	}
	
	//---------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{
		SCR_PauseGameTimeEditorComponent pauseManager = SCR_PauseGameTimeEditorComponent.Cast(SCR_PauseGameTimeEditorComponent.GetInstance(SCR_PauseGameTimeEditorComponent));
		if (pauseManager)
			pauseManager.TogglePause();
		else
			m_GameMode.PauseGame(!m_World.IsGameTimePaused(), SCR_EPauseReason.EDITOR | SCR_EPauseReason.MUSIC);
	}
	
	//---------------------------------------------------------------------------------------------
	override void Track()
	{
		m_World = GetGame().GetWorld();
		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		UpdateGameTimePause();
		GetGame().GetCallqueue().CallLater(UpdateGameTimePause, 1, true);
	}
	
	//---------------------------------------------------------------------------------------------
	override void Untrack()
	{
		GetGame().GetCallqueue().Remove(UpdateGameTimePause);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void UpdateGameTimePause()
	{
		const bool isPaused = m_World.IsGameTimePaused();
		Toggle(isPaused, isPaused);
	}
}
