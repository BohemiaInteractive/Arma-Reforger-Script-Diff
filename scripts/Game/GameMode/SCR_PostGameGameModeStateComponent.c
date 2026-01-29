[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Game Mode component responsible for adding a 'post-game' period to the game mode.")]
class SCR_PostGameGameModeStateComponentClass : SCR_BaseGameModeStateComponentClass
{
}

//! Game mode component that handles the post-game period.
class SCR_PostGameGameModeStateComponent : SCR_BaseGameModeStateComponent
{
	[Attribute("1", uiwidget: UIWidgets.CheckBox, "Disables player controls in this state if checked.", category: "Game Mode")]
	protected bool m_bDisableControls;

	//------------------------------------------------------------------------------------------------
	//! \return true if controls should be enabled in this state or false otherwise.
	override bool GetAllowControls()
	{
		if (m_bDisableControls)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Called on the authority when created to assign state this component belongs to.
	sealed override SCR_EGameModeState GetAffiliatedState()
	{
		return SCR_EGameModeState.POSTGAME;
	}
}
