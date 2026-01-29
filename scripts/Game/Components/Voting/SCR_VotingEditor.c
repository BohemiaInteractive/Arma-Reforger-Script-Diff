[BaseContainerProps(configRoot:true), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingEditorIn : SCR_VotingReferendum
{
	[Attribute(SCR_Enum.GetDefault(EEditorMode.EDIT), uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EEditorMode))]
	protected EEditorMode m_EditorModes;
	
	protected SCR_EditorManagerEntity m_EditorManager;
	
	//------------------------------------------------------------------------------------------------
	protected bool EvaluateEditor(out EVotingOutcome outcome)
	{
		if (!m_EditorManager || !m_EditorManager.IsLimited())
		{
			outcome = EVotingOutcome.FORCE_FAIL;
			return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool Evaluate(out EVotingOutcome outcome)
	{
		if (!m_EditorManager)
		{
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			m_EditorManager = core.GetEditorManager(GetValue());
		}
		
		if (EvaluateEditor(outcome))
			return true;
		else
			return super.Evaluate(outcome);
	}

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(int value, bool isOngoing)
	{
		//--- Only local player can nominate themselves, but everyone else can vote for them afterwards
		if (isOngoing || SCR_PlayerController.GetLocalPlayerId() == value)
		{
			SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
			if (editorDelegateManager)
			{
				//--- Cannot vote for existing GMs
				SCR_EditablePlayerDelegateComponent playerEditorDelegate = editorDelegateManager.GetDelegate(value);
				return playerEditorDelegate && playerEditorDelegate.HasLimitedEditor();
			}
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanSendNotification(int value)
	{
		if (!Replication.IsRunning())
			return false;
		
		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		SCR_GameModeEditor gameMode = SCR_GameModeEditor.Cast(GetGame().GetGameMode());
		return (votingManager && votingManager.GetHostPlayerID() != value) || (gameMode && gameMode.GetGameMasterTarget() != EGameModeEditorTarget.EVERYBODY);
	}

	//------------------------------------------------------------------------------------------------
	override void OnVotingEnd(int value = DEFAULT_VALUE, int winner = DEFAULT_VALUE)
	{
		if (winner == DEFAULT_VALUE)
			return;
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		SCR_EditorManagerEntity editorManager = core.GetEditorManager(winner);
		editorManager.AddEditorModes(EEditorModeAccess.BASE, m_EditorModes, false);
	}

	//------------------------------------------------------------------------------------------------
	override void InitFromTemplate(SCR_VotingBase template, int startingPlayerID, int value, float remainingDuration)
	{
		super.InitFromTemplate(template, startingPlayerID, value, remainingDuration);
		
		SCR_VotingEditorIn templateEditor = SCR_VotingEditorIn.Cast(template);
		m_EditorModes = templateEditor.m_EditorModes;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingEditorOut: SCR_VotingEditorIn
{
	//------------------------------------------------------------------------------------------------
	override protected bool EvaluateEditor(out EVotingOutcome outcome)
	{
		if (!m_EditorManager || m_EditorManager.IsLimited())
		{
			outcome = EVotingOutcome.FORCE_FAIL;
			return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override protected int GetPlayerCount()
	{
		return Math.Max(super.GetPlayerCount() - 1, 2); //--- Ignore target player. 2 is a limit to prevent instant completion in a session with just 2 people.
	}

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(int value, bool isOngoing)
	{
		//--- Cannot vote for self
		if (SCR_PlayerController.GetLocalPlayerId() == value)
			return false;
		
		//--- Cannot vote host or admin out
		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		if (!votingManager || votingManager.GetHostPlayerID() == value || GetGame().GetPlayerManager().HasPlayerRole(value, EPlayerRole.ADMINISTRATOR))
			return false;
		
		//--- Cannot vote for players who are not GMs
		SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
		if (!editorDelegateManager)
			return false;
		
		SCR_EditablePlayerDelegateComponent playerEditorDelegate = editorDelegateManager.GetDelegate(value);
		return playerEditorDelegate && !playerEditorDelegate.HasLimitedEditor();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanSendNotification(int value)
	{
		SCR_GameModeEditor gameMode = SCR_GameModeEditor.Cast(GetGame().GetGameMode());
		return gameMode && gameMode.GetGameMasterTarget() != EGameModeEditorTarget.EVERYBODY;
	}

	//------------------------------------------------------------------------------------------------
	override void OnVotingEnd(int value = DEFAULT_VALUE, int winner = DEFAULT_VALUE)
	{
		if (winner == DEFAULT_VALUE)
			return;
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		SCR_EditorManagerEntity editorManager = core.GetEditorManager(winner);
		editorManager.RemoveEditorModes(EEditorModeAccess.BASE, m_EditorModes);
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EVotingType, "m_Type")]
class SCR_VotingEditorWithdraw: SCR_VotingEditorIn
{
	//------------------------------------------------------------------------------------------------
	override bool Evaluate(out EVotingOutcome outcome)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(int value, bool isOngoing)
	{
		//--- Can apply only for self
		if (SCR_PlayerController.GetLocalPlayerId() != value)
			return false;
		
		//--- Cannot apply when a host or an admin
		SCR_VotingManagerComponent votingManager = SCR_VotingManagerComponent.GetInstance();
		if (!votingManager || votingManager.GetHostPlayerID() == value || GetGame().GetPlayerManager().HasPlayerRole(value, EPlayerRole.ADMINISTRATOR))
			return false;
		
		//--- Cannot vote for players who are not GMs
		SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
		if (!editorDelegateManager)
			return false;
		
		SCR_EditablePlayerDelegateComponent playerEditorDelegate = editorDelegateManager.GetDelegate(value);
		return playerEditorDelegate && !playerEditorDelegate.HasLimitedEditor();
	}

	//------------------------------------------------------------------------------------------------
	override void OnVotingEnd(int value = DEFAULT_VALUE, int winner = DEFAULT_VALUE)
	{
		if (winner == DEFAULT_VALUE)
			return;
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		SCR_EditorManagerEntity editorManager = core.GetEditorManager(winner);
		editorManager.RemoveEditorModes(EEditorModeAccess.BASE, m_EditorModes);
	}
}
