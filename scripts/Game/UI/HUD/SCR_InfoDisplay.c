void SCR_InfoDisplayStartStopCallback(SCR_InfoDisplay display);
typedef func SCR_InfoDisplayStartStopCallback;
typedef ScriptInvokerBase<SCR_InfoDisplayStartStopCallback> SCR_InfoDisplayInvoker;

//#define DEBUG_ADAPTIVE_OPACITY
//#define DEBUG_INFO_DISPLAY

enum EWidgetAnchor
{
	TOPLEFT,
	TOP,
	TOPRIGHT,
	LEFT,
	CENTER,
	RIGHT,
	BOTTOMLEFT,
	BOTTOM,
	BOTTOMRIGHT
}

class SCR_InfoDisplay : GroupInfoDisplay
{
	// Attributes
	[Attribute("", UIWidgets.ResourceNamePicker, "Layout", "layout")]
	ResourceName m_LayoutPath;

	[Attribute("2", UIWidgets.ComboBox, "HUD Layer for the UI element to be placed in. Ignored when InfoDisplay is nested under another InfoDisplay.", "", ParamEnumArray.FromEnum(EHudLayers))]
	EHudLayers m_eLayer;

	[Attribute("1", UIWidgets.CheckBox, "Make UI element visible when it is created.")]
	private bool m_bShowWhenCreated;

	[Attribute("0", UIWidgets.EditBox, "Override the hierarchy to show display in front or behind other displays.")]
	int m_iOverrideZOrder;

	[Attribute("", UIWidgets.EditBox, "Name of slot in parent widget, the UI element is going to be placed in. Used when InfoDisplay is nested under another InfoDisplay.")]
	protected string m_sParentSlot;

	// Dimensions and safezone
	[Attribute("", UIWidgets.EditBox, "Name of widget containing the GUI element content. Uses the root widget, if empty.")]
	protected string m_sContentWidget;

	[Attribute("0", UIWidgets.Slider, "Adjustment to the content widget width. Can be used to provide a widget-specific padding.", "-200 200 1")]
	protected int m_iContentWidthAdjustment;

	[Attribute("0", UIWidgets.Slider, "Adjustment to the content height width. Can be used to provide a widget-specific padding.", "-200 200 1")]
	protected int m_iContentHeightAdjustment;

	// Attributes for adaptive opacity
	[Attribute("1", UIWidgets.CheckBox, "Adjusts opacity of the widget based on level of ambient light.")]
	private bool m_bAdaptiveOpacity;

	[Attribute("", UIWidgets.EditBox, "Name of the widget in the layout the adaptive opacity is applied to. If empty, layout root is used.")]
	protected string m_sAdaptiveOpacityWidgetName;

	[Attribute()]
	protected ref array<ref SCR_InfoDisplayHandler> m_aHandlers;

	private Widget m_wAdaptiveOpacity;
	private float m_fAdaptiveOpacity = 1;
	protected bool m_bShown;

	protected Widget m_wRoot;
	protected Widget m_wContent;
	protected Widget m_wSlot;
	protected SCR_HUDManagerComponent m_HUDManager;

	protected int m_iChildDisplays = 0;
	protected ref array<BaseInfoDisplay> m_aChildDisplays = new array<BaseInfoDisplay>;
	protected SCR_InfoDisplay m_pParentDisplay;
	protected bool m_bRegistered = false;

	protected IEntity m_OwnerEntity;
	protected ref array<ref SCR_InfoDisplayHandler> m_aUpdatableHandlers = {};

	protected ref SCR_InfoDisplayInvoker m_OnStart = new SCR_InfoDisplayInvoker();
	protected ref SCR_InfoDisplayInvoker m_OnStop = new SCR_InfoDisplayInvoker();

