class SCR_CampaignLogisticMapUIBase : SCR_CampaignMapUIBase
{
	[Attribute("ActualPriority")]
	protected string m_sPriorityTextWidgetName;

	[Attribute("SupplyLimitSlider")]
	protected string m_sSupplyLimitSliderName;

	[Attribute("ExtendedInfoWrapper")]
	protected string m_sExtendedInfoWrapperName;

	[Attribute("PriorityButtonsWrapper")]
	protected string m_sPriorityButtonsWrapperName;

	[Attribute("CloseButton")]
	protected string m_sCloseButtonName;

	[Attribute("SwitchPriorityButton")]
	protected string m_sSwitchPriorityButtonName;

	[Attribute("ButtonPriority1")]
	protected string m_sPriorityButton1Name;

	[Attribute("ButtonPriority2")]
	protected string m_sPriorityButton2Name;

	[Attribute("ButtonPriority3")]
	protected string m_sPriorityButton3Name;

	[Attribute("ReservedSupplyAmountSlider")]
	protected string m_sReservedSupplyAmountSliderName;

	[Attribute("PriorityWrapper")]
	protected string m_sPriorityWrapperName;

	[Attribute("NextShipmentWrapper")]
	protected string m_sNextShipmentWrapperName;

	[Attribute("NextShipmentText")]
	protected string m_sNextShipmentTextName;

	[Attribute("60", params: "0 inf 1", desc: "How many seconds between updates of next shipment text")]
	protected float m_fNextShipmentTextUpdatePeriod;

	const string LOGISTIC_CLOSE_ACTION_NAME = "LogisticClose";

	protected SCR_SliderComponent m_SupplyLimitSlider;
	protected SCR_SliderComponent m_ReservedSupplyAmountSlider;

	protected SCR_MapEntity m_MapEntity;

	protected Widget m_wExtendedInfoWrapper;
	protected Widget m_wPriorityButtonsWrapper;

	protected SCR_InputButtonComponent m_CloseButton;
	protected SCR_InputButtonComponent m_SwitchPriorityButton;

	protected ref array<SCR_ModularButtonComponent> m_aPriorityButtons = {};

	protected TextWidget m_wPriorityTextWidget;
	protected Widget m_wPriorityButtonWidget1, m_wPriorityButtonWidget2, m_wPriorityButtonWidget3;
	protected Widget m_wPriorityWrapper;
	protected Widget m_wNextShipmentWrapper;
	protected TextWidget m_wNextShipmentText;
	protected Widget m_wSupplyLimitSliderWidget;

	protected static ref ScriptInvokerBool s_OnBaseSelected;
	protected static const LocalizedString NEXT_SHIPMENT_ETA = "#AR-FactionCommander_LogisticUIBaseNextShipmentETA";

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerBool GetOnBaseSelected()
	{
		if (!s_OnBaseSelected)
			s_OnBaseSelected = new ScriptInvokerBool();

		return s_OnBaseSelected;
	}

