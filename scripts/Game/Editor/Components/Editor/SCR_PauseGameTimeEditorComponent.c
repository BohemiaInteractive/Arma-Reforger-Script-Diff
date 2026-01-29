[ComponentEditorProps(category: "GameScripted/Editor", description: "Game simulation manager. Works only with SCR_EditorManagerEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_PauseGameTimeEditorComponentClass : SCR_BaseEditorComponentClass
{
}

/** @ingroup Editor_Components
*/
class SCR_PauseGameTimeEditorComponent : SCR_BaseEditorComponent
{
	[Attribute("0", desc: "When enabled, the editor will be paused when opened.")]
	protected bool m_bPauseOnOpen;

	//------------------------------------------------------------------------------------------------
	/*!
	Toggle pause mode, e.g., pause the game when it's not paused.
	*/
	void TogglePause()
	{
		const ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		SetPause(!world.IsGameTimePaused());
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set pause mode.
	\param pause True to pause the game, false to unpause it
	*/
	void SetPause(bool pause)
	{
		// Unpausing game, create a rewind point
		if (!pause)
		{
			SCR_RewindComponent rewindManager = SCR_RewindComponent.GetInstance();
			if (rewindManager && rewindManager.CanRewind() && !rewindManager.HasRewindPoint())
				rewindManager.CreateRewindPoint();
		}

		DoPause(pause);
	}

	//------------------------------------------------------------------------------------------------
	protected void DoPause(bool pause)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.PauseGame(pause, SCR_EPauseReason.EDITOR | SCR_EPauseReason.MUSIC);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Set if the game should be paused when the editor is opened.
	\param pause True to pause the game whent he editor is opened
	*/
	void SetPauseOnOpen(bool pause)
	{
		m_bPauseOnOpen = pause;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	\return True if the game is set to be paused when the editor is opened
	*/
	bool IsPauseOnOpen()
	{
		return m_bPauseOnOpen;
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorOpen()
	{
		EventProvider.ConnectEvent(GetGame().GetSaveGameManager().OnSaveCreated, OnSaved);

		// Pause game time when the editor is opened (only after a delay, to give player camera chance to iniliazed after rewinding)
		bool pause = m_bPauseOnOpen;

		// Pause once on first init of if we are loading a save (account for call queue delay with 5 seconds of grace period)
		if (!pause && SCR_PersistenceSystem.IsLoadInProgress(5000))
		{
			SCR_EditorManagerEntity editorManagerEntity = SCR_EditorManagerEntity.GetInstance();
			if (editorManagerEntity)
			{
				const EEditorMode mode = editorManagerEntity.GetCurrentMode();
				if (SCR_Enum.HasFlag(mode, EEditorMode.EDIT))
					pause = true;
			}
		}

		if (pause)
			GetGame().GetCallqueue().Call(SetPause, true);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorClose()
	{
		EventProvider.DisconnectEvent(GetGame().GetSaveGameManager().OnSaveCreated, OnSaved);

		// Always unpause for when returning back to gameplay.
		const ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (world.IsGameTimePaused())
			SetPause(false);
	}

	//------------------------------------------------------------------------------------------------
	override protected void EOnEditorInit()
	{
		// Don't pause on open when disabled by CLI param in the dev version
		if (m_bPauseOnOpen && GetGame().IsDev() && System.IsCLIParam("editorDoNotPauseOnOpen"))
			m_bPauseOnOpen = false;
	}

	//------------------------------------------------------------------------------------------------
	[ReceiverAttribute()]
	protected void OnSaved(SaveGame save)
	{
		if (save.GetType() == ESaveGameType.MANUAL)
			DoPause(true);
	}
}