	//------------------------------------------------------------------------------------------------
	SCR_InfoDisplayHandler GetHandler(typename handlerType)
	{
		foreach (SCR_InfoDisplayHandler handler : m_aHandlers)
		{
			if (handler.Type() == handlerType)
				return handler;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_InfoDisplayInvoker GetOnStart()
	{
		return m_OnStart;
	}

	//------------------------------------------------------------------------------------------------
	SCR_InfoDisplayInvoker GetOnStop()
	{
		return m_OnStop;
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetOwnerEntity()
	{
		return m_OwnerEntity;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetContentWidget()
	{
		return m_wContent;
	}

	//------------------------------------------------------------------------------------------------
	string GetContentWidgetName()
	{
		return m_sContentWidget;
	}

	//------------------------------------------------------------------------------------------------
	void SetRootWidget(notnull Widget root)
	{
		m_wRoot = root;
	}

	//------------------------------------------------------------------------------------------------
	void SetContentWidget(notnull Widget content)
	{
		m_wContent = content;
	}

	//------------------------------------------------------------------------------------------------
	//! Show/hide InfoDisplay properly, not breaking other systems like AdaptiveOpacity or ConditionalVisibility. Use this method instead of SetVisible/SetOpacity.
	//! \param show true to show, false to hide
	//! \param speed fade rate, default 0
	//! \param curve hiding interpolation, default linear
	void Show(bool show, float speed = UIConstants.FADE_RATE_INSTANT, EAnimationCurve curve = EAnimationCurve.LINEAR)
	{
		#ifdef DEBUG_ADAPTIVE_OPACITY
		if (this.Type() == SCR_AvailableActionsDisplay)
			PrintFormat("%1 [Show] show: %2 | m_bShown: %3 | m_wSlot: %4", this, show, m_bShown, m_wSlot);
		#endif

		if (!m_wRoot)
			return;

		Widget w = m_wRoot;

		// If a slot is defined, use the slot for fading and visibility control
		if (m_wSlot)
			w = m_wSlot;

		m_bShown = show;

		float targetOpacity = show;

		if (show && m_wAdaptiveOpacity && m_wAdaptiveOpacity == w)
			targetOpacity = m_fAdaptiveOpacity;

		if (speed > 0)
		{
			#ifdef DEBUG_ADAPTIVE_OPACITY
			if (this.Type() == SCR_AvailableActionsDisplay)
				PrintFormat("%1 [Show] Opacity animation started: %2 -> %3", this, w.GetOpacity(), targetOpacity);
			#endif

			WidgetAnimationOpacity anim = AnimateWidget.Opacity(w, targetOpacity, speed, true);		// true = set visibility of widget to false, if opacity drops to 0%

			if (anim)
			{
				anim.SetCurve(curve);
				anim.GetOnCompleted().Insert(OnShownFinishedPrivate);
			}
			else
			{
				w.SetOpacity(targetOpacity);
				w.SetVisible(show);
				OnShownFinished(w, targetOpacity);
			}
		}
		else
		{
			#ifdef DEBUG_ADAPTIVE_OPACITY
			if (this.Type() == SCR_AvailableActionsDisplay)
				PrintFormat("%1 [Show] Opacity insta-changed: %2 -> %3", this, w.GetOpacity(), targetOpacity);
			#endif

			w.SetOpacity(targetOpacity);
			w.SetVisible(show);
			OnShownFinished(w, targetOpacity);
		}
	}

	//------------------------------------------------------------------------------------------------
	private void OnShownFinishedPrivate(WidgetAnimationOpacity anim)
	{
		if (!anim)
			return;

		float targetOpacity = anim.GetTargetValue();
		Widget w = anim.GetWidget();

		OnShownFinished(w, targetOpacity, anim);
	}

	//------------------------------------------------------------------------------------------------
	// Interface for overriding 'OnShownFinished'
	protected void OnShownFinished(Widget w, float targetOpacity, WidgetAnimationOpacity anim = null)
	{
		#ifdef DEBUG_INFO_DISPLAY
		PrintFormat("[OnShownFinished] %1 | desired opacity: %2 (real opacity: %4) | shown: %3", this, targetOpacity, m_bShown, w.GetOpacity());
		#endif
	}

	//------------------------------------------------------------------------------------------------
	bool IsShown()
	{
		return m_bShown;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}

	//------------------------------------------------------------------------------------------------
	//! Get width and height of the InfoDisplay element, optionally with safezones adjustments
	//! \param[out] width
	//! \param[out] height
	//! \param addSafezones
	//! \return false if content widget is null, true otherwise
	bool GetDimensions(out float width, out float height, bool addSafezones = true)
	{
		if (!m_wContent)
		{
			width = 0;
			height = 0;
			return false;
		}

		m_wContent.GetScreenSize(width, height);

		WorkspaceWidget workspace = m_wContent.GetWorkspace();
		width = workspace.DPIUnscale(width);
		height = workspace.DPIUnscale(height);

		if (addSafezones)
		{
			width += m_iContentWidthAdjustment;
			height += m_iContentHeightAdjustment;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Get width and height of the InfoDisplay element, optionally with safezones adjustments
	//! \param[out] x
	//! \param[out] y
	//! \param anchor
	//! \param addSafezones
	//! \return false if content widget is null, true otherwise
	bool GetAnchorPosition(out float x, out float y, EWidgetAnchor anchor = EWidgetAnchor.TOPLEFT, bool addSafezones = true)
	{
		if (!m_wContent)
		{
			x = 0;
			y = 0;
			return false;
		}

		float width, height;

		GetDimensions(width, height, addSafezones);

		m_wContent.GetScreenPos(x, y);

		WorkspaceWidget workspace = m_wContent.GetWorkspace();
		x = workspace.DPIUnscale(x) - m_iContentWidthAdjustment * 0.5 * addSafezones;
		y = workspace.DPIUnscale(y) - m_iContentHeightAdjustment * 0.5 * addSafezones;

		switch (anchor)
		{
			case EWidgetAnchor.TOPLEFT:

				break;

			case EWidgetAnchor.TOP:

				x += width / 2;
				break;

			case EWidgetAnchor.TOPRIGHT:

				x += width;
				break;

			case EWidgetAnchor.LEFT:

				y += height / 2;
				break;

			case EWidgetAnchor.CENTER:

				y += height / 2;
				x += width / 2;
				break;

			case EWidgetAnchor.RIGHT:

				y += height / 2;
				x += width;
				break;

			case EWidgetAnchor.BOTTOMLEFT:

				y += height;
				break;

			case EWidgetAnchor.BOTTOM:

				y += height;
				x += width / 2;
				break;

			case EWidgetAnchor.BOTTOMRIGHT:

				y += height;
				x += width;
				break;

			default:

				break;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	void RegisterToHudManager()
	{
		m_bShown = false;
		m_HUDManager.RegisterHUDElement(this);
		m_bRegistered = true;
	}

	//------------------------------------------------------------------------------------------------
	private void CreateDisplayLegacy(IEntity owner)
	{
		// If SCR_InfoDisplaySlotHandler handler is attached, this function will prematurely return here.
		// This is intended behavior inorder to protect backwards compatability.
		if (m_wRoot)
			return;

		if (!m_HUDManager)
			m_HUDManager = SCR_HUDManagerComponent.GetHUDManager();

		if (!m_HUDManager)
			return;

		// Nested placement; used when parent InfoDisplay is properly defined
		if (m_pParentDisplay)
		{
			Widget wParentRoot = m_pParentDisplay.m_wRoot;

			if (wParentRoot)
			{
				m_wSlot = wParentRoot.FindAnyWidget(m_sParentSlot);
				WorkspaceWidget wWorkspace = GetGame().GetWorkspace();

				if (m_wSlot && wWorkspace)
					m_wRoot = wWorkspace.CreateWidgets(m_LayoutPath, m_wSlot);
			}
		}

		// Default placement; used when there is no parent InfoDisplay or it's slot cannot be SCR_BannedAddonsDetectedDialog
		if (!m_wRoot)
		{
			m_pParentDisplay = null;
			m_sParentSlot = "";

			RegisterToHudManager();
			m_wRoot = m_HUDManager.CreateLayout(m_LayoutPath, m_eLayer, m_iOverrideZOrder);
		}

		if (!m_wRoot)
			return;

		// Detect 'content widget'
		if (m_sContentWidget != string.Empty)
			m_wContent = m_wRoot.FindAnyWidget(m_sContentWidget);

		if (!m_wContent)
			m_wContent = m_wRoot;

		AdaptiveOpacity_Initialize();

		#ifdef DEBUG_INFO_DISPLAY
		PrintFormat("%1 [OnStartDraw] m_wRoot: %2", this, m_wRoot);
		#endif
	}

	//------------------------------------------------------------------------------------------------
	protected override event void OnStartDraw(IEntity owner)
	{
		m_HUDManager = SCR_HUDManagerComponent.GetHUDManager();

		foreach (SCR_InfoDisplayHandler handler : m_aHandlers)
		{
			if (handler.m_bCanUpdate)
				m_aUpdatableHandlers.Insert(handler);
			handler.Initialize(this);
		}

		foreach (SCR_InfoDisplayHandler handler : m_aHandlers)
		{
			handler.OnStart(this);
		}

		m_OnStart.Invoke(this);

		if (GetHandler(SCR_InfoDisplaySlotHandler) == null && GetHandler(SCR_InfoDisplayLayerHandler) == null)
			CreateDisplayLegacy(owner);

		if (!m_wContent)
			m_wContent = m_wRoot;

		AdaptiveOpacity_Initialize();
		Show(m_bShowWhenCreated);
	}

	//------------------------------------------------------------------------------------------------
	private void AdaptiveOpacity_Initialize()
	{
		#ifdef DEBUG_ADAPTIVE_OPACITY
		if (this.Type() == SCR_AvailableActionsDisplay)
			PrintFormat(">> %1 >> AdaptiveOpacity_Initialize", this);
		#endif
		
		// Safecheck for multiple adaptive opacity (parent & child display)
		if (m_bAdaptiveOpacity && m_pParentDisplay && m_pParentDisplay.m_bAdaptiveOpacity)
		{
			m_bAdaptiveOpacity = false;
			PrintFormat("[AdaptiveOpacity] Duplicate AO disabled on info display %1. Parent display %2 already has AO enabled.", this, m_pParentDisplay);
		}

		// Adaptive opacity initialization
		if (m_bAdaptiveOpacity)
		{
			if (m_sAdaptiveOpacityWidgetName != string.Empty)
				m_wAdaptiveOpacity = m_wRoot.FindAnyWidget(m_sAdaptiveOpacityWidgetName);

			if (!m_wAdaptiveOpacity)
				m_wAdaptiveOpacity = m_wRoot;

			m_HUDManager.GetSceneBrightnessChangedInvoker().Insert(AdaptiveOpacity_OnScreenBrightnessChange);
			AdaptiveOpacity_Update(m_HUDManager.GetAdaptiveOpacity(), m_HUDManager.GetSceneBrightness(), true);
		}
	}	

	//------------------------------------------------------------------------------------------------
	private void AdaptiveOpacity_OnScreenBrightnessChange(float opacity, float sceneBrightness)
	{
		AdaptiveOpacity_Update(opacity, sceneBrightness, false);
	}	
		
	//------------------------------------------------------------------------------------------------
	protected void AdaptiveOpacity_Update(float opacity, float sceneBrightness, bool init = false)
	{
		if (!m_bAdaptiveOpacity)
			return;
		
		#ifdef DEBUG_ADAPTIVE_OPACITY
		if (this.Type() == SCR_AvailableActionsDisplay)
			PrintFormat("%1 [AdaptiveOpacity_Update] opacity: %2 | sceneBrightness: %3", this, opacity, sceneBrightness);
		#endif

		// Store the calculated adaptive opacity value, so it can be used by other methods, like Show()
		m_fAdaptiveOpacity = opacity;

		// We can terminate if info display is not shown, as adaptive opacity is already stored ^^
		if (!m_bShown && !init)
			return;

		// Detect running opacity animation
		WidgetAnimationOpacity animation = WidgetAnimationOpacity.Cast(AnimateWidget.GetAnimation(m_wAdaptiveOpacity, WidgetAnimationOpacity));

		if (animation)
		{
			float targetOpacity = animation.GetTargetValue();

			// Terminate, if already fading out (should not be needed, if properly implemented as m_bShown *should* be false)
			if (targetOpacity < 0.01)
				return;

			#ifdef DEBUG_ADAPTIVE_OPACITY
			if (this.Type() == SCR_AvailableActionsDisplay)
				PrintFormat("%1 [AdaptiveOpacity_Update] Updated running opacity animation; target opacity %2 -> %3", this, targetOpacity, opacity);
			#endif

			animation.SetTargetValue(opacity);
		}
		else
		{
			#ifdef DEBUG_ADAPTIVE_OPACITY
			if (this.Type() == SCR_AvailableActionsDisplay)
				PrintFormat("%1 [AdaptiveOpacity_Update] %2 -> %3", this, m_wAdaptiveOpacity.GetOpacity(), opacity);
			#endif

			if (m_wAdaptiveOpacity && m_wAdaptiveOpacity.GetOpacity())
				m_wAdaptiveOpacity.SetOpacity(opacity);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override event void OnStopDraw(IEntity owner)
	{
		foreach (SCR_InfoDisplayHandler handler : m_aHandlers)
		{
			handler.OnStop(this);
		}

		m_OnStop.Invoke(this);

		// Adaptive opacity initialization
		if (m_wRoot && m_HUDManager && m_bAdaptiveOpacity)
			m_HUDManager.GetSceneBrightnessChangedInvoker().Remove(AdaptiveOpacity_OnScreenBrightnessChange);

		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();

		if (m_HUDManager && m_bRegistered)
			m_HUDManager.UnregisterHUDElement(this);

		#ifdef DEBUG_INFO_DISPLAY
		PrintFormat("%1 [OnStopDraw] m_wRoot: %2", this, m_wRoot);
		#endif

		m_aUpdatableHandlers.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected override event void UpdateValues(IEntity owner, float timeSlice)
	{
		foreach (SCR_InfoDisplayHandler handler : m_aUpdatableHandlers)
		{
			if (handler.IsEnabled())
				handler.OnUpdate(timeSlice);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override event void OnInit(IEntity owner)
	{
		// Get slotted children info
		m_iChildDisplays = GetInfoDisplays(m_aChildDisplays);

		m_OwnerEntity = owner;		
		
		foreach (BaseInfoDisplay pDisplay : m_aChildDisplays)
		{
			SCR_InfoDisplay pInfoDisplay = SCR_InfoDisplay.Cast(pDisplay);

			if (pInfoDisplay)
				pInfoDisplay.m_pParentDisplay = this;
		}
	}
}
