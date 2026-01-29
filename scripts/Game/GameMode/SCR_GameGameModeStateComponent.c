[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Game Mode component responsible for controlling the main game state.")]
class SCR_GameGameModeStateComponentClass : SCR_BaseGameModeStateComponentClass
{
}

class SCR_GameGameModeStateComponent : SCR_BaseGameModeStateComponent
{
	[Attribute("0", uiwidget: UIWidgets.Slider, "Time limit of game mode in seconds.", params: "0 864000 1", category: "Game Mode")]
	protected float m_fDuration;

	//------------------------------------------------------------------------------------------------
	//! \return the expected duration of this state or 0 if none.
	override float GetDuration()
	{
		return m_fDuration;
	}

	//------------------------------------------------------------------------------------------------
	//! Called on authority to check if transition to next state can occur in current step.
	//! \param[in] nextState The state to transition into.
	override bool CanAdvanceState(SCR_EGameModeState nextState)
	{
		if (m_fDuration > 0.0 && m_pGameMode.GetElapsedTime() >= m_fDuration)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if controls should be enabled in this state or false otherwise.
	//! This value must be synchronised for all the authority and proxy clients likewise.
	override bool GetAllowControls()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Called on the authority when created to assign state this component belongs to.
	sealed override SCR_EGameModeState GetAffiliatedState()
	{
		return SCR_EGameModeState.GAME;
	}
}
