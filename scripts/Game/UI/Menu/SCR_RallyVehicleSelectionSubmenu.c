class SCR_RallyVehicleSelectionSubmenu : SCR_SubMenuBase
{
	protected SCR_RaceCustomizationPointEntity m_CustomizationPoint;

	protected ProgressBarWidget m_wSpeedProgress;
	protected ProgressBarWidget m_wAccelerationProgress;
	protected ProgressBarWidget m_wHandlingProgress;
	protected ProgressBarWidget m_wWeightProgress;

	//------------------------------------------------------------------------------------------------
	override protected void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		SCR_AdditionalGameModeSettingsComponent settings = SCR_AdditionalGameModeSettingsComponent.GetInstance();
		if (!settings)
			return;

		m_CustomizationPoint = settings.GetCustomizationEntity();
		if (!m_CustomizationPoint)
			return;

		Widget vehicleSpinbox = m_wRoot.FindAnyWidget("VehicleSpinbox");
		if (!vehicleSpinbox)
			return;

		SCR_SpinBoxComponent spinboxcomp = SCR_SpinBoxComponent.Cast(vehicleSpinbox.FindHandler(SCR_SpinBoxComponent));
		if (!spinboxcomp)
			return;

		SCR_RallyVehicleCustomizationConfig config = m_CustomizationPoint.GetCustomizationConfig();
		if (!config)
		{
			Print("SCR_RallyCustomizationMenu:: Missing config for vehicle customization!", LogLevel.ERROR);
			return;
		}

		array<ref SCR_RallyVehicleCustomizationData> vehicles = config.GetVehicles();
		foreach (SCR_RallyVehicleCustomizationData vehicle : vehicles)
		{
			spinboxcomp.AddItem(vehicle.m_sVehicleBrandName);
		}

		spinboxcomp.m_OnChanged.Insert(OnVehicleSelectionChanged);

		Widget vehicleVariantWidget = m_wRoot.FindAnyWidget("VehicleVariantSpinbox");
		if (!vehicleVariantWidget)
			return;

		SCR_SpinBoxComponent spinboxVariantComp = SCR_SpinBoxComponent.Cast(vehicleVariantWidget.FindHandler(SCR_SpinBoxComponent));
		if (!spinboxVariantComp)
			return;

		spinboxVariantComp.m_OnChanged.Insert(OnVehicleVariantChanged);

		UpdateVariantsSpinbox(m_CustomizationPoint.m_iCurrentPreviewVehicleIndex);

		spinboxcomp.SetCurrentItem(m_CustomizationPoint.m_iCurrentPreviewVehicleIndex, invokeOnChanged: false);
		spinboxVariantComp.SetCurrentItem(m_CustomizationPoint.m_iCurrentPreviewVehicleVariantIndex, invokeOnChanged: false);

		LoadMemberWidgets();
		UpdateVisualAttributes(vehicles[m_CustomizationPoint.m_iCurrentPreviewVehicleIndex]);
		UpdateVehicleTexts(vehicles[m_CustomizationPoint.m_iCurrentPreviewVehicleIndex]);
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadMemberWidgets()
	{
		m_wSpeedProgress = ProgressBarWidget.Cast(m_wRoot.FindAnyWidget("SpeedProgress"));
		if (!m_wSpeedProgress)
			Print("SCR_RallyVehicleSelectionSeubMenu:: SpeedProgress widget not found!", LogLevel.ERROR);

		m_wAccelerationProgress = ProgressBarWidget.Cast(m_wRoot.FindAnyWidget("AccelerationProgress"));
		if (!m_wAccelerationProgress)
			Print("SCR_RallyVehicleSelectionSeubMenu:: AccelerationProgress widget not found!", LogLevel.ERROR);

		m_wHandlingProgress = ProgressBarWidget.Cast(m_wRoot.FindAnyWidget("HandlingProgress"));
		if (!m_wHandlingProgress)
			Print("SCR_RallyVehicleSelectionSeubMenu:: HandlingProgress widget not found!", LogLevel.ERROR);

		m_wWeightProgress = ProgressBarWidget.Cast(m_wRoot.FindAnyWidget("WeightProgress"));
		if (!m_wWeightProgress)
			Print("SCR_RallyVehicleSelectionSeubMenu:: WeightProgress widget not found!", LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] comp
	//! \param[in] index
	protected void OnVehicleSelectionChanged(SCR_SpinBoxComponent comp, int index)
	{
		UpdateVariantsSpinbox(index);

		SCR_RallyVehicleCustomizationConfig config = m_CustomizationPoint.GetCustomizationConfig();
		if (!config)
		{
			Print("SCR_RallyCustomizationMenu:: Missing config for vehicle customization!", LogLevel.ERROR);
			return;
		}

		array<ref SCR_RallyVehicleCustomizationData> vehicles = {};
		vehicles = config.GetVehicles();

		m_CustomizationPoint.m_iCurrentPreviewVehicleVariantIndex = 0;
		m_CustomizationPoint.SpawnVehicle();

		SCR_RallyVehicleCustomizationData vehicle = vehicles[index];
		if (!vehicle)
			return;

		UpdateVisualAttributes(vehicle);
		UpdateVehicleTexts(vehicle);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] vehicle
	protected void UpdateVisualAttributes(notnull SCR_RallyVehicleCustomizationData vehicle)
	{
		SCR_RallyVehicleVisualAttributes attributes = vehicle.GetVisualAttributes();
		if (!attributes)
			return;

		m_wSpeedProgress.SetCurrent(attributes.GetSpeed() * 0.1);
		m_wAccelerationProgress.SetCurrent(attributes.GetAcceleration() * 0.1);
		m_wHandlingProgress.SetCurrent(attributes.GetHandling() * 0.1);
		m_wWeightProgress.SetCurrent(attributes.GetWeight() * 0.1);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] index
	protected void UpdateVariantsSpinbox(int index)
	{
		m_CustomizationPoint.m_iCurrentPreviewVehicleIndex = index;
		Widget vehicleVariantWidget = m_wRoot.FindAnyWidget("VehicleVariantSpinbox");
		if (!vehicleVariantWidget)
			return;

		SCR_SpinBoxComponent spinboxcomp = SCR_SpinBoxComponent.Cast(vehicleVariantWidget.FindHandler(SCR_SpinBoxComponent));
		if (!spinboxcomp)
			return;

		spinboxcomp.ClearAll();

		SCR_RallyVehicleCustomizationConfig config = m_CustomizationPoint.GetCustomizationConfig();
		if (!config)
		{
			Print("SCR_RallyCustomizationMenu:: Missing config for vehicle customization!", LogLevel.ERROR);
			return;
		}

		array<ref SCR_RallyVehicleCustomizationData> vehicles = {};
		vehicles = config.GetVehicles();

		array<ResourceName> vehicleVariants = vehicles[index].m_sVehicleVariants;

		foreach (int i, ResourceName car : vehicleVariants)
		{
			spinboxcomp.AddItem("#AR-RallyDLC-PartVariant", false, new SCR_TextFormatContainer<int>(i + 1));
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] vehicleData
	protected void UpdateVehicleTexts(notnull SCR_RallyVehicleCustomizationData vehicleData)
	{
		RichTextWidget brandName = RichTextWidget.Cast(m_wRoot.FindAnyWidget("BrandName"));
		if (!brandName)
			return;

		RichTextWidget modelName = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ModelName"));
		if (!modelName)
			return;

		RichTextWidget modelDescription = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ModelDesc"));
		if (!modelDescription)
			return;

		brandName.SetText(vehicleData.m_sVehicleBrandName);
		modelName.SetText(vehicleData.m_sVehicleTypeName);
		modelDescription.SetText(vehicleData.m_sVehicleDescription);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] comp
	//! \param[in] index
	protected void OnVehicleVariantChanged(SCR_SpinBoxComponent comp, int index)
	{
		m_CustomizationPoint.m_iCurrentPreviewVehicleVariantIndex = index;
		m_CustomizationPoint.SpawnVehicle();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnTabShow()
	{
		m_CustomizationPoint.MoveCamera(SCR_ERallyCustomizationCameraPosition.VEHICLE);

		Widget vehicleSpinbox = m_wRoot.FindAnyWidget("VehicleSpinbox");
		if (vehicleSpinbox)
			GetGame().GetCallqueue().CallLater(GetGame().GetWorkspace().SetFocusedWidget, 0, false, vehicleSpinbox, false);

		super.OnTabShow();
	}
}
