[BaseContainerProps(configRoot: true)]
class SCR_PlayerListActions
{
	[Attribute(defvalue: "", desc: "Player List Actions")]
	protected ref array<ref SCR_PlayerListBaseAction> m_aActions;

	//------------------------------------------------------------------------------------------------
	//!
	void OnPlayerListOpened(notnull SCR_PlayerController userPlayerController)
	{
		SCR_ChimeraCharacter currentlyControlledCharacter = SCR_ChimeraCharacter.Cast(userPlayerController.GetControlledEntity());
		foreach (SCR_PlayerListBaseAction action : m_aActions)
		{
			action.OnPlayerListOpened(userPlayerController, currentlyControlledCharacter);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	void OnPlayerListClosed(notnull SCR_PlayerController userPlayerController)
	{
		SCR_ChimeraCharacter currentlyControlledCharacter = SCR_ChimeraCharacter.Cast(userPlayerController.GetControlledEntity());
		foreach (SCR_PlayerListBaseAction action : m_aActions)
		{
			action.OnPlayerListClosed(userPlayerController, currentlyControlledCharacter);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	bool CanUseAnyAction(int targetPlayerId, int userPlayerId, notnull SCR_PlayerController localPlayerController)
	{
		foreach (int id, SCR_PlayerListBaseAction action : m_aActions)
		{
			if (action.CanBeShown(targetPlayerId, userPlayerId, localPlayerController))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void CreateActionEntries(notnull SCR_ComboBoxComponent combo, int targetPlayerId, int userPlayerId, notnull SCR_PlayerController localPlayerController)
	{
		foreach (int id, SCR_PlayerListBaseAction action : m_aActions)
		{
			if (!action.CanBeShown(targetPlayerId, userPlayerId, localPlayerController))
				continue;

			combo.AddItem(action.GetActionName(), true, new SCR_PlayerListComboEntryData(id, SCR_EPlayerListComboAction.CUSTOM_ACTION));
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	bool PerformActionClient(int actionId, int targetPlayerId, int userPlayerId, notnull SCR_PlayerController localPlayerController, out bool executeOnServer = false)
	{
		if (!m_aActions.IsIndexValid(actionId))
			return false;

		SCR_PlayerListBaseAction action = m_aActions[actionId];
		if (!action)
			return false;

		if (!action.CanBePerformed(targetPlayerId, userPlayerId, localPlayerController))
			return false;

		if (action.IsLocalAction())
			action.PerformAction(targetPlayerId, userPlayerId, localPlayerController);
		else
			executeOnServer = true;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void PerformActionServer(int actionId, int targetPlayerId, int userPlayerId, notnull SCR_PlayerController userPlayerController)
	{
		if (!m_aActions.IsIndexValid(actionId))
			return;

		SCR_PlayerListBaseAction action = m_aActions[actionId];
		if (!action)
			return;

		if (action.IsLocalAction())
		{
			PrintFormat("WARNING: Player %1 wanted server to execute action '%2' which is a LOCAL action!", SCR_PlayerIdentityUtils.GetPlayerLogInfo(userPlayerId), action.GetActionName(), level: LogLevel.WARNING);
			return;
		}

		if (!action.CanBePerformed(targetPlayerId, userPlayerId, userPlayerController))
			return;

		action.PerformAction(targetPlayerId, userPlayerId, userPlayerController);
	}
}
