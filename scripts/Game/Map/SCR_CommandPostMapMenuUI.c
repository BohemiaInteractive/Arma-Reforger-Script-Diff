
//! Command post map menu
class SCR_CommandPostMapMenuUI : SCR_MapMenuUI
{
	protected ref ScriptInvokerVoid m_OnClosedByEditor = new ScriptInvokerVoid();

	protected bool m_bWasMapClosedByEditor;
	protected SCR_CharacterControllerComponent m_CharacterController;

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnClosedByEditor()
	{
		return m_OnClosedByEditor;
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenMap()
	{
		if (m_MapEntity)
		{
			BaseGameMode gameMode = GetGame().GetGameMode();
			if (!gameMode)
				return;

			SCR_MapConfigComponent configComp = SCR_MapConfigComponent.Cast(gameMode.FindComponent(SCR_MapConfigComponent));
			if (!configComp)
				return;

			EMapEntityMode currentMapEntityMode = configComp.GetCommandPostMapEntityMode();
			ResourceName configResourceName = "";

			switch (currentMapEntityMode)
			{
				case EMapEntityMode.COMBAT_SUPPORT_STATION :
					configResourceName = configComp.GetCombatSupportStationMapConfig();
					break;

				case EMapEntityMode.OPERATIONS_STATION :
					configResourceName = configComp.GetOperationsStationMapConfig();
					break;

				case EMapEntityMode.LOGISTICS_STATION :
					configResourceName = configComp.GetLogisticsStationMapConfig();
					break;

				default:
					Print("SCR_CommandPostMapMenuUI EMapEntityMode is not set", LogLevel.ERROR);
					break;
			}

			MapConfiguration mapConfigFullscreen = m_MapEntity.SetupMapConfig(currentMapEntityMode, configResourceName, GetRootWidget());
			m_MapEntity.OpenMap(mapConfigFullscreen);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		OpenMap();

		Widget wChatPanel = GetRootWidget().FindAnyWidget("ChatPanel");
		if (wChatPanel)
			m_ChatPanel = SCR_ChatPanel.Cast(wChatPanel.FindHandler(SCR_ChatPanel));

		GetGame().GetInputManager().AddActionListener("ChatToggle", EActionTrigger.DOWN, Callback_OnChatToggleAction);

		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		if (!editorManagerEntity)
			return;

		editorManagerEntity.GetOnOpened().Insert(OnEditorOpened);
		editorManagerEntity.GetOnClosed().Insert(OnEditorClosed);

		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!characterController)
			return;

		m_CharacterController = characterController;
		m_CharacterController.m_OnLifeStateChanged.Insert(OnLifeStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();

		SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
		if (!editorManagerEntity)
			return;

		editorManagerEntity.GetOnOpened().Remove(OnEditorOpened);
		editorManagerEntity.GetOnClosed().Remove(OnEditorClosed);

		if (m_CharacterController)
			m_CharacterController.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorOpened()
	{
		if (m_MapEntity.IsOpen())
		{
			// close map, the map will be closed by the editor anyway
			m_MapEntity.CloseMap();
			m_bWasMapClosedByEditor = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorClosed()
	{
		if (m_bWasMapClosedByEditor)
		{
			m_OnClosedByEditor.Invoke();
			Close();
		}

		m_bWasMapClosedByEditor = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState, bool isJIP)
	{
		if (newLifeState == ECharacterLifeState.ALIVE)
			return;

		m_bWasMapClosedByEditor = false;

		Close();
	}
}
