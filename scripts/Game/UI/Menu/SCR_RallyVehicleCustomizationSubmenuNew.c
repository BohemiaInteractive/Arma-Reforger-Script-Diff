class SCR_RAllyVehicleCustomizationSubmenuNew : SCR_SubMenuBase
{
	protected SCR_RaceCustomizationPointEntity m_CustomizationPoint;
	protected ref SCR_TabViewComponent m_CustomizationTabView;
	protected SCR_ERallyCustomizationParts m_CurrentPart;
	protected Vehicle m_CurrentPreviewVehicle;
	protected ref map<SCR_ERallyCustomizationParts, ResourceName> m_mDefaultParts = new map<SCR_ERallyCustomizationParts, ResourceName>();
	protected ref map<SCR_ERallyCustomizationParts, int> m_mSelectedVariantIndexes = new map<SCR_ERallyCustomizationParts, int>();

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

		SCR_RallyVehicleCustomizationConfig config = m_CustomizationPoint.GetCustomizationConfig();
		if (!config)
		{
			Print("SCR_RAllyVehicleCustomizationSubmenuNew:: Missing config for vehicle customization!", LogLevel.ERROR);
			return;
		}

		array<ref SCR_RallyVehicleCustomizationData> vehicles = config.GetVehicles();
		SCR_RallyVehicleCustomizationData vehicle = vehicles[m_CustomizationPoint.m_iCurrentPreviewVehicleIndex];
		if (!vehicle)
			return;

		LoadMemberWidgets();
		UpdateVehicleTexts(vehicle);

		Widget tabViewWidget = m_wRoot.FindAnyWidget("CustomizationTabview");
		if (tabViewWidget)
		{
			m_CustomizationTabView = SCR_TabViewComponent.Cast(tabViewWidget.FindHandler(SCR_TabViewComponent));
			if (m_CustomizationTabView)
			{
				m_CustomizationTabView.GetOnTabChange().Insert(OnCustomizationTabChange);
			}
			else
			{
				Print("SCR_RAllyVehicleCustomizationSubmenuNew:: CustomizationTabview has no SCR_TabViewComponent!", LogLevel.ERROR);
			}
		}
		else
		{
			Print("SCR_RAllyVehicleCustomizationSubmenuNew:: CustomizationTabview widget not found!", LogLevel.ERROR);
		}

		super.OnTabCreate(menuRoot, buttonsLayout, index);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] tabView
	//! \param[in] widget
	protected void OnCustomizationTabChange(SCR_TabViewComponent tabView, Widget widget)
	{
		SCR_TabViewContent content = tabView.GetShownTabComponent();
		if (!content)
			return;

		string identifier = content.m_sTabIdentifier;
		identifier.ToLower();

		if (identifier == "spoiler")
		{
			SwapCamera(SCR_ERallyCustomizationParts.SPOILER);
			PopulateVariantSpinbox(SCR_ERallyCustomizationParts.SPOILER);
			return;
		}

		if (identifier == "bodykit")
		{
			SwapCamera(SCR_ERallyCustomizationParts.BODYKIT);
			PopulateVariantSpinbox(SCR_ERallyCustomizationParts.BODYKIT);
			return;
		}

		if (identifier == "lights")
		{
			SwapCamera(SCR_ERallyCustomizationParts.LIGHTS);
			PopulateVariantSpinbox(SCR_ERallyCustomizationParts.LIGHTS);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Populates the VariantSpinbox with the available part options for the given part type,
	//! based on the currently previewed vehicle's config data.
	//! \param[in] part
	protected void PopulateVariantSpinbox(SCR_ERallyCustomizationParts part)
	{
		m_CurrentPart = part;

		if (!m_CustomizationPoint)
			return;

		Widget contentWidget = m_wRoot.FindAnyWidget("VariantSpinbox");
		if (!contentWidget)
			return;

		SCR_SpinBoxComponent comp = SCR_SpinBoxComponent.Cast(contentWidget.FindHandler(SCR_SpinBoxComponent));
		if (!comp)
			return;

		comp.m_OnChanged.Remove(OnVariantSelected);
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
		foreach (SCR_RallyPartCustomizationData partData : parts)
		{
			if (partData.GetPartType() == part)
			{
				changedPart = partData;
				break;
			}
		}

		if (!changedPart)
			return;

		// Capture the part the vehicle had before any customization, once per part,
		// so it can be restored later by picking the first spinbox entry.
		if (!m_mDefaultParts.Contains(part))
			m_mDefaultParts.Set(part, GetAttachedPartResource(part));

		comp.AddItem("#AR-Editor_Attribute_OverlayLogo_Default");

		foreach (int i, ResourceName resource : changedPart.m_aAvailableParts)
		{
			comp.AddItem("#AR-RallyDLC-PartVariant", false, new SCR_TextFormatContainer<int>(i + 1));
		}

		comp.SetCurrentItem(GetSelectedVariantIndex(part, changedPart));

		comp.m_OnChanged.Insert(OnVariantSelected);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the spinbox index that matches the currently attached part
	//! \param[in] part
	//! \param[in] partData
	protected int GetSelectedVariantIndex(SCR_ERallyCustomizationParts part, notnull SCR_RallyPartCustomizationData partData)
	{
		if (!m_CustomizationPoint.GetCurrentPreviewVehicle())
			return 0;

		ResourceName attachedPart = GetAttachedPartResource(part);

		ResourceName defaultPart;
		bool hasDefault = m_mDefaultParts.Find(part, defaultPart);

		// Use the remembered selected index if it still matches the part that is currently attached to the vehicle
		int selectedIndex;
		if (m_mSelectedVariantIndexes.Find(part, selectedIndex))
		{
			if (selectedIndex == 0 && hasDefault && attachedPart == defaultPart)
				return 0;

			if (selectedIndex > 0 && selectedIndex <= partData.m_aAvailableParts.Count() && partData.m_aAvailableParts[selectedIndex - 1] == attachedPart)
				return selectedIndex;
		}

		// If the currently attached part does not match the part assigned to the remembered selected index, find the index that matches it
		if (hasDefault && attachedPart == defaultPart)
			return 0;

		foreach (int i, ResourceName resource : partData.m_aAvailableParts)
		{
			if (resource == attachedPart)
				return i + 1;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Reads the ResourceName of whatever entity is currently attached in the vehicle's slot
	//! for the given part, straight off the vehicle prefab (before any customization changes it).
	//! \param[in] part
	protected ResourceName GetAttachedPartResource(SCR_ERallyCustomizationParts part)
	{
		Vehicle previewVehicle = m_CustomizationPoint.GetCurrentPreviewVehicle();
		if (!previewVehicle)
			return string.Empty;

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(previewVehicle.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return string.Empty;

		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);

		string slotName = GetSlotNameForPart(part);

		foreach (EntitySlotInfo slot : slots)
		{
			if (slot.GetSourceName() != slotName)
				continue;

			IEntity attachedEntity = slot.GetAttachedEntity();
			if (!attachedEntity)
				return string.Empty;

			return attachedEntity.GetPrefabData().GetPrefabName();
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] part
	protected string GetSlotNameForPart(SCR_ERallyCustomizationParts part)
	{
		switch (part)
		{
			case SCR_ERallyCustomizationParts.SPOILER:
				return "Spoiler";

			case SCR_ERallyCustomizationParts.BODYKIT:
				return "BodyKit";

			case SCR_ERallyCustomizationParts.LIGHTS:
				return "RallyLights";
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] comp
	//! \param[in] index
	protected void OnVariantSelected(SCR_SpinBoxComponent comp, int index)
	{
		if (!m_CustomizationPoint)
			return;

		m_mSelectedVariantIndexes.Set(m_CurrentPart, index);

		if (index == 0)
		{
			m_CustomizationPoint.SetVehiclePartByPrefab(m_CurrentPart, m_mDefaultParts.Get(m_CurrentPart));
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
		foreach (SCR_RallyPartCustomizationData partData : parts)
		{
			if (partData.GetPartType() == m_CurrentPart)
			{
				changedPart = partData;
				break;
			}
		}

		if (!changedPart)
			return;

		ResourceName selectedPart = changedPart.m_aAvailableParts.Get(index - 1);

		m_CustomizationPoint.SetVehiclePartByPrefab(m_CurrentPart, selectedPart);
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshVehicleInfo()
	{
		if (!m_CustomizationPoint)
			return;

		SCR_RallyVehicleCustomizationConfig config = m_CustomizationPoint.GetCustomizationConfig();
		if (!config)
		{
			Print("SCR_RAllyVehicleCustomizationSubmenuNew:: Missing config for vehicle customization!", LogLevel.ERROR);
			return;
		}

		array<ref SCR_RallyVehicleCustomizationData> vehicles = config.GetVehicles();
		SCR_RallyVehicleCustomizationData vehicle = vehicles[m_CustomizationPoint.m_iCurrentPreviewVehicleIndex];
		if (!vehicle)
			return;

		Vehicle previewVehicle = m_CustomizationPoint.GetCurrentPreviewVehicle();
		if (previewVehicle && (previewVehicle != m_CurrentPreviewVehicle))
		{
			m_CurrentPreviewVehicle = previewVehicle;
			m_mDefaultParts.Clear();
			m_mSelectedVariantIndexes.Clear();
		}

		UpdateVisualAttributes(vehicle);
		UpdateVehicleTexts(vehicle);
		UpdateTabAvailability(vehicle);
	}

	//------------------------------------------------------------------------------------------------
	//! Disables tabs (e.g. Bodykit/Spoiler/Lights) for parts that have no available options
	//! in the config for the currently previewed vehicle.
	//! \param[in] vehicle
	protected void UpdateTabAvailability(notnull SCR_RallyVehicleCustomizationData vehicle)
	{
		if (!m_CustomizationTabView)
			return;

		array<ref SCR_RallyPartCustomizationData> parts = vehicle.GetParts();

		int availableTabCount = 0;
		int availableTabIndex = -1;

		array<ref SCR_TabViewContent> tabs = m_CustomizationTabView.GetContents();
		foreach (int i, SCR_TabViewContent tabContent : tabs)
		{
			SCR_ERallyCustomizationParts partType;
			switch (tabContent.m_sTabIdentifier)
			{
				case "bodykit":
					partType = SCR_ERallyCustomizationParts.BODYKIT;
					break;

				case "spoiler":
					partType = SCR_ERallyCustomizationParts.SPOILER;
					break;

				case "lights":
					partType = SCR_ERallyCustomizationParts.LIGHTS;
					break;

				default:
					continue;
			}

			bool hasOptions = false;
			foreach (SCR_RallyPartCustomizationData partData : parts)
			{
				if (partData.GetPartType() == partType && partData.m_aAvailableParts && !partData.m_aAvailableParts.IsEmpty())
				{
					hasOptions = true;
					break;
				}
			}

			m_CustomizationTabView.EnableTab(i, hasOptions);

			if (hasOptions)
			{
				availableTabCount++;
				availableTabIndex = i;
			}

			if (partType == m_CurrentPart)
			{
				Widget variantSpinboxWidget = m_wRoot.FindAnyWidget("VariantSpinbox");
				if (variantSpinboxWidget)
					variantSpinboxWidget.SetEnabled(hasOptions);
			}
		}

		// Only one part can be customized on this vehicle (e.g. just lights) - select its tab automatically
		if (availableTabCount == 1 && m_CustomizationTabView.GetShownTab() != availableTabIndex)
		{
			m_CustomizationTabView.ShowTab(availableTabIndex, true, false);

			Widget spinboxWidget = m_wRoot.FindAnyWidget("VariantSpinbox");
			if (spinboxWidget)
				spinboxWidget.SetEnabled(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadMemberWidgets()
	{
		m_wSpeedProgress = ProgressBarWidget.Cast(m_wRoot.FindAnyWidget("SpeedProgress"));
		if (!m_wSpeedProgress)
			Print("SCR_RAllyVehicleCustomizationSubmenuNew:: SpeedProgress widget not found!", LogLevel.ERROR);

		m_wAccelerationProgress = ProgressBarWidget.Cast(m_wRoot.FindAnyWidget("AccelerationProgress"));
		if (!m_wAccelerationProgress)
			Print("SCR_RAllyVehicleCustomizationSubmenuNew:: AccelerationProgress widget not found!", LogLevel.ERROR);

		m_wHandlingProgress = ProgressBarWidget.Cast(m_wRoot.FindAnyWidget("HandlingProgress"));
		if (!m_wHandlingProgress)
			Print("SCR_RAllyVehicleCustomizationSubmenuNew:: HandlingProgress widget not found!", LogLevel.ERROR);

		m_wWeightProgress = ProgressBarWidget.Cast(m_wRoot.FindAnyWidget("WeightProgress"));
		if (!m_wWeightProgress)
			Print("SCR_RAllyVehicleCustomizationSubmenuNew:: WeightProgress widget not found!", LogLevel.ERROR);
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
	//!
	//! \param[in] part
	protected void SwapCamera(SCR_ERallyCustomizationParts part)
	{
		if (!m_CustomizationPoint)
			return;

		SCR_ERallyCustomizationCameraPosition cameraPosition;

		switch (part)
		{
			case SCR_ERallyCustomizationParts.SPOILER:
				cameraPosition = SCR_ERallyCustomizationCameraPosition.SPOILER;
				break;

			case SCR_ERallyCustomizationParts.BODYKIT:
				cameraPosition = SCR_ERallyCustomizationCameraPosition.WHEEL;
				break;

			case SCR_ERallyCustomizationParts.LIGHTS:
				cameraPosition = SCR_ERallyCustomizationCameraPosition.LIGHTS;
				break;
		}

		m_CustomizationPoint.MoveCamera(cameraPosition);
	}

	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		RefreshVehicleInfo();

		if (m_CustomizationTabView)
		{
			OnCustomizationTabChange(m_CustomizationTabView, null);
			GetGame().GetCallqueue().CallLater(FocusCustomizationTabView, 0, false);
		}

		super.OnTabShow();
	}

	//------------------------------------------------------------------------------------------------
	protected void FocusCustomizationTabView()
	{
		Widget variantSpinboxWidget = m_wRoot.FindAnyWidget("VariantSpinbox");
		if (variantSpinboxWidget && variantSpinboxWidget.IsFocusable())
			GetGame().GetWorkspace().SetFocusedWidget(variantSpinboxWidget);
		else
			GetGame().GetWorkspace().SetFocusedWidget(null);
	}
}
