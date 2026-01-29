class SCR_DeathScreenUIComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("BackMainMenu")]
	protected string m_sMainMenuButtonName;
	
	[Attribute("RestartMissionButton")]
	protected string m_sRestartButtonName;
	
	[Attribute("LoadSaveButton")]
	protected string m_sLoadSaveButtonName;
	
	//------------------------------------------------------------------------------------------------
	protected void ReturnToMenuPressed()
	{
		SCR_ConfigurableDialogUi dlg = SCR_CommonDialogs.CreateDialog("scenario_exit");
		if (!dlg)
			return;
		
		dlg.m_OnConfirm.Insert(BackToMainMenuPopupConfirm);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void BackToMainMenuPopupConfirm()
	{
		OnButtonPressed();
		GameStateTransitions.RequestGameplayEndTransition();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RestartScenarioPressed()
	{
		SCR_ConfigurableDialogUi dlg = SCR_CommonDialogs.CreateDialog("scenario_restart");
		if (!dlg)
			return;
		
		dlg.m_OnConfirm.Insert(RestartPopupConfirm);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RestartPopupConfirm()
	{
		OnButtonPressed();
		
		auto manager = GetGame().GetSaveGameManager();
		manager.StartPlaythrough(manager.GetCurrentMissionResource());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void LoadSavePressed()
	{
		SCR_ConfigurableDialogUi dlg = SCR_CommonDialogs.CreateDialog("scenario_load");
		if (!dlg)
			return;
		
		dlg.m_OnConfirm.Insert(LoadPopupConfirm);
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadPopupConfirm()
	{
		OnButtonPressed();

		auto manager = GetGame().GetSaveGameManager();
		const SaveGame activeSave = manager.GetActiveSave();
		if (activeSave)
		{
			manager.Load(activeSave);
			return;
		}

		// Restart and stay on same playthrough
		GameStateTransitions.RequestScenarioRestart();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonPressed()
	{
		ChimeraWorld world = GetGame().GetWorld();
		if (!world)
			return;
				
		if (world.IsGameTimePaused())
			world.PauseGameTime(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		Widget returnToMenuBtn = m_wRoot.FindAnyWidget(m_sMainMenuButtonName);
		if (returnToMenuBtn)
		{
			SCR_InputButtonComponent returnToMenuButton = SCR_InputButtonComponent.Cast(returnToMenuBtn.FindHandler(SCR_InputButtonComponent));
			if (returnToMenuButton)
				returnToMenuButton.m_OnActivated.Insert(ReturnToMenuPressed);
		}
		
		Widget restartBtn = m_wRoot.FindAnyWidget(m_sRestartButtonName);
		if (restartBtn)
		{
			SCR_InputButtonComponent restartButton = SCR_InputButtonComponent.Cast(restartBtn.FindHandler(SCR_InputButtonComponent));
			if (restartButton)
				restartButton.m_OnActivated.Insert(RestartScenarioPressed);
		}
		
		Widget loadSaveBtn = m_wRoot.FindAnyWidget(m_sLoadSaveButtonName);
		if (loadSaveBtn)
		{
			SCR_InputButtonComponent loadSaveButton = SCR_InputButtonComponent.Cast(loadSaveBtn.FindHandler(SCR_InputButtonComponent));
			if (loadSaveButton)
				loadSaveButton.m_OnActivated.Insert(LoadSavePressed);
		}
	}
}
