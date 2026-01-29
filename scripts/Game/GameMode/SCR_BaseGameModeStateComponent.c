[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Base for game mode state component handlers.", visible: false)]
class SCR_BaseGameModeStateComponentClass : SCR_BaseGameModeComponentClass
{
}

//! Base component for handling game mode states.
class SCR_BaseGameModeStateComponent : SCR_BaseGameModeComponent
{
	//------------------------------------------------------------------------------------------------
	//! \return false on authority if controls for this state should be disabled.
	bool GetAllowControls()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the expected duration of this state or 0 if none.
	float GetDuration()
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Called on authority to check if transition to next state can occur in current step.
	//! \param[in] nextState The state to transition into.
	bool CanAdvanceState(SCR_EGameModeState nextState)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when game mode transitions into the state this component represents.
	protected void OnStateEntered();

	//------------------------------------------------------------------------------------------------
	//! Called when game mode state changes called on all machines.
	//! \param[in] state New state game mode transitioned into.
	override void OnGameStateChanged(SCR_EGameModeState state)
	{
		super.OnGameStateChanged(state);

		if (state == GetAffiliatedState())
			OnStateEntered();
	}

	//------------------------------------------------------------------------------------------------
	//! Called on the authority when created to assign state this component belongs to.
	SCR_EGameModeState GetAffiliatedState()
	{
		Debug.Error("Not implemented!");
		return -1;
	}
}
