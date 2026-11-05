class SCR_RAllyVehicleCustomizationSubmenu : SCR_SubMenuBase
{
	protected SCR_RaceCustomizationPointEntity m_CustomizationPoint;

	protected Widget m_wCategories;
	protected Widget m_wCosmeticCustomization;

	//------------------------------------------------------------------------------------------------
	override protected void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return;

		m_CustomizationPoint = settings.GetCustomizationEntity();
		if (!m_CustomizationPoint)
			return;

		m_wCategories = m_wRoot.FindAnyWidget("Categories");
		if (!m_wCategories)
			return;

		m_wCosmeticCustomization = m_wRoot.FindAnyWidget("CosmeticOptions");
		if (!m_wCosmeticCustomization)
			return;

		Widget button = m_wRoot.FindAnyWidget("Spoiler");
		if (!button)
			return;

		SCR_ButtonImageComponent comp = SCR_ButtonImageComponent.Cast(button.FindHandler(SCR_ButtonImageComponent));
		if (!comp)
			return;

		comp.m_OnClicked.Insert(OnSpoiler);

		button = m_wRoot.FindAnyWidget("Lights");
		if (!button)
			return;

		comp = SCR_ButtonImageComponent.Cast(button.FindHandler(SCR_ButtonImageComponent));
		if (!comp)
			return;

		comp.m_OnClicked.Insert(OnLights);

		button = m_wRoot.FindAnyWidget("Bodykit");
		if (!button)
			return;

		comp = SCR_ButtonImageComponent.Cast(button.FindHandler(SCR_ButtonImageComponent));
		if (!comp)
			return;

		comp.m_OnClicked.Insert(OnBodyKit);

		super.OnTabCreate(menuRoot, buttonsLayout, index);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSpoiler()
	{
		m_wCategories.SetEnabled(false);
		m_wCategories.SetVisible(false);
		m_wCosmeticCustomization.SetEnabled(true);
		m_wCosmeticCustomization.SetVisible(true);

		m_CustomizationPoint.MoveCamera(SCR_ERallyCustomizationCameraPosition.SPOILER);

		Widget contentWidget = m_wCosmeticCustomization.FindAnyWidget("OptionsSpinbox");
		if (!contentWidget)
			return;

		SCR_SpinBoxComponent comp = SCR_SpinBoxComponent.Cast(contentWidget.FindHandler(SCR_SpinBoxComponent));
		if (!comp)
			return;

		comp.ClearAll();

		SCR_RallyVehicleCustomizationConfig conf = m_CustomizationPoint.GetCustomizationConfig();
		if (!conf)
			return;

		array<ref SCR_RallyVehicleCustomizationData> vehicles = conf.GetVehicles();

		SCR_RallyVehicleCustomizationData selectedVehicle = vehicles.Get(m_CustomizationPoint.GetCurrentVehicleIndex());
		if (!selectedVehicle)
			return;

		array<ref SCR_RallyPartCustomizationData> parts = selectedVehicle.GetParts();

		SCR_RallyPartCustomizationData changedPart;
		foreach (SCR_RallyPartCustomizationData part : parts)
		{
			if (part.GetPartType() == SCR_ERallyCustomizationParts.SPOILER)
			{
				changedPart = part;
				break;
			}

		}

		comp.AddItem("#AR-Editor_Attribute_OverlayLogo_Default");

		foreach (ResourceName resource : changedPart.m_aAvailableParts)
		{
			comp.AddItem(FilePath.StripPath(resource));
		}

		comp.m_OnChanged.Insert(OnSpoilerSelected);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSpoilerSelected(SCR_SpinBoxComponent comp, int index)
	{
		if (index == 0)
		{
			m_CustomizationPoint.SetVehiclePartByPrefab(SCR_ERallyCustomizationParts.SPOILER, "");
			return;
		}

		SCR_RallyVehicleCustomizationConfig conf = m_CustomizationPoint.GetCustomizationConfig();
		if (!conf)
			return;

		array<ref SCR_RallyVehicleCustomizationData> vehicles = conf.GetVehicles();

		SCR_RallyVehicleCustomizationData selectedVehicle = vehicles.Get(m_CustomizationPoint.GetCurrentVehicleIndex());
		if (!selectedVehicle)
			return;

		array<ref SCR_RallyPartCustomizationData> parts = selectedVehicle.GetParts();

		SCR_RallyPartCustomizationData changedPart;
		foreach (SCR_RallyPartCustomizationData part : parts)
		{
			if (part.GetPartType() == SCR_ERallyCustomizationParts.SPOILER)
			{
				changedPart = part;
				break;
			}

		}

		ResourceName selectedPart = changedPart.m_aAvailableParts.Get(index - 1);

		m_CustomizationPoint.SetVehiclePartByPrefab(SCR_ERallyCustomizationParts.SPOILER, selectedPart);

	}

	//------------------------------------------------------------------------------------------------
	protected void OnBodyKit()
	{
		m_wCategories.SetEnabled(false);
		m_wCategories.SetVisible(false);
		m_wCosmeticCustomization.SetEnabled(true);
		m_wCosmeticCustomization.SetVisible(true);

		m_CustomizationPoint.MoveCamera(SCR_ERallyCustomizationCameraPosition.WHEEL);

		Widget contentWidget = m_wCosmeticCustomization.FindAnyWidget("OptionsSpinbox");
		if (!contentWidget)
			return;

		SCR_SpinBoxComponent comp = SCR_SpinBoxComponent.Cast(contentWidget.FindHandler(SCR_SpinBoxComponent));
		if (!comp)
			return;

		comp.ClearAll();

		SCR_RallyVehicleCustomizationConfig conf = m_CustomizationPoint.GetCustomizationConfig();
		if (!conf)
			return;

		array<ref SCR_RallyVehicleCustomizationData> vehicles = conf.GetVehicles();

		SCR_RallyVehicleCustomizationData selectedVehicle = vehicles.Get(m_CustomizationPoint.GetCurrentVehicleIndex());
		if (!selectedVehicle)
			return;

		array<ref SCR_RallyPartCustomizationData> parts = selectedVehicle.GetParts();

		SCR_RallyPartCustomizationData changedPart;
		foreach (SCR_RallyPartCustomizationData part : parts)
		{
			if (part.GetPartType() == SCR_ERallyCustomizationParts.BODYKIT)
			{
				changedPart = part;
				break;
			}

		}

		comp.AddItem("#AR-Editor_Attribute_OverlayLogo_Default");

		foreach (ResourceName resource : changedPart.m_aAvailableParts)
		{
			comp.AddItem(FilePath.StripPath(resource));
		}

		comp.m_OnChanged.Insert(OnBodykitSelected);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBodykitSelected(SCR_SpinBoxComponent comp, int index)
	{
		if (index == 0)
		{
			m_CustomizationPoint.SetVehiclePartByPrefab(SCR_ERallyCustomizationParts.BODYKIT, "");
			return;
		}

		SCR_RallyVehicleCustomizationConfig conf = m_CustomizationPoint.GetCustomizationConfig();
		if (!conf)
			return;

		array<ref SCR_RallyVehicleCustomizationData> vehicles = conf.GetVehicles();

		SCR_RallyVehicleCustomizationData selectedVehicle = vehicles.Get(m_CustomizationPoint.GetCurrentVehicleIndex());
		if (!selectedVehicle)
			return;

		array<ref SCR_RallyPartCustomizationData> parts = selectedVehicle.GetParts();

		SCR_RallyPartCustomizationData changedPart;
		foreach (SCR_RallyPartCustomizationData part : parts)
		{
			if (part.GetPartType() == SCR_ERallyCustomizationParts.BODYKIT)
			{
				changedPart = part;
				break;
			}
		}

		ResourceName selectedPart = changedPart.m_aAvailableParts.Get(index - 1);

		m_CustomizationPoint.SetVehiclePartByPrefab(SCR_ERallyCustomizationParts.BODYKIT, selectedPart);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLights()
	{
		m_wCategories.SetEnabled(false);
		m_wCategories.SetVisible(false);
		m_wCosmeticCustomization.SetEnabled(true);
		m_wCosmeticCustomization.SetVisible(true);

		Widget contentWidget = m_wCosmeticCustomization.FindAnyWidget("OptionsSpinbox");
		if (!contentWidget)
			return;

		SCR_SpinBoxComponent comp = SCR_SpinBoxComponent.Cast(contentWidget.FindHandler(SCR_SpinBoxComponent));
		if (!comp)
			return;

		comp.ClearAll();

		SCR_RallyVehicleCustomizationConfig conf = m_CustomizationPoint.GetCustomizationConfig();
		if (!conf)
			return;

		array<ref SCR_RallyVehicleCustomizationData> vehicles = conf.GetVehicles();

		SCR_RallyVehicleCustomizationData selectedVehicle = vehicles.Get(m_CustomizationPoint.GetCurrentVehicleIndex());
		if (!selectedVehicle)
			return;

		array<ref SCR_RallyPartCustomizationData> parts = selectedVehicle.GetParts();

		SCR_RallyPartCustomizationData changedPart;
		foreach (SCR_RallyPartCustomizationData part : parts)
		{
			if (part.GetPartType() == SCR_ERallyCustomizationParts.LIGHTS)
			{
				changedPart = part;
				break;
			}
		}

		comp.AddItem("#AR-Editor_Attribute_OverlayLogo_Default");

		foreach (ResourceName resource : changedPart.m_aAvailableParts)
		{
			comp.AddItem(FilePath.StripPath(resource));
		}

		comp.m_OnChanged.Insert(OnLightsSelected);

		m_CustomizationPoint.MoveCamera(SCR_ERallyCustomizationCameraPosition.LIGHTS);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLightsSelected(SCR_SpinBoxComponent comp, int index)
	{
		if (index == 0)
		{
			m_CustomizationPoint.SetVehiclePartByPrefab(SCR_ERallyCustomizationParts.LIGHTS, "");
			return;
		}

		SCR_RallyVehicleCustomizationConfig conf = m_CustomizationPoint.GetCustomizationConfig();
		if (!conf)
			return;

		array<ref SCR_RallyVehicleCustomizationData> vehicles = conf.GetVehicles();

		SCR_RallyVehicleCustomizationData selectedVehicle = vehicles.Get(m_CustomizationPoint.GetCurrentVehicleIndex());
		if (!selectedVehicle)
			return;

		array<ref SCR_RallyPartCustomizationData> parts = selectedVehicle.GetParts();

		SCR_RallyPartCustomizationData changedPart;
		foreach (SCR_RallyPartCustomizationData part : parts)
		{
			if (part.GetPartType() == SCR_ERallyCustomizationParts.LIGHTS)
			{
				changedPart = part;
				break;
			}
		}

		ResourceName selectedPart = changedPart.m_aAvailableParts[index - 1];

		m_CustomizationPoint.SetVehiclePartByPrefab(SCR_ERallyCustomizationParts.LIGHTS, selectedPart);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		m_CustomizationPoint.MoveCamera(SCR_ERallyCustomizationCameraPosition.VEHICLE);

		super.OnTabShow();
	}
}
