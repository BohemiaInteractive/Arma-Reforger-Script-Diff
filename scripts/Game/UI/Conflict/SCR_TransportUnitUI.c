class SCR_TransportUnitUI : SCR_ScriptedWidgetComponent
{
	[Attribute("TransportUnitName")]
	protected string m_sTransportUnitNameWidgetName;

	[Attribute("VehicleActivityIcon")]
	protected string m_sVehicleActivityIconWidgetName;

	[Attribute("DestinationName")]
	protected string m_sDestinationNameWidgetName;

	[Attribute("VehicleConditionIcon")]
	protected string m_sVehicleConditionIconWidgetName;

	[Attribute("SourceBaseComboBox")]
	protected string m_sSourceBaseComboBoxWidgetName;

	[Attribute("ModularButton")]
	protected string m_sModularButtonWidgetName;

	[Attribute("{357AD0CC75BA2313}UI/Textures/LogisticTeamState.imageset", UIWidgets.ResourceNamePicker, params: "imageset")]
	protected ResourceName m_sTransportUnitImageStateResourceName;

	[Attribute("Waiting", desc: "Used for every other resupply task solver state.")]
	protected string m_sWaitingVehicleActivityImageName;

	[Attribute("OnRoadForward")]
	protected string m_sForwardArrows;

	[Attribute("MissingVehicle")]
	protected string m_sMissingVehicleConditionImageName;

	[Attribute("LoadedFull")]
	protected string m_sFullyLoadedVehicleConditionImageName;

	[Attribute("LoadedEmpty")]
	protected string m_sNotLoadedVehicleConditionImageName;

	[Attribute("0.98 0.26 0.26 1", UIWidgets.ColorPicker)]
	protected ref Color m_FaultStateColor;

	[Attribute(UIColors.GetColorAttribute(GUIColors.ENABLED), UIWidgets.ColorPicker)]
	protected ref Color m_NormalStateColor;

	protected SCR_TransportUnitComponent m_TransportUnit;

	protected TextWidget m_wTransportUnitName;
	protected ImageWidget m_wVehicleActivityIcon;
	protected ImageWidget m_wVehicleConditionIcon;
	protected TextWidget m_wDestinationName;
	protected SCR_ModularButtonComponent m_wModularButton;

	protected SCR_ComboBoxComponent m_SourceBaseComboBox;
	protected ref array<Widget> m_mComboBoxWidgets = {};

	protected ref SCR_ResourceSystemSubscriptionHandleBase m_ResourceSubscriptionHandleConsumer;
	protected RplId m_ResourceInventoryPlayerComponentRplId;
	protected SCR_ResourceComponent m_VehicleResourceComponent;

	protected SCR_VehicleConditionManager m_VehicleConditionManager;

	//------------------------------------------------------------------------------------------------
	void Init(SCR_TransportUnitComponent transportUnit)
	{
		m_TransportUnit = transportUnit;

		if (!m_TransportUnit)
			return;

		m_VehicleConditionManager = m_TransportUnit.GetVehicleConditionManager();
		if (!m_VehicleConditionManager)
			return;

		SetName(m_TransportUnit.GetAIGroup());
		UpdateVehicleActivityIcon();
		OnVehicleChanged(m_TransportUnit.GetVehicle(), null);
		SetupComboBox();

		OnResupplyTakSolverStateChanged(m_TransportUnit.GetResupplyTaskSolverState());
		m_VehicleConditionManager.GetOnVehicleUsabilityChanged().Insert(OnVehicleUsabilityChanged);
		m_TransportUnit.GetOnVehicleChanged().Insert(OnVehicleChanged);
		m_TransportUnit.GetOnResupplyTaskSolverStateChanged().Insert(OnResupplyTakSolverStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	void Deinit()
	{
		if (m_SourceBaseComboBox)
		{
			m_SourceBaseComboBox.m_OnChanged.Remove(OnSourceBaseChanged);
			m_SourceBaseComboBox.m_OnOpened.Remove(OnComboBoxListOpened);
			m_SourceBaseComboBox.m_OnClosed.Remove(OnComboBoxListClosed);
		}

		if (m_TransportUnit)
		{
			m_TransportUnit.GetOnStateChanged().Remove(OnTransportUnitStateChanged);
			m_TransportUnit.GetOnVehicleChanged().Remove(OnVehicleChanged);
			m_TransportUnit.GetOnResupplyTaskSolverStateChanged().Remove(OnResupplyTakSolverStateChanged);

			SCR_ResourceConsumer consumer = GetVehicleSuppliesConsumer(m_TransportUnit.GetVehicle());
			if (consumer)
				consumer.GetOnResourcesChanged().Remove(OnVehicleResourcesChanged);
		}

		if (m_VehicleConditionManager)
			m_VehicleConditionManager.GetOnVehicleUsabilityChanged().Remove(OnVehicleUsabilityChanged);

		if (m_VehicleResourceComponent)
			m_VehicleResourceComponent.TEMP_GetOnInteractorReplicated().Remove(OnInteractorReplicated);

		m_ResourceSubscriptionHandleConsumer = null;
	}

	//------------------------------------------------------------------------------------------------
	void FocusButton()
	{
		if (!m_wModularButton)
			return;

		GetGame().GetWorkspace().SetFocusedWidget(m_wModularButton.GetRootWidget());
	}

	//------------------------------------------------------------------------------------------------
	protected bool SetTexture(ImageWidget widget, ResourceName texture, string image = "", Color color = null)
	{
		if (!widget || texture == ResourceName.Empty)
			return false;

		bool success;
		if (texture.EndsWith(".edds"))
			success = widget.LoadImageTexture(0, texture);
		else
			success = widget.LoadImageFromSet(0, texture, image);

		if (success && color)
			widget.SetColor(color);

		return success;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVehicleChanged(Vehicle vehicle, Vehicle previousVehicle)
	{
		if (previousVehicle)
		{
			SCR_ResourceConsumer consumer = GetVehicleSuppliesConsumer(previousVehicle);
			if (consumer)
				consumer.GetOnResourcesChanged().Remove(OnVehicleResourcesChanged);

			m_VehicleResourceComponent.TEMP_GetOnInteractorReplicated().Remove(OnInteractorReplicated);
		}

		UpdateVehicleConditionIcon(vehicle);

		if (!vehicle)
			return;

		SCR_ResourceConsumer consumer = GetVehicleSuppliesConsumer(vehicle);
		if (!consumer)
			return;

		if (!m_ResourceInventoryPlayerComponentRplId || !m_ResourceInventoryPlayerComponentRplId.IsValid())
			m_ResourceInventoryPlayerComponentRplId = Replication.FindId(SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent)));

		m_ResourceSubscriptionHandleConsumer = GetGame().GetResourceSystemSubscriptionManager().RequestSubscriptionListenerHandle(consumer, m_ResourceInventoryPlayerComponentRplId);

		consumer.GetOnResourcesChanged().Insert(OnVehicleResourcesChanged);

		m_VehicleResourceComponent = SCR_ResourceComponent.FindResourceComponent(vehicle);
		if (!m_VehicleResourceComponent)
			return;

		m_VehicleResourceComponent.TEMP_GetOnInteractorReplicated().Insert(OnInteractorReplicated);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInteractorReplicated()
	{
		UpdateVehicleConditionIcon(m_TransportUnit.GetVehicle());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVehicleResourcesChanged(SCR_ResourceInteractor interactor, float previousValue)
	{
		UpdateVehicleConditionIconBasedOnResources(SCR_ResourceConsumer.Cast(interactor));
	}

	protected void OnVehicleUsabilityChanged(bool isUsable)
	{
		UpdateVehicleConditionIcon(m_TransportUnit.GetVehicle());
	}

	//------------------------------------------------------------------------------------------------
	protected bool UpdateVehicleConditionIconBasedOnDamage()
	{
		SCR_VehicleConditionCheck conditionCheck = m_VehicleConditionManager.GetFirstFailedCheck(m_TransportUnit.GetVehicle());
		if (!conditionCheck)
			return false;

		SCR_UIInfo failedConditionUIInfo = conditionCheck.GetFailedConditionUIInfo();
		if (!failedConditionUIInfo)
			return false;

		if (failedConditionUIInfo.SetIconTo(m_wVehicleConditionIcon))
			m_wVehicleConditionIcon.SetColor(m_FaultStateColor);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateVehicleConditionIconBasedOnResources(SCR_ResourceConsumer consumer)
	{
		if (m_TransportUnit.GetSolver().GetResupplyTaskSolverState() != SCR_EResupplyTaskSolverState.LOADING_SUPPLIES &&
			(consumer && consumer.GetAggregatedResourceValue() > 0))
		{
			SetTexture(m_wVehicleConditionIcon, m_sTransportUnitImageStateResourceName, m_sFullyLoadedVehicleConditionImageName, m_NormalStateColor);
		}
		else
		{
			SetTexture(m_wVehicleConditionIcon, m_sTransportUnitImageStateResourceName, m_sNotLoadedVehicleConditionImageName, m_NormalStateColor);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTransportUnitStateChanged(SCR_ETransportUnitState transportUnitState)
	{
		UpdateVehicleActivityIcon();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateVehicleActivityIcon()
	{
		SetVehicleActivityIcon(m_TransportUnit.GetResupplyTaskSolverState());
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateVehicleConditionIcon(Vehicle vehicle)
	{
		if (vehicle)
		{
			bool vehicleConditionIconUpdated = UpdateVehicleConditionIconBasedOnDamage();
			if (vehicleConditionIconUpdated)
				return;

			UpdateVehicleConditionIconBasedOnResources(GetVehicleSuppliesConsumer(vehicle));
		}
		else
		{
			SetTexture(m_wVehicleConditionIcon, m_sTransportUnitImageStateResourceName, m_sMissingVehicleConditionImageName, m_FaultStateColor);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ResourceConsumer GetVehicleSuppliesConsumer(Vehicle vehicle)
	{
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(vehicle);
		if (!resourceComponent)
			return null;

		return resourceComponent.GetConsumer(EResourceGeneratorID.VEHICLE_UNLOAD, EResourceType.SUPPLIES);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnResupplyTakSolverStateChanged(SCR_EResupplyTaskSolverState resupplyTaskSolverState)
	{
		SetVehicleActivityIcon(resupplyTaskSolverState);
		SetVehicleDestinationName(resupplyTaskSolverState);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetVehicleActivityIcon(SCR_EResupplyTaskSolverState resupplyTaskSolverState)
	{
		if (resupplyTaskSolverState == SCR_EResupplyTaskSolverState.MOVING_TO_SUPPLIED_BASE || resupplyTaskSolverState == SCR_EResupplyTaskSolverState.MOVING_TO_SOURCE_BASE)
		{
			SetTexture(m_wVehicleActivityIcon, m_sTransportUnitImageStateResourceName, m_sForwardArrows);
		}
		else
		{
			SetTexture(m_wVehicleActivityIcon, m_sTransportUnitImageStateResourceName, m_sWaitingVehicleActivityImageName);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetVehicleDestinationName(SCR_EResupplyTaskSolverState resupplyTaskSolverState)
	{
		if (!m_wDestinationName || !m_TransportUnit)
			return;

		m_wDestinationName.SetText(m_TransportUnit.GetDestinationName());
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupComboBox()
	{
		SCR_CampaignMilitaryBaseManager baseManager = SCR_GameModeCampaign.GetInstance().GetBaseManager();
		array<SCR_CampaignMilitaryBaseComponent> bases = {};
		baseManager.GetBases(bases, m_TransportUnit.GetFaction());

		int selectedIndex = -1;
		m_SourceBaseComboBox.ClearAll();
		m_SourceBaseComboBox.AddItem("-");
		foreach (int index, SCR_MilitaryBaseComponent base : bases)
		{
			if (base == m_TransportUnit.GetSourceBase())
			{
				selectedIndex = index;
			}

			m_SourceBaseComboBox.AddItem(base.GetCallsignDisplayName(), false, base);
		}

		m_SourceBaseComboBox.SetCurrentItem(selectedIndex + 1);
		m_SourceBaseComboBox.m_OnChanged.Insert(OnSourceBaseChanged);
		m_SourceBaseComboBox.m_OnOpened.Insert(OnComboBoxListOpened);
		m_SourceBaseComboBox.m_OnClosed.Insert(OnComboBoxListClosed);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetName(SCR_AIGroup transportGroup)
	{
		if (!m_wTransportUnitName || !transportGroup)
			return;

		string company, platoon, squad, character, format;
		transportGroup.GetCallsigns(company, platoon, squad, character, format);
		m_wTransportUnitName.SetTextFormat(format, company, platoon, squad, character);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSourceBaseChanged(SCR_ComboBoxComponent comboBoxComponent, int index)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_PlayerControllerGroupComponent playerControllerGroup = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (!playerControllerGroup)
			return;

		if (m_TransportUnit)
		{
			SCR_CampaignMilitaryBaseComponent militaryBase = SCR_CampaignMilitaryBaseComponent.Cast(m_SourceBaseComboBox.GetCurrentItemData());
			playerControllerGroup.SetTransportUnitSourceBase(m_TransportUnit, militaryBase);
		}

		if (m_mComboBoxWidgets.IsIndexValid(index))
		{
			Widget widget = m_mComboBoxWidgets[index];
			if (widget)
			{
				WidgetFlags flags = widget.GetFlags();
				widget.SetFlags(SCR_Enum.SetFlag(flags, WidgetFlags.NOFOCUS));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnComboBoxListOpened()
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;

		// Handle Context so the player does not zoom in/out the map while scrolling through list
		SCR_MapCursorModule cursorModule = SCR_MapCursorModule.Cast(mapEntity.GetMapModule(SCR_MapCursorModule));
		if (cursorModule)
			cursorModule.HandleContextualMenu(false);

		m_mComboBoxWidgets.Clear();
		m_SourceBaseComboBox.GetElementWidgets(m_mComboBoxWidgets);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnComboBoxListClosed()
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;

		// List is closed, return context so the player can navigate through the map
		SCR_MapCursorModule cursorModule = SCR_MapCursorModule.Cast(mapEntity.GetMapModule(SCR_MapCursorModule));
		if (cursorModule)
			cursorModule.HandleContextualMenu(true);

		FocusButton();
	}

	//------------------------------------------------------------------------------------------------
	void OnModularButtonClicked(SCR_ModularButtonComponent button)
	{
		m_SourceBaseComboBox.OpenList();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;

		super.HandlerAttached(w);

		m_wTransportUnitName = TextWidget.Cast(w.FindAnyWidget(m_sTransportUnitNameWidgetName));
		m_wVehicleActivityIcon = ImageWidget.Cast(w.FindAnyWidget(m_sVehicleActivityIconWidgetName));
		m_wDestinationName = TextWidget.Cast(w.FindAnyWidget(m_sDestinationNameWidgetName));
		m_wVehicleConditionIcon = ImageWidget.Cast(w.FindAnyWidget(m_sVehicleConditionIconWidgetName));

		m_SourceBaseComboBox = SCR_ComboBoxComponent.GetComboBoxComponent(m_sSourceBaseComboBoxWidgetName, w);

		Widget button = w.FindAnyWidget(m_sModularButtonWidgetName);
		if (!button)
			return;

		m_wModularButton = SCR_ModularButtonComponent.Cast(button.FindHandler(SCR_ModularButtonComponent));
		if (!m_wModularButton)
			return;

		m_wModularButton.m_OnClicked.Insert(OnModularButtonClicked);
	}
}
