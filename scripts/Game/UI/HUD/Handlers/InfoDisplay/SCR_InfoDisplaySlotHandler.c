[BaseContainerProps()]
class SCR_InfoDisplaySlotHandler : SCR_InfoDisplayHandler
{
	[Attribute()]
	protected string m_sSlotName;

	protected float m_fWidth, m_fHeight;
	protected Widget m_wSlot;
	protected Widget m_wContent;
	protected SCR_HUDGroupUIComponent m_GroupComponent;
	protected SCR_HUDSlotUIComponent m_SlotComponent;
	protected WorkspaceWidget m_wWorkspace;
	protected bool m_bIsVisible;

	[Attribute("0.1", desc: "When a difference in size equivalent to this value is detected, the slot will be resized.")]
	protected float m_fResizeSensitivity;

	//------------------------------------------------------------------------------------------------
	SCR_HUDSlotUIComponent GetSlotUIComponent()
	{
		if (!m_wContent)
			return null;
		
		Widget parentWidget = m_wContent.GetParent();
		if (parentWidget)
			m_SlotComponent = SCR_HUDSlotUIComponent.Cast(parentWidget.FindHandler(SCR_HUDSlotUIComponent));

		return m_SlotComponent;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetSlotWidget()
	{
		if (!m_SlotComponent)
			return null;

		return m_SlotComponent.GetRootWidget();
	}

	//------------------------------------------------------------------------------------------------
	SCR_HUDGroupUIComponent GetGroupUIComponent()
	{
		if (!m_wContent)
			return null;

		Widget slotWidget = m_wContent.GetParent();
		if (!slotWidget)
			return null;

		Widget groupWidget = slotWidget.GetParent();
		if (!groupWidget)
			return null;

		m_GroupComponent = SCR_HUDGroupUIComponent.Cast(groupWidget.FindHandler(SCR_HUDGroupUIComponent));
		return m_GroupComponent;
	}

	string GetSlotName()
	{
		return m_sSlotName;
	}

	void SetSlotComponent(notnull SCR_HUDSlotUIComponent slot)
	{
		m_SlotComponent = slot;
	}

	void SetGroupComponent(notnull SCR_HUDGroupUIComponent group)
	{
		m_GroupComponent = group;
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnStart(notnull SCR_InfoDisplay display)
	{
#ifdef WORKBENCH
		if (!m_bCanUpdate)
			Print("SCR_InfoDisplaySlotHandler requires CanUpdate to be set to true!", LogLevel.WARNING);
#endif
		m_wWorkspace = GetGame().GetWorkspace();

		SCR_HUDManagerComponent hudManager = null;
		SCR_HUDLayout owningLayout = null;

		if (!GrabHudComponents(m_GroupComponent, m_SlotComponent, hudManager, owningLayout))
		{
			Print("Failed To Create A Slot: " + m_sSlotName + ". One of the properties of the Info Dispaly is not defined properly, witch is preventing the initialization.", LogLevel.ERROR);
			return;
		}

		SCR_HUDManagerLayoutHandler layoutHandler = SCR_HUDManagerLayoutHandler.Cast(hudManager.FindHandler(SCR_HUDManagerLayoutHandler));
		if (!layoutHandler)
		{
			Print("Failed to obtain SCR_HUDManagerLayoutHandler for InfoDisplay: " + m_OwnerDisplay.Type().ToString(), LogLevel.ERROR);
			return;
		}

		m_wSlot = m_SlotComponent.GetRootWidget();
		if (!m_wSlot)
			return;

		if (m_wSlot.GetChildren())
		{
			Print("Slot Already Taken: " + m_sSlotName + ". The Slot an Info Display is trying to occupy is already occupied!", LogLevel.ERROR);
			return;
		}

		Widget displayWidget = m_wWorkspace.CreateWidgets(display.m_LayoutPath, m_wSlot);
		if (!displayWidget)
			return;
		m_wContent = displayWidget;

		SCR_HUDElement hudElement = new SCR_HUDElement();
		hudElement.SetWidget(displayWidget);
		hudElement.SetParentWidgetName(m_sSlotName);
		hudElement.SetParentLayout(owningLayout);
		owningLayout.AddHudElement(hudElement, false);

		display.SetRootWidget(displayWidget);
		display.SetContentWidget(displayWidget);
		display.RegisterToHudManager();

		m_bIsVisible = displayWidget.IsVisible();

		m_SlotComponent.Initialize();
		m_SlotComponent.GetOnResize().Insert(OnSlotResize);

		m_GroupComponent.ResizeGroup();
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnStop(notnull SCR_InfoDisplay display)
	{
		if (m_SlotComponent)
			m_SlotComponent.GetOnResize().Remove(OnSlotResize);
	}

	//------------------------------------------------------------------------------------------------
	protected bool GrabHudComponents(out SCR_HUDGroupUIComponent groupComponent, out SCR_HUDSlotUIComponent slotComponent, out SCR_HUDManagerComponent hudManager, out SCR_HUDLayout owningLayout)
	{
		hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (!hudManager)
			return false;

		SCR_HUDManagerLayoutHandler layoutHandler = SCR_HUDManagerLayoutHandler.Cast(hudManager.FindHandler(SCR_HUDManagerLayoutHandler));
		if (!layoutHandler)
			return false;

		slotComponent = layoutHandler.FindSlotComponentFromAnyLayout(m_sSlotName, owningLayout);
		if (!slotComponent)
			return false;

		Widget slotWidget = slotComponent.GetRootWidget();
		if (!slotWidget)
			return false;

		Widget groupWidget = slotWidget.GetParent();
		if (!groupWidget)
			return false;

		groupComponent = SCR_HUDGroupUIComponent.Cast(groupWidget.FindHandler(SCR_HUDGroupUIComponent));
		if (!groupComponent)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	private void OnSlotResize(notnull SCR_HUDSlotUIComponent slot)
	{
		m_fWidth = slot.GetWidth();
		m_fHeight = slot.GetHeight();
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float timeSlice)
	{
		if (!m_wContent)
			return;

		float width, height;
		m_wContent.GetScreenSize(width, height);

		width = m_wWorkspace.DPIUnscale(width);
		height = m_wWorkspace.DPIUnscale(height);

		bool isVisible = m_wContent.IsVisible();

		if (!float.AlmostEqual(width, m_fWidth, m_fResizeSensitivity) || !float.AlmostEqual(height, m_fHeight, m_fResizeSensitivity) || m_bIsVisible != isVisible)
		{
			m_GroupComponent.ResizeGroup();
			m_fHeight = height;
			m_fWidth = width;
			m_bIsVisible = isVisible;
		}

	}
}