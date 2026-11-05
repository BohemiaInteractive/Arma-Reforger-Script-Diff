class SCR_RallyCustomizationMenu : SCR_SuperMenuBase
{
	static ref ScriptInvokerVoid s_OnMenuOpened = new ScriptInvokerVoid();
	static ref ScriptInvokerVoid s_OnMenuClosed = new ScriptInvokerVoid();

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		s_OnMenuOpened.Invoke();

		//guard against master volume being left faded out by a finish-screen fade
		//that never got restored (e.g. session stopped mid-fade in a previous scenario run)
		AudioSystem.SetMasterVolume(AudioSystem.SFX, 1);
		AudioSystem.SetMasterVolume(AudioSystem.Dialog, 1);
		AudioSystem.SetMasterVolume(AudioSystem.Music, 1);
		AudioSystem.SetMasterVolume(AudioSystem.UI, 1);
		AudioSystem.SetMasterVolume(AudioSystem.VoiceChat, 1);

		Widget confirm = GetRootWidget().FindAnyWidget("ConfirmCustomization");
		if (!confirm)
			return;

		SCR_InputButtonComponent comp = SCR_InputButtonComponent.Cast(confirm.FindHandler(SCR_InputButtonComponent));
		if (!comp)
			return;

		comp.m_OnActivated.Insert(OnRaceConfirmed);

		Widget pauseButton = GetRootWidget().FindAnyWidget("PauseButton");
		if (!pauseButton)
			return;

		SCR_InputButtonComponent pauseComp = SCR_InputButtonComponent.Cast(pauseButton.FindHandler(SCR_InputButtonComponent));
		if (!pauseComp)
			return;

		pauseComp.m_OnActivated.Insert(OnPauseMenu);

		UpdateTitle();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateTitle()
	{
		TextWidget title = TextWidget.Cast(GetRootWidget().FindAnyWidget("TitlePage"));
		if (!title)
			return;

		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return;

		SCR_RaceCustomizationPointEntity point = settings.GetCustomizationEntity();
		if (!point)
			return;

		SCR_RaceEntity race = SCR_RaceEntity.Cast(GetGame().GetWorld().FindEntityByName(point.GetAssignedRace()));
		if (!race)
			return;

		title.SetText(race.GetRaceName());
	}

	//------------------------------------------------------------------------------------------------
	void OnPauseMenu()
	{
		MenuBase menu = GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.PauseMenu, 0, true, false);

		PauseMenuUI pauseMenu = PauseMenuUI.Cast(menu);
		if (pauseMenu)
			pauseMenu.FadeBackground(true, true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		s_OnMenuClosed.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	void OnRaceConfirmed()
	{
		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return;

		SCR_RaceCustomizationPointEntity point = settings.GetCustomizationEntity();
		if (!point)
			return;

		point.OnCustomizationConfirmed();

		Close();
	}
}

//! Menu presets
modded enum ChimeraMenuPreset
{
	RallyDLC_CustomizationMenu,
	RallyDLC_GameOverScreen
}
