void SCR_HUDSlotResizeCallback(SCR_InfoDisplay display);
typedef func SCR_SlotResizeCallback;
typedef ScriptInvokerBase<SCR_SlotResizeCallback> SCR_HUDSlotResizeInvoker;

class SCR_HUDSlotUIComponent : ScriptedWidgetComponent
{
// Define member variables that are only included in the Workbench for debugging purposes.
#ifdef WORKBENCH
	OverlayWidget m_wDebugOverlay;
	ImageWidget m_wDebugImg;
	Widget m_wDebugVertical
	TextWidget m_wDebugNameText;
	RichTextWidget m_wDebugHeightText;
	RichTextWidget m_wDebugWidthText;
	TextWidget m_wDebugPriorityText;
#endif

	//! A boolean value indicating whether the Slot has been initialized
	protected bool m_bInitialized;
	protected WorkspaceWidget m_wWorkspace;
	//! The root widget of the Slot
	protected Widget m_wRoot;
	//! The height of the Slot
	protected int m_iHeight;
	//! The width of the Slot
	protected int m_iWidth;

	protected SCR_HUDGroupUIComponent m_GroupComponent; // The Group this Slot is part of
	protected ref SCR_HUDSlotResizeInvoker m_OnResize = new SCR_HUDSlotResizeInvoker();

	[Attribute(desc: "Priority of the Slot")]
	protected int m_iPriority;

	[Attribute(desc: "Possible height steps for the Slot.")]
	ref array<int> m_aHeightSteps;

	[Attribute(desc: "Possible width steps for the component.")]
	ref array<int> m_aWidthSteps;

	[Attribute(desc: "Whether the Slot should be sized to its content.")]
	bool m_bSizeToContent;

