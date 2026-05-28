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

	protected SCR_TaskSolverManagerComponent m_TaskSolverManager;
	protected SCR_TransportUnitComponent m_TransportUnit;

	protected TextWidget m_wTransportUnitName;
	protected ImageWidget m_wVehicleActivityIcon;
	protected ImageWidget m_wVehicleConditionIcon;
	protected TextWidget m_wDestinationName;
	protected SCR_ModularButtonComponent m_wModularButton;

	protected SCR_ComboBoxComponent m_SourceBaseComboBox;
	protected ref array<Widget> m_aComboBoxWidgets = {};

	//------------------------------------------------------------------------------------------------
	void Init(SCR_TransportUnitComponent transportUnit)
	{
		m_TransportUnit = transportUnit;
		if (!m_TransportUnit)
			return;

		m_TaskSolverManager = SCR_TaskSolverManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_TaskSolverManagerComponent));
		if (!m_TaskSolverManager)
			return;

		SetName(m_TransportUnit.GetAIGroup());
		UpdateVehicleConditionIcon(m_TransportUnit.GetVehicle());
		SetVehicleActivityIcon(m_TransportUnit.GetResupplyTaskSolverState());
		SetVehicleDestinationName(m_TransportUnit.GetResupplyTaskSolverState());

		SetupComboBox();

		m_TransportUnit.GetOnVehicleResourcesValueChanged().Insert(OnVehicleResourcesValueChanged);
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
			m_TransportUnit.GetOnVehicleResourcesValueChanged().Remove(OnVehicleResourcesValueChanged);
			m_TransportUnit.GetOnVehicleChanged().Remove(OnVehicleChanged);
			m_TransportUnit.GetOnResupplyTaskSolverStateChanged().Remove(OnResupplyTakSolverStateChanged);
		}
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
		UpdateVehicleConditionIcon(vehicle);
	}

	//------------------------------------------------------------------------------------------------
	protected bool UpdateVehicleConditionIconBasedOnDamage()
	{
		if (m_TransportUnit.IsVehicleUsable())
			return false;

		SCR_ResupplyTaskSolverEntry resuplyTaskSolveEntry = SCR_ResupplyTaskSolverEntry.Cast(m_TaskSolverManager.GetTaskSolverEntry(SCR_ResupplyTaskSolverEntry));
		if (!resuplyTaskSolveEntry)
			return false;

		SCR_ConditionCheckUIEntry conditionCheckUIEntry = resuplyTaskSolveEntry.GetTaskSolverEntry(m_TransportUnit.GetVehicleFailedConditionType());
		if (!conditionCheckUIEntry)
			return false;

		SCR_UIInfo failedConditionUIInfo = conditionCheckUIEntry.GetFailedConditionUIInfo();
		if (!failedConditionUIInfo)
			return false;

		if (failedConditionUIInfo.SetIconTo(m_wVehicleConditionIcon))
			m_wVehicleConditionIcon.SetColor(m_FaultStateColor);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVehicleResourcesValueChanged(float aggregatedResourcesValue)
	{
		if (aggregatedResourcesValue < 0)
			return;

		if (m_TransportUnit.GetSolver().GetResupplyTaskSolverState() != SCR_EResupplyTaskSolverState.LOADING_SUPPLIES && aggregatedResourcesValue > 0)
		{
			SetTexture(m_wVehicleConditionIcon, m_sTransportUnitImageStateResourceName, m_sFullyLoadedVehicleConditionImageName, m_NormalStateColor);
		}
		else
		{
			SetTexture(m_wVehicleConditionIcon, m_sTransportUnitImageStateResourceName, m_sNotLoadedVehicleConditionImageName, m_NormalStateColor);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateVehicleConditionIcon(Vehicle vehicle)
	{
		if (m_TransportUnit.HasVehicle())
		{
			bool vehicleConditionIconUpdated = UpdateVehicleConditionIconBasedOnDamage();
			if (vehicleConditionIconUpdated)
				return;

			OnVehicleResourcesValueChanged(m_TransportUnit.GetVehicleAggregatedResourceValue())
		}
		else
		{
			SetTexture(m_wVehicleConditionIcon, m_sTransportUnitImageStateResourceName, m_sMissingVehicleConditionImageName, m_FaultStateColor);
		}
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

		if (m_aComboBoxWidgets.IsIndexValid(index))
		{
			Widget widget = m_aComboBoxWidgets[index];
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

		m_aComboBoxWidgets.Clear();
		m_SourceBaseComboBox.GetElementWidgets(m_aComboBoxWidgets);
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

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;

		super.HandlerAttached(w);

		if (m_wModularButton)
			m_wModularButton.m_OnClicked.Remove(OnModularButtonClicked);
	}
}
