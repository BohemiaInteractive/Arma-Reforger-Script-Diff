class SCR_RallyCharacterCustomizationSubmenu : SCR_SubMenuBase
{
	protected SCR_RaceCustomizationPointEntity m_CustomizationPoint;
	protected SCR_RallyVehicleCustomizationConfig m_CustomizationConfig;

	//------------------------------------------------------------------------------------------------
	override protected void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return;

		m_CustomizationPoint = settings.GetCustomizationEntity();
		if (!m_CustomizationPoint)
			return;

		m_CustomizationConfig = m_CustomizationPoint.GetCustomizationConfig();
		if (!m_CustomizationConfig)
			return;

		array<ref SCR_RallyCharacterCustomizationSlot> characterCustomization = m_CustomizationConfig.GetCharacterCustomizationOptions();

		SetupCustomizationOptions();

		super.OnTabCreate(menuRoot, buttonsLayout, index);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupCustomizationOptions()
	{
		SCR_RallyVehicleCustomizationConfig conf = m_CustomizationPoint.GetCustomizationConfig();
		if (!conf)
			return;

		array<ref SCR_RallyCharacterCustomizationSlot> slots = conf.GetCharacterCustomizationOptions();

		string slotName;
		Widget spinbox;
		SCR_SpinBoxComponent comp;
		array<ref SCR_RallyCharacterCustomizationPart> parts = {};
		foreach (SCR_RallyCharacterCustomizationSlot slot : slots)
		{
			slotName = slot.GetSlotName();

			spinbox = m_wRoot.FindAnyWidget(slotName);
			if (!spinbox)
			{
				Print("SCR_RallyCharacterCustomizationSubmenu:: Widget for option with name: " + slotName + " not found in the layout!", LogLevel.WARNING);
				return;
			}

			comp = SCR_SpinBoxComponent.Cast(spinbox.FindHandler(SCR_SpinBoxComponent));
			if (!comp)
				return;

			parts = slot.GetAvailableParts();

			foreach (int i, SCR_RallyCharacterCustomizationPart part : parts)
			{
				if (i == 0)
					comp.AddItem(part.GetPartDisplayName());
				else
					comp.AddItem("#AR-RallyDLC-PartVariant", false, new SCR_TextFormatContainer<string>(i.ToString()));
			}

			comp.m_OnChanged.Insert(OnOptionChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnOptionChanged(SCR_SpinBoxComponent comp, int index)
	{
		//this is safe to do as it uses widgets name, not UI label, so it is unlocalized text
		string partName = comp.GetRootWidget().GetName();

		SCR_RallyVehicleCustomizationConfig conf = m_CustomizationPoint.GetCustomizationConfig();
		if (!conf)
			return;

		array<ref SCR_RallyCharacterCustomizationSlot> slots = conf.GetCharacterCustomizationOptions();

		SCR_RallyCharacterCustomizationSlot foundSlot;
		foreach (SCR_RallyCharacterCustomizationSlot slot : slots)
		{
			if (slot.GetSlotName() == partName)
			{
				foundSlot = slot;
				break;
			}
		}

		ResourceName selectedPart = foundSlot.GetAvailableParts().Get(index).GetPartPath();

		m_CustomizationPoint.SetCharacterPartByPrefab(partName, selectedPart);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTabShow()
	{
		m_CustomizationPoint.MoveCamera(SCR_ERallyCustomizationCameraPosition.CHARACTER);

		Widget hatSpinbox = m_wRoot.FindAnyWidget("Hat");
		if (hatSpinbox)
			GetGame().GetCallqueue().CallLater(GetGame().GetWorkspace().SetFocusedWidget, 0, false, hatSpinbox, false);

		super.OnTabShow();
	}
}
