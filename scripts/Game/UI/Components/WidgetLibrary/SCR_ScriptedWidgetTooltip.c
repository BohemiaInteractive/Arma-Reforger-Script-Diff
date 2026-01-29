/*
Parent class of Scripted Widgets Tooltips.
These tooltips are set on widgets themselves, in the Behaviour section.
In order to keep things orderly with the huge amount of tooltips required, and for ease of access, they can be defined in .conf files.
Create a .conf file from SCR_ScriptedWidgetTooltipPresets to define a group of tooltips, add members, and give each preset a unique tag.
In the Behaviour section of the widget you want to have a tooltip, pick this class, reference the .conf file and use the appropriate tag.
Don't forget to set Ignore Cursor to false or the tooltip won't trigger!
*/

void ScriptInvokerTooltipMethod(SCR_ScriptedWidgetTooltip tooltip);
typedef func ScriptInvokerTooltipMethod;
typedef ScriptInvokerBase<ScriptInvokerTooltipMethod> ScriptInvokerTooltip;

//------------------------------------------------------------------------------------------------
class SCR_ScriptedWidgetTooltip : ScriptedWidgetTooltip
{
	[Attribute(desc:"Class must be SCR_ScriptedWidgetTooltipPresets", params:"conf class=SCR_ScriptedWidgetTooltipPresets")]
	protected ResourceName m_sPresetsConfig;

	[Attribute(desc:"tag to find the preset in the .conf")]
	protected string m_sPresetTag;

	protected ref SCR_ScriptedWidgetTooltipPreset m_Preset;
	protected WorkspaceWidget m_wWorkspace;
	protected Widget m_wTooltipProxy;
	protected Widget m_wHoverWidget;

	protected float m_fTargetPosition[2];

	// Const
	private const float DISTANCE_THRESHOLD = 0.001;
	private const int DELAYED_INIT_WAIT_FRAMES = 2;

	// Static
	protected static WidgetAnimationPosition m_PositionAnimation;
	protected static SCR_ScriptedWidgetTooltip m_CurrentTooltip;

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// Static invokers on tooltips means a lot of different components in a menu will need to listen and react to these events even though none of them but one at a time will care about the triggered tooltip. Different instances of the same component might fight to update the tooltip with their data (like in a list or menu tabs). There is also the fact that these tooltips are set on the widgets in the layouts, meaning they can get lost in huge hierarchies. 
	
	// Invokers
	// Reliance on static invokers is a bit of a bandaid solution but there is no other way for other scripts to access the tooltip class
	// If possible, only bind on hover/focus gained and make sure to unbind on lost.
	protected static ref ScriptInvokerTooltip m_OnTooltipShowInit;	// Called before creating the content widget
	protected static ref ScriptInvokerTooltip m_OnTooltipShow;		// Called after creating the content widget
	protected static ref ScriptInvokerTooltip m_OnTooltipHide;		// Called after removing the content widget

	//---- REFACTOR NOTE END ----
	
	protected const string DEBUG_BORDER_NAME = "DebugBorder";
	
	//! ---- OVERRIDES ----
	//------------------------------------------------------------------------------------------------
	//! Override in child classes to change the proxy layout
	override static Widget CreateTooltipWidget()
	{
		return GetGame().GetWorkspace().CreateWidgets("{39445BE1E35BEA33}UI/layouts/Menus/Tooltips/TooltipBaseProxy.layout");
	}

