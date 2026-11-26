modded class PauseMenuUI
{
	protected SCR_ButtonTextComponent m_RallyRestartButton;

	//------------------------------------------------------------------------------------------------
	override protected void OnMenuOpen()
	{
		super.OnMenuOpen();

		SCR_RaceEntity race = GetActiveRallyRace();
		if (!race)
			return;

		HideRespawnButton();

		SCR_ButtonTextComponent restartComp = SCR_ButtonTextComponent.GetButtonText("Restart", m_wRoot);
		if (!restartComp)
			return;

		Widget restartButton = restartComp.GetRootWidget();
		if (!restartButton)
			return;

		Widget parent = restartButton.GetParent();
		if (!parent)
			return;

		array<ref Widget> existingButtons = {};
		SCR_WidgetHelper.GetAllChildren(parent, existingButtons);

		int insertPosition = existingButtons.Find(restartButton) + 1;
		if (insertPosition <= 0)
			return;

		int order;
		foreach (Widget existingButton : existingButtons)
		{
			if (order == insertPosition)
				order++; //Skip what will be the position of the new button

			existingButton.SetZOrder(order);
			order++;
		}

		Widget newButton = GetGame().GetWorkspace().CreateWidgets("{9ECCD201BCF07E95}UI/layouts/Menus/PauseMenu/PauseMenuButton.layout", parent);
		if (!newButton)
			return;

		newButton.SetZOrder(insertPosition);

		// Match the size/padding overrides pauseMenu.layout applies to every button instance,
		// which CreateWidgets() does not inherit since it only instantiates the base prefab.
		SizeLayoutWidget newButtonSize = SizeLayoutWidget.Cast(newButton.GetChildren());
		if (newButtonSize)
		{
			newButtonSize.SetHeightOverride(54);
			LayoutSlot.SetPadding(newButtonSize, 7, 1, 0, 1);
		}

		m_RallyRestartButton = SCR_ButtonTextComponent.Cast(newButton.FindHandler(SCR_ButtonTextComponent));
		if (!m_RallyRestartButton)
			return;

		m_RallyRestartButton.SetText("#AR-RallyDLC-PauseMenu_RestartRace");
		m_RallyRestartButton.SetEnabled(!IsPlayerEnteringOrLeavingVehicle());
		m_RallyRestartButton.m_OnClicked.Insert(OnRallyRestartRace);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the controlled character is currently getting in or out of a vehicle
	//! \return True if the character is mid getting in/out, false otherwise
	protected bool IsPlayerEnteringOrLeavingVehicle()
	{
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(pc.GetControlledEntity());
		if (!character)
			return false;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (compartmentAccess && (compartmentAccess.IsGettingIn() || compartmentAccess.IsGettingOut()))
			return true;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (controller && (controller.IsOpeningVehicleDoor() || controller.IsClosingVehicleDoor()))
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void HideRespawnButton()
	{
		SCR_ButtonTextComponent respawnComp = SCR_ButtonTextComponent.GetButtonText("Respawn", m_wRoot);
		if (!respawnComp)
			return;

		Widget respawnButton = respawnComp.GetRootWidget();
		if (!respawnButton)
			return;

		respawnButton.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_RaceEntity GetActiveRallyRace()
	{
		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return null;

		SCR_RaceCustomizationPointEntity point = settings.GetCustomizationEntity();
		if (!point)
			return null;

		return SCR_RaceEntity.Cast(GetGame().GetWorld().FindEntityByName(point.GetAssignedRace()));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRallyRestartRace()
	{
		SCR_RaceEntity race = GetActiveRallyRace();
		if (!race)
			return;

		Close();
		race.RequestRestart(SCR_PlayerController.GetLocalPlayerId());
	}
}
