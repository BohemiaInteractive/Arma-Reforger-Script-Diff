[BaseContainerProps(configRoot: true)]
class SCR_PlayerListBaseAction : ScriptAndConfig
{
	[Attribute(defvalue: "1", desc: "True if action handled locally by the client.\nFalse will result in client sending the action to be handled by the server")]
	protected bool m_bIsLocalAction;

	[Attribute(desc: "Name of the action which is going to be displayed in the context menu")]
	protected string m_sActionName;

	//------------------------------------------------------------------------------------------------
	//! \returns true when action is to be performed by the client, otherwise action will be performed by the server
	bool IsLocalAction()
	{
		return m_bIsLocalAction;
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered only on the client when player list menu is opened, to work similar to a ScriptComponent.OnPostInit
	void OnPlayerListOpened(notnull SCR_PlayerController userPlayerController, SCR_ChimeraCharacter currentlyControlledCharacter);

	//------------------------------------------------------------------------------------------------
	//! Triggered only on the client  when player list menu is closed, to work similar to a ScriptComponent.OnDelete
	void OnPlayerListClosed(notnull SCR_PlayerController userPlayerController, SCR_ChimeraCharacter currentlyControlledCharacter);

	//------------------------------------------------------------------------------------------------
	//! Client side logic executed when player is opening the list of available actions for currently selected player from the list
	bool CanBeShown(int targetPlayerId, int userPlayerId, SCR_PlayerController userPlayerController);

	//------------------------------------------------------------------------------------------------
	//! Logic executed by both client and server - in case when IsLocalAction returns false
	//! Client executes this before triggering PerformAction if it is a local action, or before sending a request to the server for it to perform the action
	//! Server executes this before triggering PerformAction to see if it should be possible
	bool CanBePerformed(int targetPlayerId, int userPlayerId, SCR_PlayerController userPlayerController);

	//------------------------------------------------------------------------------------------------
	//! Triggered when action is selected by the client. Execution is happening either on the client or on the server, depending if IsLocalAction returns true or false
	void PerformAction(int targetPlayerId, int userPlayerId, SCR_PlayerController userPlayerController);

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetActionName()
	{
		return m_sActionName;
	}
}