	//------------------------------------------------------------------------------------------------
	override void Show(WorkspaceWidget pWorkspace, Widget pToolTipWidget, float desiredPosX, float desiredPosY)
	{
		ForceHideCurrentTooltip();
		
		m_CurrentTooltip = this;
		
		// Create presets
		Resource rsc = BaseContainerTools.LoadContainer(m_sPresetsConfig);
		BaseContainer container = rsc.GetResource().ToBaseContainer();
		SCR_ScriptedWidgetTooltipPresets presets = SCR_ScriptedWidgetTooltipPresets.Cast(BaseContainerTools.CreateInstanceFromContainer(container));

		if (!presets)
		{
			ForceHidden();
			return;
		}
		
		// Find preset
		m_Preset = presets.FindPreset(m_sPresetTag);

		if (!m_Preset)
		{
			ForceHidden();
			return;
		}
		
		// Setup
		m_wWorkspace = pWorkspace;
		m_wTooltipProxy = pToolTipWidget;
		
		if (GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE)
			m_wHoverWidget = WidgetManager.GetWidgetUnderCursor();
		else
			m_wHoverWidget = pWorkspace.GetFocusedWidget();
		
		if (m_OnTooltipShowInit)
			m_OnTooltipShowInit.Invoke(this);
		
		// Proxy initialization
		FrameSlot.SetAnchorMin(pToolTipWidget, 0, 0);
		FrameSlot.SetAnchorMax(pToolTipWidget, 0, 0);

		FrameSlot.SetSize(pToolTipWidget, m_Preset.m_vSize[0], m_Preset.m_vSize[1]);
		FrameSlot.SetSizeToContent(pToolTipWidget, m_Preset.m_bSizeToContent);
	
		// TODO: debug visualization of paddings vs tooltip whole area
		Widget debugBorder = pToolTipWidget.FindAnyWidget(DEBUG_BORDER_NAME);
		
		#ifdef WORKBENCH
			bool showDebug = m_Preset.m_bShowDebugBorder;
		#else
			const bool showDebug = false;
		#endif
		
		if (debugBorder)
			debugBorder.SetVisible(showDebug);

		// Content creation
		if (!m_Preset.m_Content)
		{
			Print(string.Format("SCR_ScriptedWidgetTooltip - Show() - Missing content class! Tag: %1 | Conf: %2", GetTag(), m_sPresetsConfig), LogLevel.ERROR);
			return;
		}
		
		if (!m_Preset.m_Content.Init(m_wWorkspace, GetContentWrapper()))
		{
			ForceHidden();
			return;
		}
		
		// Determine and cache the correct content position inside the proxy
		InitContentPosition();
		
		// Wait for the new screen to have a screen presence, as on controller the tooltip gets triggered as soon as Enfusions flags the widget as focused, even if the hierarchy has not finished initializing yet
		DelayedPositionInit(DELAYED_INIT_WAIT_FRAMES);

		// Fade in
		if (m_Preset.m_fFadeInSpeed > 0)
		{
			m_wTooltipProxy.SetOpacity(0);
			AnimateWidget.Opacity(m_wTooltipProxy, 1, m_Preset.m_fFadeInSpeed);
		}

		//! Forced hiding events
		SCR_MenuHelper.GetOnMenuOpen().Insert(OnMenuChange);
		SCR_MenuHelper.GetOnMenuClose().Insert(OnMenuChange);
		SCR_MenuHelper.GetOnTabChange().Insert(OnTabChange);

		// Invoker
		if (m_OnTooltipShow)
			m_OnTooltipShow.Invoke(this);

		// Tick - Used to update the tooltip's position
		GetGame().GetCallqueue().CallLater(Update, m_Preset.m_fUpdateFrequency, true);
		
		// Super
		super.Show(pWorkspace, pToolTipWidget, desiredPosX, desiredPosY);
	}

	//------------------------------------------------------------------------------------------------
	override void Hide(WorkspaceWidget pWorkspace, Widget pToolTipWidget)
	{
		OnHide();
		
		super.Hide(pWorkspace, pToolTipWidget);
	}
	
