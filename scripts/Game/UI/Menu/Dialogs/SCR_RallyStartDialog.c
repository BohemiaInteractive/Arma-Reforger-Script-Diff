class SCR_RallyStartDialog : SCR_ConfigurableDialogUi
{
	protected ref array<string> m_aRaceNames = {};
	protected ref array<ResourceName> m_aVehicleList = {};

	protected SCR_ComboBoxComponent m_VehiclesComboComponent;
	protected SCR_ComboBoxComponent m_RacesComboComponent;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] raceSystem unused
	void SCR_RallyStartDialog(SCR_RaceSystem raceSystem)
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "rally_start", this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);

		Widget vehiclesCombo = m_wRoot.FindAnyWidget("VehiclesCombo");
		Widget racesCombo = m_wRoot.FindAnyWidget("RacesCombo");

		if (!vehiclesCombo || !racesCombo)
			return;

		m_VehiclesComboComponent = SCR_ComboBoxComponent.Cast(vehiclesCombo.FindHandler(SCR_ComboBoxComponent));
		m_RacesComboComponent = SCR_ComboBoxComponent.Cast(racesCombo.FindHandler(SCR_ComboBoxComponent));

		if (!m_VehiclesComboComponent || !m_RacesComboComponent)
			return;

		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return;

		m_aRaceNames = settings.GetAvailableRaceNames();
		m_aVehicleList = settings.GetRaceAvailableVehicles();

		foreach (string race : m_aRaceNames)
		{
			m_RacesComboComponent.AddItem(race);
		}

		foreach (ResourceName vehicle : m_aVehicleList)
		{
			m_VehiclesComboComponent.AddItem(FilePath.StripPath(vehicle.GetPath()));
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnConfirm()
	{
		super.OnConfirm();

		//change this so it gets it from system which gets it from config
		SCR_RaceEntity race = SCR_RaceEntity.Cast(GetGame().GetWorld().FindEntityByName(m_aRaceNames.Get(m_RacesComboComponent.GetCurrentIndex())));
		if (!race)
			return;

		race.StartRace(SCR_PlayerController.GetLocalPlayerId(), m_aVehicleList.Get(m_VehiclesComboComponent.GetCurrentIndex()));

		Close();
	}
}
