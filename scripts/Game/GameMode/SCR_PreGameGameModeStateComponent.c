[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Game Mode component responsible for adding a 'pre-game' period to the game mode.")]
class SCR_PreGameGameModeStateComponentClass : SCR_BaseGameModeStateComponentClass
{
}

//! Game mode component that handles the pre-game period.
class SCR_PreGameGameModeStateComponent : SCR_BaseGameModeStateComponent
{
	[Attribute("30", uiwidget: UIWidgets.Slider, "Duration of this state in seconds or 0 for infinite duration. (Requires manual start)", params: "0 864000 1", category: "Game Mode")]
	protected float m_fDuration;

	[Attribute("1", uiwidget: UIWidgets.CheckBox, "Disables player controls in this state if checked.", category: "Game Mode")]
	protected bool m_bDisableControls;
	
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
		return m_fDuration > 0.0 && m_pGameMode.GetElapsedTime() >= m_fDuration;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if controls should be enabled in this state, false otherwise.
	override bool GetAllowControls()
	{
		return !m_bDisableControls;
	}

	//------------------------------------------------------------------------------------------------
	//! Called on the authority when created to assign state this component belongs to.
	sealed override SCR_EGameModeState GetAffiliatedState()
	{
		return SCR_EGameModeState.PREGAME;
	}
}