	//------------------------------------------------------------------------------------------------
	/*!
	/return the height of the Slot.
	*/
	int GetHeight()
	{
		return m_iHeight;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	/return the Width of the Slot.
	*/
	int GetWidth()
	{
		return m_iWidth;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	/return the Priority of the Slot.
	*/
	int GetPriority()
	{
		return m_iPriority;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Sets the Priority of the Slot.
	*/
	void SetPriority(int newPriority)
	{
		m_iPriority = newPriority;

#ifdef WORKBENCH
		if (m_wDebugPriorityText)
			m_wDebugPriorityText.SetText("Priority: " + m_iPriority.ToString());
#endif

		if (m_GroupComponent)
			m_GroupComponent.ResizeGroup();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	/return the ScriptInvoker which gets invoked when the Slot is resized.
	*/
	SCR_HUDSlotResizeInvoker GetOnResize()
	{
		return m_OnResize;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Sets the Height of the Slot.
	*/
	void SetHeight(int height, bool notifyResizing = true)
	{
		if (m_iHeight == height)
			return;

		m_iHeight = height;

#ifdef WORKBENCH
		if (SCR_Global.IsEditMode())
		{
			DebugHeightInfo();
			return;
		}
#endif
		Widget contentWidget = GetContentWidget();

		// If the Slot is not set to size to content, set the Height of the content Widget to the new height
		if (!m_bSizeToContent && contentWidget)
		{
				FrameSlot.SetSizeY(contentWidget, m_iHeight);
		}
		// Otherwise, set the height of the Slot to the height of the content Widget
		else if (m_bSizeToContent)
		{
			float contentWidth = 0;
			float contentHeight = 0;

			if (contentWidget)
				contentWidget.GetScreenSize(contentWidth, contentHeight);

			m_iHeight = m_wWorkspace.DPIUnscale(contentHeight);
		}

		if (notifyResizing)
			m_OnResize.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	A method that sets the Width of the Slot.
	*/
	void SetWidth(int width, bool notifyResizing = true)
	{
		if (m_iWidth == width)
			return;

		m_iWidth = width;
#ifdef WORKBENCH
		if (SCR_Global.IsEditMode())
		{
			DebugWidthInfo();
			return;
		}
#endif
		Widget contentWidget = GetContentWidget();

		// If the Slot is not set to size to content, set the Width of the content Widget to the new width
		if (!m_bSizeToContent && contentWidget)
		{
			FrameSlot.SetSizeX(contentWidget, m_iWidth);
		}
		// Otherwise, set the width of the component to the width of the content Widget
		else if (m_bSizeToContent)
		{
			float contentWidth = 0;
			float contentHeight = 0;

			if (contentWidget)
				contentWidget.GetScreenSize(contentWidth, contentHeight);

			m_iWidth = m_wWorkspace.DPIUnscale(contentWidth);
		}

		if (notifyResizing)
			m_OnResize.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Gets the Content Widget of the Slot.
	*/
	Widget GetContentWidget()
	{
		return m_wRoot.GetChildren();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Gets the Root Widget of the Slot.
	*/
	Widget GetRootWidget()
	{
		return m_wRoot;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Intializes the Slot.
	*/
	void Initialize()
	{
		if (m_bInitialized)
			return;

		Widget child = m_wRoot.GetChildren();
		if (!child)
			Print(m_wRoot.GetName() + " Has no Content!", LogLevel.WARNING);

		if (!GetGroupComponent())
			Debug.Error2("No Group Component", "A Slot's parent must always have a Group Component! Slot: " + m_wRoot.GetName());

		m_wWorkspace = GetGame().GetWorkspace();
		m_bInitialized = true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Gets the group component of the Slot.
	*/
	protected SCR_HUDGroupUIComponent GetGroupComponent()
	{
		Widget parent = m_wRoot.GetParent();

		if (!m_GroupComponent && parent)
			m_GroupComponent = SCR_HUDGroupUIComponent.Cast(parent.FindHandler(SCR_HUDGroupUIComponent));
		return m_GroupComponent;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;

		m_aHeightSteps.Sort();
		m_aWidthSteps.Sort();
		GetGroupComponent();
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	/*!
	Displays debugging information for the height of the slot. Only available within Workbench.
	*/
	void DebugHeightInfo()
	{
		if (!m_wDebugHeightText || !SCR_Global.IsEditMode())
			return;
		Widget contentWidget = GetContentWidget();

		if (!contentWidget)
			contentWidget = m_wRoot.GetChildren();

		FrameSlot.SetSizeY(contentWidget, m_iHeight);

		m_wDebugHeightText.SetText("Height:");
		foreach (int step : m_aHeightSteps)
		{
			if (step == m_iHeight)
				m_wDebugHeightText.SetText(m_wDebugHeightText.GetText() + " <color name='green'>" + step + "</color>");
			else
				m_wDebugHeightText.SetText(m_wDebugHeightText.GetText() + " " + step);
		}

		if (m_aHeightSteps.IsEmpty())
			m_wDebugHeightText.SetText(string.Empty);

		if (m_bSizeToContent)
		{
			FrameSlot.SetSizeY(m_wDebugOverlay, m_iHeight);
			m_wDebugHeightText.SetText("Height: <color name='green'>" + m_iHeight + "</color>");
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Displays debugging information for the width of the slot. Only available within Workbench.
	*/
	void DebugWidthInfo()
	{
		if (!m_wDebugWidthText || !SCR_Global.IsEditMode())
			return;

		FrameSlot.SetSizeX(m_wDebugOverlay, m_iWidth);

		m_wDebugWidthText.SetText("Width:");
		foreach (int step : m_aWidthSteps)
		{
			if (step == m_iWidth)
				m_wDebugWidthText.SetText(m_wDebugWidthText.GetText() + " <color name='green'>" + step + "</color>");
			else
				m_wDebugWidthText.SetText(m_wDebugWidthText.GetText() + " " + step);
		}

		if (m_aWidthSteps.IsEmpty())
			m_wDebugWidthText.SetText(string.Empty);

		if (m_bSizeToContent)
		{
			if (!m_aWidthSteps.IsEmpty())
			{
				m_iWidth = m_aWidthSteps[m_aWidthSteps.Count() - 1];
				FrameSlot.SetSizeX(m_wDebugOverlay, m_iWidth);
			}

			m_wDebugWidthText.SetText("Width: <color name='green'>" + m_iWidth + "</color>");
		}
	}
#endif
}