	//! ---- PROTECTED ----
	//------------------------------------------------------------------------------------------------
	protected void DelayedPositionInit(int totalFrames)
	{
		if (totalFrames <= 0)
		{
			// Cache desired position and instantly place the tooltip there
			UpdatePosition(true, false, true);
			
			return;
		}
		
		totalFrames--;
		GetGame().GetCallqueue().Call(DelayedPositionInit, totalFrames);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Update()
	{
		if (!m_wWorkspace || !m_wTooltipProxy || !m_wHoverWidget || !m_Preset || !m_Preset.GetContentRoot())
		{
			Clear();
			return;
		}

		UpdatePosition(m_Preset.m_bFollowCursor && GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE);

		if (!m_wTooltipProxy.IsVisible())
			OnHide();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHide()
	{
		Clear();

		if (m_OnTooltipHide)
			m_OnTooltipHide.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuChange(ChimeraMenuBase menu)
	{
		ForceHidden();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTabChange(SCR_TabViewComponent tabView, Widget widget)
	{
		ForceHidden();
	}

	//------------------------------------------------------------------------------------------------
	protected void Clear()
	{		
		GetGame().GetCallqueue().Remove(Update);

		SCR_MenuHelper.GetOnMenuOpen().Remove(OnMenuChange);
		SCR_MenuHelper.GetOnMenuClose().Remove(OnMenuChange);
		SCR_MenuHelper.GetOnTabChange().Remove(OnTabChange);

		if (m_PositionAnimation)
			m_PositionAnimation.Stop();
		
		if (m_Preset.m_Content)
			m_Preset.m_Content.Clear();
	}
	
	// Align the content to the correct side of the proxy
	//------------------------------------------------------------------------------------------------
	protected void InitContentPosition()
	{
		SCR_TooltipPositionPreset positionPreset = m_Preset.GetInputPositionSettings();
		if (!positionPreset)
			return;
		
		Widget contentRoot = m_Preset.GetContentRoot();
		if (!contentRoot)
			return;
		
		float targetX = positionPreset.GetContentAlignmentHorizontal();
		float targetY = positionPreset.GetContentAlignmentVertical();
		
		FrameSlot.SetAlignment(contentRoot, targetX, targetY);
		FrameSlot.SetAnchorMin(contentRoot, targetX, targetY);
		FrameSlot.SetAnchorMax(contentRoot, targetX, targetY);
	}
	
	//! ---- PUBLIC ----
	//------------------------------------------------------------------------------------------------
	void UpdatePosition(bool followTarget = true, bool animate = true, bool force = false)
	{
		vector tooltipSize = m_Preset.GetTooltipSize(m_wTooltipProxy);
		bool rendered = tooltipSize[0] != 0 && tooltipSize[1] != 0;

		vector goalPosition, goalAlignment, goalAlpha;
		
		bool shouldUpdate = m_Preset.GetGoalPosition(m_Preset.GetContentRoot(), m_wHoverWidget, goalPosition, goalAlignment, goalAlpha, (!rendered || force));
		if (!shouldUpdate)
			return;
		
		// When the content layout is created, its size is 0.
		// If the proxy is SizeToContent and the tooltip is stationary, we need to slide it in position as soon as we have access to its actual size.
		if (followTarget || !rendered)
			m_fTargetPosition = {goalPosition[0], goalPosition[1]};

		// -- Proxy --
		// Move the Tooltip
		vector tooltipPos = FrameSlot.GetPos(m_wTooltipProxy);

		// Alignment
		FrameSlot.SetAlignment(m_wTooltipProxy, goalAlignment[0], goalAlignment[1]);

		// Position
		if (!animate)
			FrameSlot.SetPos(m_wTooltipProxy, goalPosition[0], goalPosition[1]);

		else if (vector.DistanceSq(Vector(tooltipPos[0], tooltipPos[1], 0), Vector(m_fTargetPosition[0], m_fTargetPosition[1], 0)) >= DISTANCE_THRESHOLD)
			m_PositionAnimation = AnimateWidget.Position(m_wTooltipProxy, m_fTargetPosition, m_Preset.m_fInterpolationSpeed);
		
		// -- Content --
		// Move the content to the correct edge of the proxy
		Widget contentRoot = m_Preset.GetContentRoot();
		if (!contentRoot)
			return;
		
		FrameSlot.SetAlignment(contentRoot, goalAlignment[0], goalAlignment[1]);
		FrameSlot.SetAnchorMin(contentRoot, goalAlignment[0], goalAlignment[1]);
		FrameSlot.SetAnchorMax(contentRoot, goalAlignment[0], goalAlignment[1]);
	}

	//------------------------------------------------------------------------------------------------
	void ForceHidden()
	{
		if (!m_wTooltipProxy)
			return;

		m_wTooltipProxy.SetVisible(false);
		OnHide();
	}
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Tooltips rely on hardcoded string tags, far from ideal. This method is also somewhat weird, as it performs extra checks based on the optional parameters. It could probably be written better. it is necessary because we must use static invokers to notify classes that a tooltip has appeared: ideally, Widgets should have build in events for their tooltips, that ScriptedWidetComponent would expose to script
	
	//------------------------------------------------------------------------------------------------
	// Check if the tooltip has the right parameters. The most important one is the tag, since it differentiates tooltips
	bool IsValid(string tag, Widget hoverWidget = null, ResourceName presets = string.Empty)
	{
		if (!m_wTooltipProxy || !m_Preset || !GetContent() || !m_wHoverWidget)
			return false;
		
		bool valid = tag == GetTag();
		
		if (hoverWidget)
			valid = valid && hoverWidget == m_wHoverWidget;
		
		if (!presets.IsEmpty())
			valid = valid && presets == m_sPresetsConfig;
		
		return valid;
	}
	
//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	string GetTag()
	{
		return m_Preset.m_sTag;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ScriptedWidgetTooltipContentBase GetContent()
	{
		return m_Preset.GetContent();
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetContentRoot()
	{
		return m_Preset.GetContentRoot();
	}
	
	//------------------------------------------------------------------------------------------------
	// Note: on the frame of creation, the proxy will still be invisible, causing this to return false
	bool IsVisible()
	{
		return m_wTooltipProxy && m_Preset && m_wTooltipProxy.IsVisible();
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetHoverWidget()
	{
		return m_wHoverWidget;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Override to change in which widget the content layout gets placed
	Widget GetContentWrapper()
	{
		return m_wTooltipProxy;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool ForceHideCurrentTooltip()
	{
		if (!m_CurrentTooltip)
			return false;
			
		m_CurrentTooltip.ForceHidden();
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_ScriptedWidgetTooltip GetCurrentTooltip()
	{
		return m_CurrentTooltip;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerTooltip GetOnTooltipShowInit()
	{
		if (!m_OnTooltipShowInit)
			m_OnTooltipShowInit = new ScriptInvokerTooltip();

		return m_OnTooltipShowInit;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerTooltip GetOnTooltipShow()
	{
		if (!m_OnTooltipShow)
			m_OnTooltipShow = new ScriptInvokerTooltip();

		return m_OnTooltipShow;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerTooltip GetOnTooltipHide()
	{
		if (!m_OnTooltipHide)
			m_OnTooltipHide = new ScriptInvokerTooltip();

		return m_OnTooltipHide;
	}
}

//------------------------------------------------------------------------------------------------
// --- Configuration classes ---
//------------------------------------------------------------------------------------------------
//! Configuration for a Tooltip
[BaseContainerProps(configRoot : true), SCR_BaseContainerCustomTitleField("m_sTag")]
class SCR_ScriptedWidgetTooltipPreset
{
	[Attribute("", UIWidgets.Auto, "Custom tag, used for finding this preset at run time.")]
	string m_sTag;

	[Attribute(desc: "Content handling class")]
	ref SCR_ScriptedWidgetTooltipContentBase m_Content;
	
	[Attribute()]
	protected ref SCR_TooltipPositionPreset m_MousePositionSettings;

	[Attribute()]
	protected ref SCR_TooltipPositionPreset m_GamepadPositionSettings;

	[Attribute("1", UIWidgets.CheckBox, "Defines if the tooltip should animate to follow mouse cursor or remain at initial position")]
	bool m_bFollowCursor;

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Given that we have objects for position settings, this should probably be in SCR_TooltipPositionPreset and allow different fixed positions based on device
	
	[Attribute(desc: "Fixed Absolute screen position")]
	protected vector m_vFixedPosition;

//---- REFACTOR NOTE END ----
	
	[Attribute("0", UIWidgets.CheckBox, desc: "Should the proxy adapt to the content: the proxy is used to check for overflowing and contains the actual Content layout. Giving it a fixed size will prevent slide in effect for overflowing tooltips. This will NOT influence the look of the content layout, as long as it fits inside the proxy and it's not set to stretch")]
	bool m_bSizeToContent;

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// This is needed for overflowing tooltips, as it allows to create a safe zone and avoid the tooltip flickering in on the frame it's actual size is initialized. It feels like a hacky workaround, there's probably a better way to tackle the issue (like starting with 0 opacity and fading in the next frame, perhaps?)
	
	[Attribute(desc: "Fixed proxy layout size. The proxy is used to check for overflowing and contains the actual Content layout.  This will NOT influence the look of the comntent layout, as long as it fits inside the proxy and it's not set to stretch")]
	vector m_vSize;

//---- REFACTOR NOTE END ----
	
	[Attribute(UIConstants.FADE_RATE_FAST.ToString(), desc: "FadeIn speed. Set to 0 to skip the animation")]
	float m_fFadeInSpeed;

	[Attribute(UIConstants.FADE_RATE_SUPER_FAST.ToString(), desc: "Movement speed")]
	float m_fInterpolationSpeed;

	[Attribute("40", desc: "Queued update delay in ms")]
	float m_fUpdateFrequency;

	[Attribute("0", UIWidgets.CheckBox)]
	bool m_bShowDebugBorder;

	//------------------------------------------------------------------------------------------------
	SCR_ScriptedWidgetTooltipContentBase GetContent()
	{
		return m_Content;
	}
	
	//------------------------------------------------------------------------------------------------
	Widget GetContentRoot()
	{
		if (!m_Content)
			return null;
		
		return m_Content.GetContentRoot();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calculate desired position and alignment
	//! Returns false if the tooltip does not need to be moved
	bool GetGoalPosition(Widget tooltipContent, Widget hoverWidget, out vector goalPosition, out vector goalAlignment, out vector goalAlpha, bool force = false)
	{
		vector tooltipSize = GetTooltipSize(tooltipContent);
		SCR_TooltipPositionPreset positionSettings = GetInputPositionSettings();

		if (!positionSettings || (positionSettings.m_eAnchor != SCR_ETooltipAnchor.CURSOR && !force))
			return false;

		positionSettings.GetGoalPosition(tooltipSize, hoverWidget, m_vFixedPosition, goalPosition, goalAlignment, goalAlpha);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	vector GetTooltipSize(Widget tooltip)
	{
		vector size;
		WorkspaceWidget workspace = GetGame().GetWorkspace();

		// Tooltip size - SCALED
		float xTooltipSize = workspace.DPIScale(m_vSize[0]);
		float yTooltipSize = workspace.DPIScale(m_vSize[1]);
		if (m_bSizeToContent && tooltip)
			tooltip.GetScreenSize(xTooltipSize, yTooltipSize);

		size[0] = xTooltipSize;
		size[1] = yTooltipSize;
		return size;
	}

	//------------------------------------------------------------------------------------------------
	SCR_TooltipPositionPreset GetInputPositionSettings()
	{
		bool isUsingMouse = GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE;
		if (isUsingMouse)
			return m_MousePositionSettings;
		else
			return m_GamepadPositionSettings;
	}
}

//------------------------------------------------------------------------------------------------
//! Class for a .conf file with multiple Tooltip presets.
[BaseContainerProps(configRoot : true)]
class SCR_ScriptedWidgetTooltipPresets
{
	[Attribute()]
	ref array<ref SCR_ScriptedWidgetTooltipPreset> m_aPresets;

	//------------------------------------------------------------------------------------------------
	//! Finds a preset by tag
	SCR_ScriptedWidgetTooltipPreset FindPreset(string tag)
	{
		foreach (SCR_ScriptedWidgetTooltipPreset preset : m_aPresets)
		{
			if (preset.m_sTag == tag)
				return preset;
		}

		return null;
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot : true)]
class SCR_TooltipPositionPreset
{
	[Attribute("0", UIWidgets.ComboBox, "Where to position the Tooltip", "", ParamEnumArray.FromEnum(SCR_ETooltipAnchor))]
	SCR_ETooltipAnchor m_eAnchor;

	[Attribute("0", UIWidgets.ComboBox, "Horizontal Alignment", "", ParamEnumArray.FromEnum(SCR_ETooltipAlignmentHorizontal))]
	SCR_ETooltipAlignmentHorizontal m_eHorizontalAlignment;

	[Attribute("0", UIWidgets.ComboBox, "Vertical Alignment", "", ParamEnumArray.FromEnum(SCR_ETooltipAlignmentVertical))]
	SCR_ETooltipAlignmentVertical m_eVerticalAlignment;

	[Attribute("8 20 0", desc: "Tooltip offset from cursor or desired position")]
	vector m_vOffset;

	[Attribute("0", UIWidgets.ComboBox, "How to deal with overflow", "", ParamEnumArray.FromEnum(SCR_ETooltipOverflowHandling))]
	SCR_ETooltipOverflowHandling m_eOverflowHandling;
		
	const float ALIGNMENT_CENTER = 0.5;
	const float ALIGNMENT_LEFT_TOP = 0;
	const float ALIGNMENT_RIGHT_DOWN = 1;

	//------------------------------------------------------------------------------------------------
	//! Requires DPI scaled values
	void GetGoalPosition(vector tooltipSize, Widget hoverWidget, vector absolutePosition, out vector goalPosition, out vector goalAlignment, out vector goalAlpha)
	{
		float xHover, yHover;
		float xHoverSize, yHoverSize;
		
		if (hoverWidget)
		{
			// Hover widget position - SCALED
			hoverWidget.GetScreenPos(xHover, yHover);
	
			// Hover widget size - SCALED
			hoverWidget.GetScreenSize(xHoverSize, yHoverSize);
		}
		
		// Cursor position - SCALED
		int xCursor, yCursor;
		WidgetManager.GetMousePos(xCursor, yCursor);

		// Workspace size - SCALED
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		float xWorkspaceSize, yWorkspaceSize;
		workspace.GetScreenSize(xWorkspaceSize, yWorkspaceSize);

		// Calculations setup
		float xOffset = m_vOffset[0];
		float yOffset = m_vOffset[1];

		float xGoal, yGoal;
		float xAlignment, yAlignment;
		float xTargetSize, yTargetSize;

		switch (m_eAnchor)
		{
			case SCR_ETooltipAnchor.CURSOR:
				xGoal = xCursor;
				yGoal = yCursor;
				break;

			case SCR_ETooltipAnchor.HOVERED_WIDGET:
				xGoal = xHover;
				yGoal = yHover;
				xTargetSize = xHoverSize;
				yTargetSize = yHoverSize;
				break;

			case SCR_ETooltipAnchor.ABSOLUTE:
				xGoal = absolutePosition[0];
				yGoal = absolutePosition[1];
				break;
		}

		CalculateGoalPosition(m_eHorizontalAlignment, xTargetSize, tooltipSize[0], xWorkspaceSize, xGoal, xOffset, xAlignment);
		CalculateGoalPosition(m_eVerticalAlignment, yTargetSize, tooltipSize[1], yWorkspaceSize, yGoal, yOffset, yAlignment);
		
		// Set the alpha - SCALED - for anchors related to workspace size, as an alternative to screen position
		if (xWorkspaceSize == 0)
			xWorkspaceSize = 1;

		if (yWorkspaceSize == 0)
			yWorkspaceSize = 1;
		
		goalAlpha[0] = (xGoal + workspace.DPIScale(xOffset)) / xWorkspaceSize;
		goalAlpha[1] = (yGoal + workspace.DPIScale(yOffset)) / yWorkspaceSize;
		
		// Set the goals - UNSCALED
		goalPosition[0] = workspace.DPIUnscale(xGoal + workspace.DPIScale(xOffset));
		goalPosition[1] = workspace.DPIUnscale(yGoal + workspace.DPIScale(yOffset));
		
		// Set the desired Alignment
		goalAlignment[0] = xAlignment;
		goalAlignment[1] = yAlignment;
	}

	//------------------------------------------------------------------------------------------------
	protected void CalculateGoalPosition(int alignmentCase, float targetSize, float tooltipSize, float maxAreaSize, inout float desiredPos, inout float offset, inout float alignment)
	{
		switch (alignmentCase)
		{
			case 0:
				break;

			case 1:
				desiredPos = desiredPos + (targetSize * ALIGNMENT_CENTER);
				alignment = ALIGNMENT_CENTER;
				break;

			case 2:
				desiredPos = desiredPos + targetSize;
				alignment = ALIGNMENT_RIGHT_DOWN;
				break;

			case 3:
				alignment = ALIGNMENT_RIGHT_DOWN;
				break;

			case 4:
				desiredPos = desiredPos + targetSize;
				break;
		}

		CheckOverflow(tooltipSize, maxAreaSize, desiredPos, offset, alignment);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if the desired position causes the Tooltip to overflow, and prevents it by inverting the alignment.
	//! The tooltip is forcefully anchored to 0, 0 on Show(), so we just need to work with alignment
	bool CheckOverflow(float tooltipSize, float maxAreaSize, inout float desiredPos, inout float offset, inout float alignment)
	{
		SCR_ETooltipOverflowType overflowType;
		
		// Extending to the right of the position
		if (alignment < ALIGNMENT_CENTER && desiredPos + offset + tooltipSize > maxAreaSize)
		{
			overflowType = SCR_ETooltipOverflowType.RIGHT;
		}

		// Extending to the left of the position
		else if (alignment > ALIGNMENT_CENTER && desiredPos + offset - tooltipSize < 0)
		{
			overflowType = SCR_ETooltipOverflowType.LEFT;
		}

		// Center
		else
		{
			if (desiredPos + offset + (tooltipSize * ALIGNMENT_CENTER) > maxAreaSize)
				overflowType = SCR_ETooltipOverflowType.CENTER_RIGHT;

			else if (desiredPos + offset - (tooltipSize * ALIGNMENT_CENTER) < 0)
				overflowType = SCR_ETooltipOverflowType.CENTER_LEFT;
		}

		if (overflowType == SCR_ETooltipOverflowType.NONE)
			return false;

		// TODO: "ignore" option for debugging
		// TODO: there seems to be an issue with overflow calculations on tooltips spawned when mouse is very close to screen edges, causing the tooltip to think it's overflowing while still far from the edge
		// -- Handle overflow --
		// Invert
		if (m_eOverflowHandling == SCR_ETooltipOverflowHandling.INVERT)
		{
			if (alignment == ALIGNMENT_CENTER)
				desiredPos = Math.Clamp(desiredPos, offset + (tooltipSize * ALIGNMENT_CENTER), maxAreaSize - (offset + (tooltipSize * ALIGNMENT_CENTER)));
	
			alignment = 1 - alignment;
		}
			
		// Stop
		else
		{
			switch (overflowType)
			{
				case SCR_ETooltipOverflowType.LEFT: 		desiredPos = tooltipSize + offset;										break;
				case SCR_ETooltipOverflowType.RIGHT: 		desiredPos = maxAreaSize - tooltipSize - offset;						break;
				case SCR_ETooltipOverflowType.CENTER_LEFT: 	desiredPos = (tooltipSize * ALIGNMENT_CENTER) + offset;					break;
				case SCR_ETooltipOverflowType.CENTER_RIGHT:	desiredPos = maxAreaSize - (tooltipSize * ALIGNMENT_CENTER) - offset;	break;
			}
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetContentAlignmentHorizontal()
	{
		switch (m_eHorizontalAlignment)
		{
			case SCR_ETooltipAlignmentHorizontal.LEFT: 			return ALIGNMENT_LEFT_TOP;
			case SCR_ETooltipAlignmentHorizontal.RIGHT:			return ALIGNMENT_RIGHT_DOWN;
			case SCR_ETooltipAlignmentHorizontal.CENTER: 		return ALIGNMENT_CENTER;
			case SCR_ETooltipAlignmentHorizontal.SIDE_LEFT:		return ALIGNMENT_LEFT_TOP;
			case SCR_ETooltipAlignmentHorizontal.SIDE_RIGHT:	return ALIGNMENT_RIGHT_DOWN;
		}
		
		return ALIGNMENT_LEFT_TOP;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetContentAlignmentVertical()
	{
		switch (m_eVerticalAlignment)
		{
			case SCR_ETooltipAlignmentVertical.TOP: 	return ALIGNMENT_LEFT_TOP;
			case SCR_ETooltipAlignmentVertical.ABOVE:	return ALIGNMENT_RIGHT_DOWN;
			case SCR_ETooltipAlignmentVertical.BELOW: 	return ALIGNMENT_LEFT_TOP;
			case SCR_ETooltipAlignmentVertical.BOTTOM:	return ALIGNMENT_RIGHT_DOWN;
			case SCR_ETooltipAlignmentVertical.CENTER:	return ALIGNMENT_CENTER;
		}
		
		return ALIGNMENT_LEFT_TOP;
	}
}

//------------------------------------------------------------------------------------------------
/*
LEFT:
	 __
	|__|_____
	|        |
	|________|


CENTER:
	    __
	 __|__|__
	|        |
	|________|


RIGHT:
		   __
	 _____|__|
	|        |
	|________|


SIDE_LEFT:
	 __ ________
	|__| 		|
	   |________|


SIDE_RIGHT:
	 ________ __
	| 		 |__|
	|________|
*/
enum SCR_ETooltipAlignmentHorizontal
{
	LEFT,
	CENTER,
	RIGHT,
	SIDE_LEFT,
	SIDE_RIGHT
}

//------------------------------------------------------------------------------------------------
/*
TOP:
	 ________
	|  |__|  |
	|________|


CENTER:
	 ________
	|   __   |
	|________|


BOTTOM:
	 ________
	|   __   |
	|__|__|__|


ABOVE:
		__
	 __|__|__
	|        |
	|________|


BELOW:
	 ________
	|        |
	|________|
	|__|
*/
enum SCR_ETooltipAlignmentVertical
{
	TOP,
	CENTER,
	BOTTOM,
	ABOVE,
	BELOW
}

//------------------------------------------------------------------------------------------------
enum SCR_ETooltipAnchor
{
	CURSOR,
	HOVERED_WIDGET,
	ABSOLUTE
}

//------------------------------------------------------------------------------------------------
enum SCR_ETooltipOverflowHandling
{
	INVERT,
	STOP
}

//------------------------------------------------------------------------------------------------
enum SCR_ETooltipOverflowType
{
	NONE,
	LEFT,
	RIGHT,
	CENTER_LEFT,
	CENTER_RIGHT
}