	//------------------------------------------------------------------------------------------------
	bool IsSelected()
	{
		return m_bIsSelected;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOpened()
	{
		return m_wExtendedInfoWrapper && m_wExtendedInfoWrapper.IsVisible();
	}

	//------------------------------------------------------------------------------------------------
	override protected bool CanShowOpenButton()
	{
		if (!super.CanShowOpenButton() || m_wExtendedInfoWrapper.IsVisible())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanOpenExtendedInfoWrapper()
	{
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		if (faction != m_Base.GetFaction() || m_Base.GetType() == SCR_ECampaignBaseType.SOURCE_BASE)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenExtendedInfoWrapper()
	{
		if (!CanOpenExtendedInfoWrapper())
			return;

		if (!m_bIsSelected && !m_bIsAnyElementHovered)
		{
			m_wExtendedInfoWrapper.SetVisible(true);

			SetPriorityButtonWidgetsVisibility(true);

			if (m_wOpenButton)
				m_wOpenButton.SetVisible(false);

			m_bIsAnyElementClicked = true;
			SelectIcon();
			m_bIsAnyElementClicked = false;
			m_wHighlightImg.SetVisible(false);

			UpdateSliderDisplayValue(m_SupplyLimitSlider);
			UpdateSliderDisplayValue(m_ReservedSupplyAmountSlider);

			GetGame().GetInputManager().AddActionListener(LOGISTIC_CLOSE_ACTION_NAME, EActionTrigger.DOWN, OnLogisticClose);

			if (m_wSupplyLimitSliderWidget)
				GetGame().GetWorkspace().SetFocusedWidget(m_wSupplyLimitSliderWidget);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpenButtonActivated(SCR_InputButtonComponent button, string action)
	{
		OpenExtendedInfoWrapper();
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);

		OpenExtendedInfoWrapper();

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		if (!m_Base || faction != m_Base.GetFaction())
		{
			m_wPriorityWrapper.SetVisible(false);
			m_wNextShipmentWrapper.SetVisible(false);
			return super.OnMouseEnter(w, x, y);
		}

		if (m_Base.GetType() == SCR_ECampaignBaseType.SOURCE_BASE)
		{
			m_wPriorityWrapper.SetVisible(false);
			m_wNextShipmentWrapper.SetVisible(true);
			SetNextShipmentText();
			GetGame().GetCallqueue().CallLater(SetNextShipmentText, m_fNextShipmentTextUpdatePeriod * 1000, true);
		}
		else
		{
			m_wNextShipmentWrapper.SetVisible(false);
			UpdatePriorityWidgetsValues(m_Base.GetSupplyRequestExecutionPriority());
			m_Base.GetOnSupplyRequestExecutionPriorityChanged().Insert(UpdatePriorityWidgetsValues);
		}

		return super.OnMouseEnter(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override void InitBase(SCR_CampaignMilitaryBaseComponent base)
	{
		super.InitBase(base);

		if (!base)
			return;

		if (!m_SupplyLimitSlider)
			return;

		m_SupplyLimitSlider.SetValue(base.GetSupplyLimit());

		if (!base.IsResupplyTaskCreationEnabled())
		{
			m_SupplyLimitSlider.SetVisible(false);
		}

		if (!m_ReservedSupplyAmountSlider)
			return;

		m_ReservedSupplyAmountSlider.SetValue(base.GetReservedSupplyAmount());
	}

	//------------------------------------------------------------------------------------------------
	override void Select(bool select = true)
	{
		super.Select(select);

		if (m_Base && m_Base.GetType() == SCR_ECampaignBaseType.SOURCE_BASE)
			m_wPriorityWrapper.SetVisible(false);

		if (!select)
		{
			SetPriorityButtonWidgetsVisibility(false);

			if (m_wOpenButton)
				m_wOpenButton.SetVisible(CanOpenExtendedInfoWrapper());

			m_wExtendedInfoWrapper.SetVisible(false);
			m_wPriorityTextWidget.SetVisible(true);

			GetGame().GetInputManager().RemoveActionListener(LOGISTIC_CLOSE_ACTION_NAME, EActionTrigger.DOWN, OnLogisticClose);
		}
		else
		{
			m_wPriorityTextWidget.SetVisible(false);
		}

		if (s_OnBaseSelected)
			s_OnBaseSelected.Invoke(select);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_bIsSelected)
			return false;

		if (m_Base.GetType() == SCR_ECampaignBaseType.SOURCE_BASE)
			GetGame().GetCallqueue().Remove(SetNextShipmentText);

		return super.OnMouseLeave(w, enterW, x, y);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLogisticClose()
	{
		// Add delay so Select is executed after OnClick and widget is properly closed
		GetGame().GetCallqueue().CallLater(Select, 0, false, false);

		if (m_Base.GetType() == SCR_ECampaignBaseType.SOURCE_BASE)
			GetGame().GetCallqueue().Remove(SetNextShipmentText);
	}

	// ------------------------------------------------------------------------------
	protected void OnSupplyLimitChanged(SCR_SliderComponent sliderComponent, float value)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_CampaignFactionCommanderPlayerComponent campaignFactionCommander = SCR_CampaignFactionCommanderPlayerComponent.Cast(playerController.FindComponent(SCR_CampaignFactionCommanderPlayerComponent));
		if (!campaignFactionCommander)
			return;

		UpdateSliderDisplayValue(sliderComponent);

		if (m_Base)
			campaignFactionCommander.SetSuppliesLimit(m_Base, value);
	}

	// ------------------------------------------------------------------------------
	protected void OnReservedSupplyAmountChanged(SCR_SliderComponent sliderComponent, float value)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_CampaignFactionCommanderPlayerComponent campaignFactionCommander = SCR_CampaignFactionCommanderPlayerComponent.Cast(playerController.FindComponent(SCR_CampaignFactionCommanderPlayerComponent));
		if (!campaignFactionCommander)
			return;

		UpdateSliderDisplayValue(sliderComponent);

		campaignFactionCommander.SetReservedSupplyAmount(m_Base, value);
	}

	// ------------------------------------------------------------------------------
	//! Multiplies the slider value with the max amount of supplies of base
	protected void UpdateSliderDisplayValue(SCR_SliderComponent slider)
	{
		if (m_Base)
			slider.SetShownValueMultiplier(m_Base.GetSuppliesMax());

		slider.SetValue(slider.GetValue());
	}

	// ------------------------------------------------------------------------------
	protected void OnPriorityButtonToggled(notnull SCR_ModularButtonComponent newPriorityButton, bool isOn)
	{
		if (!isOn)
			newPriorityButton.SetToggled(true, false);

		SetPriorityButtonsState(newPriorityButton);
	}

	// ------------------------------------------------------------------------------
	protected void OnSwitchPriorityButton()
	{
		int currentPriority = m_Base.GetSupplyRequestExecutionPriority();
		int minPriority, maxPriority;
		SCR_Enum.GetRange(SCR_ESupplyRequestExecutionPriority, minPriority, maxPriority);

		int newPriority = Math.Mod(currentPriority + 1, maxPriority + 1);
		SCR_ModularButtonComponent newPriorityButton = m_aPriorityButtons.Get(newPriority);
		if (!newPriorityButton)
			return;

		SetPriorityButtonsState(newPriorityButton);
	}

	// ------------------------------------------------------------------------------
	protected void SetPriorityButtonsState(notnull SCR_ModularButtonComponent newPriorityButton)
	{
		// Set the priority based on newPriorityButton
		// We toggle off all other priority buttons
		foreach (int index, SCR_ModularButtonComponent currentButton : m_aPriorityButtons)
		{
			if (currentButton == newPriorityButton)
				SetSupplyRequestExecutionPriority(index);
			else
				currentButton.SetToggled(false, false);
		}
	}

	// ------------------------------------------------------------------------------
	protected void SetSupplyRequestExecutionPriority(int priority)
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return;

		SCR_CampaignFactionCommanderPlayerComponent campaignFactionCommander = SCR_CampaignFactionCommanderPlayerComponent.Cast(playerController.FindComponent(SCR_CampaignFactionCommanderPlayerComponent));
		if (!campaignFactionCommander)
			return;

		if (m_Base)
			campaignFactionCommander.SetSupplyRequestExecutionPriority(m_Base, priority);
	}

	// ------------------------------------------------------------------------------
	protected void SetPriorityButtonWidgetsVisibility(bool isVisible)
	{
		m_wPriorityButtonsWrapper.SetVisible(isVisible);
	}

	// ------------------------------------------------------------------------------
	protected void UpdatePriorityWidgetsValues(int priority)
	{
		if (m_wPriorityTextWidget)
			m_wPriorityTextWidget.SetText((priority + 1).ToString());

		foreach (int index, SCR_ModularButtonComponent currentButton : m_aPriorityButtons)
		{
			if (index == priority)
				currentButton.SetToggled(true, false);
			else
				currentButton.SetToggled(false, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetNextShipmentText()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		m_wNextShipmentText.SetText(WidgetManager.Translate(NEXT_SHIPMENT_ETA, Math.Ceil((m_Base.GetSuppliesArrivalTime() - world.GetWorldTime()) * 0.001 / 60)));
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		if (!GetGame().InPlayMode())
			return;

		m_wExtendedInfoWrapper = w.FindAnyWidget(m_sExtendedInfoWrapperName);
		if (!m_wExtendedInfoWrapper)
			return;

		m_wExtendedInfoWrapper.SetVisible(false);

		m_wPriorityButtonsWrapper = w.FindAnyWidget(m_sPriorityButtonsWrapperName);
		if (!m_wPriorityButtonsWrapper)
			return;

		m_wSupplyLimitSliderWidget = w.FindAnyWidget(m_sSupplyLimitSliderName);
		if (!m_wSupplyLimitSliderWidget)
			return;

		m_SupplyLimitSlider = SCR_SliderComponent.Cast(m_wSupplyLimitSliderWidget.FindHandler(SCR_SliderComponent));
		if (!m_SupplyLimitSlider)
			return;
		
		m_SupplyLimitSlider.SetMin(SCR_CampaignMilitaryBaseComponent.SUPPLY_LIMIT_MIN);
		m_SupplyLimitSlider.SetMax(SCR_CampaignMilitaryBaseComponent.SUPPLY_LIMIT_MAX);

		Widget reservedSupplyAmountSliderWidget = w.FindAnyWidget(m_sReservedSupplyAmountSliderName);
		if (!reservedSupplyAmountSliderWidget)
			return;

		m_ReservedSupplyAmountSlider = SCR_SliderComponent.Cast(reservedSupplyAmountSliderWidget.FindHandler(SCR_SliderComponent));
		if (!m_ReservedSupplyAmountSlider)
			return;

		m_SupplyLimitSlider.GetOnChangedFinal().Insert(OnSupplyLimitChanged);
		m_ReservedSupplyAmountSlider.GetOnChangedFinal().Insert(OnReservedSupplyAmountChanged);

		m_MapEntity = SCR_MapEntity.GetMapInstance();

		m_wPriorityWrapper = w.FindAnyWidget(m_sPriorityWrapperName);

		m_wPriorityButtonWidget1 = w.FindAnyWidget(m_sPriorityButton1Name);
		m_wPriorityButtonWidget2 = w.FindAnyWidget(m_sPriorityButton2Name);
		m_wPriorityButtonWidget3 = w.FindAnyWidget(m_sPriorityButton3Name);

		m_wPriorityTextWidget = TextWidget.Cast(w.FindAnyWidget(m_sPriorityTextWidgetName));

		m_aPriorityButtons.Insert(SCR_ModularButtonComponent.FindComponent(m_wPriorityButtonWidget1));
		m_aPriorityButtons.Insert(SCR_ModularButtonComponent.FindComponent(m_wPriorityButtonWidget2));
		m_aPriorityButtons.Insert(SCR_ModularButtonComponent.FindComponent(m_wPriorityButtonWidget3));

		foreach (SCR_ModularButtonComponent button : m_aPriorityButtons)
		{
			if (button)
				button.m_OnToggled.Insert(OnPriorityButtonToggled);
		}

		m_wNextShipmentWrapper = w.FindAnyWidget(m_sNextShipmentWrapperName);

		m_wNextShipmentText = TextWidget.Cast(w.FindAnyWidget(m_sNextShipmentTextName));

		SetPriorityButtonWidgetsVisibility(false);

		m_CloseButton = SCR_InputButtonComponent.GetInputButtonComponent(m_sCloseButtonName, w);
		if (!m_CloseButton)
			return;

		m_CloseButton.m_OnActivated.Insert(OnLogisticClose);

		m_SwitchPriorityButton = SCR_InputButtonComponent.GetInputButtonComponent(m_sSwitchPriorityButtonName, w);
		if (!m_SwitchPriorityButton)
			return;

		m_SwitchPriorityButton.m_OnActivated.Insert(OnSwitchPriorityButton);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		if (m_SupplyLimitSlider)
			m_SupplyLimitSlider.GetOnChangedFinal().Remove(OnSupplyLimitChanged);

		if (m_ReservedSupplyAmountSlider)
			m_ReservedSupplyAmountSlider.GetOnChangedFinal().Remove(OnReservedSupplyAmountChanged);

		if (m_CloseButton)
			m_CloseButton.m_OnActivated.Remove(OnLogisticClose);

		if (m_SwitchPriorityButton)
			m_SwitchPriorityButton.m_OnActivated.Remove(OnSwitchPriorityButton);

		if (m_aPriorityButtons)
		{
			foreach (SCR_ModularButtonComponent button : m_aPriorityButtons)
			{
				if (button)
					button.m_OnToggled.Remove(OnPriorityButtonToggled);
			}
		}

		if (m_Base)
			m_Base.GetOnSupplyRequestExecutionPriorityChanged().Remove(UpdatePriorityWidgetsValues);
	}
}
