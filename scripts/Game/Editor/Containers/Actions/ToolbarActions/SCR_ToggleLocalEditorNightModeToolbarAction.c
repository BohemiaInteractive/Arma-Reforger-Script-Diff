[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_Info")]
class SCR_ToggleLocalEditorNightModeToolbarAction : SCR_BaseToggleToolbarAction
{
	protected SCR_NightModeGameModeComponent m_NightModeComponent;
	
	//------------------------------------------------------------------------------------------------
	protected void OnLocalEditorNightModeEnabledChanged(bool state)
	{
		Toggle(state, state)
	}

	//------------------------------------------------------------------------------------------------
	override bool IsServer()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		if (!m_NightModeComponent)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (!gameMode)
				return false;
			
			m_NightModeComponent = SCR_NightModeGameModeComponent.Cast(gameMode.FindComponent(SCR_NightModeGameModeComponent));
		}
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager || editorManager.IsLimited())
			return false;
		
		return m_NightModeComponent && m_NightModeComponent.CanEnableNightMode() && !m_NightModeComponent.IsGlobalNightModeEnabled();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition, int flags)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void Perform(SCR_EditableEntityComponent hoveredEntity, notnull set<SCR_EditableEntityComponent> selectedEntities, vector cursorWorldPosition,int flags, int param = -1)
	{		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (!editorManager || editorManager.IsLimited())
			return;
		
		//~ Toggle Local Editor Nightmode
		m_NightModeComponent.EnableLocalEditorNightMode(!m_NightModeComponent.IsLocalEditorNightModeEnabled(), false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Track()
	{
		if (!m_NightModeComponent)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (!gameMode)
				return;
			
			m_NightModeComponent = SCR_NightModeGameModeComponent.Cast(gameMode.FindComponent(SCR_NightModeGameModeComponent));
			
			if (!m_NightModeComponent)
				return;
		}

		m_NightModeComponent.GetOnLocalEditorNightModeEnabledChanged().Insert(OnLocalEditorNightModeEnabledChanged);
		OnLocalEditorNightModeEnabledChanged(m_NightModeComponent.IsLocalEditorNightModeEnabled());
	}
	
	//------------------------------------------------------------------------------------------------
	override void Untrack()
	{
		if (!m_NightModeComponent) 
			return;
		
		m_NightModeComponent.GetOnLocalEditorNightModeEnabledChanged().Remove(OnLocalEditorNightModeEnabledChanged);
	}
}