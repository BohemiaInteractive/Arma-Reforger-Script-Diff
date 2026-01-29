class SCR_HUDGroupUIComponent : ScriptedWidgetComponent
{
	[Attribute("1")]
	protected bool m_bEnableDebugFont;
	
	[Attribute("20")]
	//! For debugging within the WorkBench Layout Editor only.
	private int m_iDebugFontSize; 

	protected float m_fWidth;
	protected float m_fHeight;

	protected Widget m_wRoot;

	protected ref SCR_HUDSizeCalculator m_HUDSizeCalculator = new SCR_HUDSizeCalculator();
	protected ref array<SCR_HUDSlotUIComponent> m_aSlots = {};

	//------------------------------------------------------------------------------------------------
	/*!
	\return Widget this GroupUIComponent is attached to.
	*/
	Widget GetRootWidget()
	{
		return m_wRoot;
	}

	//------------------------------------------------------------------------------------------------
	int GetSlotComponents(notnull out array<SCR_HUDSlotUIComponent> slotComponents)
	{
		slotComponents.Clear();

		foreach (SCR_HUDSlotUIComponent slot : m_aSlots)
		{
			slotComponents.Insert(slot);
		}

		return slotComponents.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_HUDSlotUIComponent FindSlotComponent(string slotName)
	{
		foreach (SCR_HUDSlotUIComponent slot : m_aSlots)
		{
			Widget slotWidget = slot.GetRootWidget();
			if (!slotWidget)
				continue;
			
			if (slotWidget.GetName() == slotName)
				return slot;
		}
		
		return null;
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(Widget w)
	{
		if (!SCR_Global.IsEditMode())
			return false;
		
		m_wRoot.GetScreenSize(m_fWidth, m_fHeight);
		m_HUDSizeCalculator.ResizeGroup(m_aSlots, m_fHeight, m_fWidth);

		return false;
	}
#endif

	//------------------------------------------------------------------------------------------------
	/*!
	Resizes all the slots within this Group, distributing the size of the Group Widget to all slots.
	*/
	void ResizeGroup()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();

		m_wRoot.GetScreenSize(m_fWidth, m_fHeight);
		m_fWidth = workspace.DPIUnscale(m_fWidth);
		m_fHeight = workspace.DPIUnscale(m_fHeight);

		m_HUDSizeCalculator.ResizeGroup(m_aSlots, m_fHeight, m_fWidth);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;

		Widget iteratedWidget = w.GetChildren();
		while (iteratedWidget)
		{
			SCR_HUDSlotUIComponent slot = SCR_HUDSlotUIComponent.Cast(iteratedWidget.FindHandler(SCR_HUDSlotUIComponent));
			if (!slot)
			{
				iteratedWidget = iteratedWidget.GetSibling();
				continue;
			}

			slot.Initialize();
			m_aSlots.Insert(slot);
			iteratedWidget = iteratedWidget.GetSibling();
		}

#ifdef WORKBENCH
		if (!SCR_Global.IsEditMode())
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		foreach (SCR_HUDSlotUIComponent slot : m_aSlots)
		{
			FrameWidget slotFrame = FrameWidget.Cast(slot.GetRootWidget());
			if (!slotFrame)
			{
				Debug.Error2("Wrong Slot Widget Type", "SCR_HUDSlotUIComponent Requires to be attached to a FrameWidget!");
				return;
			}

			slot.m_wDebugOverlay = OverlayWidget.Cast(workspace.CreateWidget(WidgetType.OverlayWidgetTypeID, WidgetFlags.VISIBLE, Color.FromInt(Color.BLACK), 0, slot.GetRootWidget()));
			if (!slot.m_wDebugOverlay || !m_bEnableDebugFont)
				return;			

			slot.m_wDebugImg = ImageWidget.Cast(workspace.CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE, new Color(Math.RandomFloat(0, 0.1), Math.RandomFloat(0, 0.1), Math.RandomFloat(0, 0.1), 1), 0, slot.m_wDebugOverlay));
			LayoutSlot.SetHorizontalAlign(slot.m_wDebugImg, LayoutHorizontalAlign.Stretch);
			LayoutSlot.SetVerticalAlign(slot.m_wDebugImg, LayoutVerticalAlign.Stretch);

			slot.m_wDebugVertical = workspace.CreateWidget(WidgetType.VerticalLayoutWidgetTypeID, WidgetFlags.VISIBLE, Color.FromInt(Color.BLACK), 0, slot.m_wDebugOverlay);
			OverlaySlot.SetHorizontalAlign(slot.m_wDebugVertical, LayoutHorizontalAlign.Stretch);
			OverlaySlot.SetVerticalAlign(slot.m_wDebugVertical, LayoutVerticalAlign.Stretch);

			slot.m_wDebugNameText = TextWidget.Cast(workspace.CreateWidget(WidgetType.TextWidgetTypeID, WidgetFlags.VISIBLE, Color.FromInt(Color.WHITE), 0, slot.m_wDebugVertical));
			OverlaySlot.SetHorizontalAlign(slot.m_wDebugNameText, LayoutHorizontalAlign.Center);
			slot.m_wDebugNameText.SetText(slot.GetRootWidget().GetName());
			slot.m_wDebugNameText.SetExactFontSize(m_iDebugFontSize);

			slot.m_wDebugPriorityText = TextWidget.Cast(workspace.CreateWidget(WidgetType.TextWidgetTypeID, WidgetFlags.VISIBLE, Color.FromInt(Color.WHITE), 0, slot.m_wDebugVertical));
			OverlaySlot.SetHorizontalAlign(slot.m_wDebugPriorityText, LayoutHorizontalAlign.Center);
			slot.SetPriority(slot.GetPriority());
			slot.m_wDebugPriorityText.SetExactFontSize(m_iDebugFontSize);

			slot.m_wDebugHeightText = RichTextWidget.Cast(workspace.CreateWidget(WidgetType.RichTextWidgetTypeID, WidgetFlags.VISIBLE, Color.FromInt(Color.WHITE), 0, slot.m_wDebugVertical));
			OverlaySlot.SetHorizontalAlign(slot.m_wDebugHeightText, LayoutHorizontalAlign.Center);
			slot.m_wDebugHeightText.SetExactFontSize(m_iDebugFontSize);

			slot.m_wDebugWidthText = RichTextWidget.Cast(workspace.CreateWidget(WidgetType.RichTextWidgetTypeID, WidgetFlags.VISIBLE, Color.FromInt(Color.WHITE), 0, slot.m_wDebugVertical));
			OverlaySlot.SetHorizontalAlign(slot.m_wDebugWidthText, LayoutHorizontalAlign.Center);
			slot.m_wDebugWidthText.SetExactFontSize(m_iDebugFontSize);
		}

#endif
	}
};
