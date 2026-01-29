[BaseContainerProps()]
class SCR_InfoDisplayLayerHandler : SCR_InfoDisplayHandler
{
	[Attribute()]
	protected string m_sLayerName;

	//------------------------------------------------------------------------------------------------
	protected override void OnStart(notnull SCR_InfoDisplay display)
	{
		if (display.GetHandler(SCR_InfoDisplaySlotHandler) != null)
		{
			Print("Info Display cant have SCR_InfoDisplayLayerHandler & SCR_InfoDisplaySlotHandler co-exist as handlers!", LogLevel.ERROR);
			return;
		}

		WorkspaceWidget workspace = GetGame().GetWorkspace();

		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.GetHUDManager();
		if (!hudManager)
		{
			Print("No HUD Manager for Layer Slot: " + m_sLayerName, LogLevel.ERROR);
			return;
		}

		SCR_HUDManagerLayoutHandler layoutHandler = SCR_HUDManagerLayoutHandler.Cast(hudManager.FindHandler(SCR_HUDManagerLayoutHandler));
		if (!layoutHandler)
		{
			Print("Cant find SCR_HUDManagerLayoutHandler for Layer Slot: " + m_sLayerName, LogLevel.ERROR);
			return;
		}

		SCR_HUDLayout owningLayout;
		Widget layerSlotWidget = layoutHandler.FindWidgetByNameFromAnyLayout(m_sLayerName, owningLayout);
		if (!layerSlotWidget)
		{
			Print("No Slot Widget for Layer Slot: " + m_sLayerName, LogLevel.ERROR);
			return;
		}

		Widget rootWidget = workspace.CreateWidgets(display.m_LayoutPath, layerSlotWidget);
		if (!rootWidget)
		{
			Print("Can't initialize Info Display on provided Slot Widget: " + m_sLayerName, LogLevel.ERROR);
			return;
		}

		display.SetRootWidget(rootWidget);
		display.SetContentWidget(rootWidget);

		SCR_HUDElement hudElement = new SCR_HUDElement();
		hudElement.SetWidget(rootWidget);
		hudElement.SetParentWidgetName(m_sLayerName);
		hudElement.SetParentLayout(owningLayout);
		owningLayout.AddHudElement(hudElement, false);
		display.RegisterToHudManager();
	}
}
