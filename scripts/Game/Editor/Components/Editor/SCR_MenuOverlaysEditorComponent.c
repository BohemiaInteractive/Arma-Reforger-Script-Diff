[ComponentEditorProps(category: "GameScripted/Editor", description: "Overlays for in-game editor. Works only with SCR_EditorBaseEntity!", icon: "WBData/ComponentEditorProps/componentEditor.png")]
class SCR_MenuOverlaysEditorComponentClass: SCR_BaseEditorComponentClass
{
};

/** @ingroup Editor_Components
*/

/*!
Manager of menu overlays.

Multiple overlays can be present at the same time, one per each layer type (SCR_EditorMenuOverlayLayer).

Overlays are created in the menu created by SCR_MenuEditorComponent. They won't work if that editor component is not available.
*/
class SCR_MenuOverlaysEditorComponent : SCR_BaseEditorComponent
{	
	[Attribute()]
	private ref array<ref SCR_EditorMenuOverlayLayer> m_aLayers;

	/*!
	Get overlay layer script object which holds all of its parameters.
	\param layerType Overlay type
	\return Layer object
	*/
	SCR_EditorMenuOverlayLayer GetOverlayLayer(EEditorMenuOverlayLayer layerType)
	{
		if (!m_aLayers) return null;
		foreach (SCR_EditorMenuOverlayLayer layer: m_aLayers)
		{
			if (layer.GetLayer() == layerType) return layer;
		}
		return null;
	}
	
	override void ResetEditorComponent()
	{
		foreach (SCR_EditorMenuOverlayLayer layer: m_aLayers)
		{
			layer.ResetOverlay();
		}
	}
	override void EOnEditorDebug(array<string> debugTexts)
	{
		if (!m_aLayers || m_aLayers.IsEmpty()) return;
		foreach (SCR_EditorMenuOverlayLayer layer: m_aLayers)
		{
			SCR_EditorMenuOverlay overlay = layer.GetCurrentOverlay();
			if (overlay) debugTexts.Insert(string.Format("Overlay %1: %2", Type().EnumToString(EEditorMenuOverlayLayer, layer.GetLayer()), overlay.GetDisplayName()));
		}
	}
	override void EOnEditorActivate()
	{
		if (!m_aLayers || m_aLayers.IsEmpty()) return;
		foreach (SCR_EditorMenuOverlayLayer layer: m_aLayers)
		{
			layer.ActivateLayer();
		}
	}
	override void EOnEditorPostActivate()
	{
		if (!m_aLayers || m_aLayers.IsEmpty()) return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace) return;
		
		SCR_MenuEditorComponent menuEditor = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent, true));
		if (!menuEditor) return;
		
		EditorMenuBase menu = menuEditor.GetMenu();
		if (!menu) return;
		
		foreach (SCR_EditorMenuOverlayLayer layer: m_aLayers)
		{
			layer.PostActivateLayer(menuEditor, workspace);
		}
	}
	override void EOnEditorDeactivate()
	{
		if (!m_aLayers || m_aLayers.IsEmpty()) return;
		
		SCR_MenuEditorComponent menuEditor = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		if (!menuEditor) return;
		
		EditorMenuBase menu = menuEditor.GetMenu();
		if (!menu) return;
		
		foreach (SCR_EditorMenuOverlayLayer layer: m_aLayers)
		{
			layer.ExitLayer(menuEditor);
		}
	}
};
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditorMenuOverlayLayer, "m_Layer")]
class SCR_EditorMenuOverlayLayer
{
	//--- Name of the frame widget in the root in which the overlay is created
	private const string DEFAULT_ROOT_FRAME = "Frame";
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorMenuOverlayLayer))]
	private EEditorMenuOverlayLayer m_Layer;
	
	//[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorMenuOverlayRoot))]
	//private EEditorMenuOverlayRoot m_Root;
	
	[Attribute()];
	private bool m_bCanHide;
	
	[Attribute(defvalue: "", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorMenuOverlayLayer))]
	private EEditorMenuOverlayLayer m_CustomRoot;
	
	[Attribute()];
	private bool m_bAlignToRoot;
	
	[Attribute(desc: "Name of the frame widget in the root in which the overlay is created.\nWhen undefined, default will be used.")];
	private string m_sRootFrameName;
	
	[Attribute(defvalue: "-1")];
	private int m_iZOrder;
	
	//[Attribute()]
	//private bool m_bCanHide;
	
	[Attribute(defvalue: "0")]
	private int m_iCurrentOverlay;
	
	[Attribute()]
	private ref array<ref SCR_EditorMenuOverlay> m_aOverlays;
	
	private int m_iDefaultOverlay;
	private Widget m_RootWidget;
	private Widget m_RootWidgetDefault;
	private SCR_EditorMenuOverlayLayer m_RootLayer;
	private ref ScriptInvoker Event_OnOverlayChanged = new ScriptInvoker;
	
	/*!
	Get layer type.
	\return Layer type
	*/
	EEditorMenuOverlayLayer GetLayer()
	{
		return m_Layer;
	}
	/*!
	Get all overlays within this layer.
	\param[out] outOverlays Array to be filled with overlays
	\return Number of overlays
	*/
	int GetOverlays(out notnull array<SCR_EditorMenuOverlay> outOverlays)
	{
		outOverlays.Clear();
		if (!m_aOverlays) return 0;
		int countOverlays = m_aOverlays.Count();
		for (int i = 0; i < countOverlays; i++)
		{
			outOverlays.Insert(m_aOverlays[i]);
		}
		return countOverlays;
	}
	/*!
	Set current overlay.
	\param index Overlay index from pre-defined list
	\param forced Set the current overlay even if the current one is the same (used on init)
	*/
	void SetCurrentOverlay(int index, bool forced = false)
	{
		if (!m_aOverlays || (index == m_iCurrentOverlay && !forced)) return;
		
		if (index < 0 || index >= m_aOverlays.Count())
		{
			Print(string.Format("Canot set current overlay of %1, index out of bounds (is %2, must be 0-%3)", typename.EnumToString(EEditorMenuOverlayLayer, m_Layer), index, m_aOverlays.Count() - 1), LogLevel.ERROR);
			return;
		}
		
		if (m_CustomRoot != 0) UpdateCustomRoot();
		if (!m_RootWidget) return;
		
		m_aOverlays[m_iCurrentOverlay].DeleteWidget();
		m_iCurrentOverlay = index;
		m_aOverlays[m_iCurrentOverlay].CreateWidget(m_RootWidget, m_iZOrder, m_bAlignToRoot);
		Event_OnOverlayChanged.Invoke(m_aOverlays[m_iCurrentOverlay]);
		
		if (m_bCanHide && m_CustomRoot != 0)
		{
			SCR_MenuEditorComponent menuEditor = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
			if (menuEditor && !menuEditor.IsVisible()) m_aOverlays[m_iCurrentOverlay].SetWidgetOpacity(0);
		}
	}
	/*!
	Set overlay to defautl value.
	*/
	void ResetOverlay()
	{
		SetCurrentOverlay(m_iDefaultOverlay);
	}
	/*!
	Get current overlay.
	\return Overlay
	*/
	SCR_EditorMenuOverlay GetCurrentOverlay()
	{
		if (!m_aOverlays || m_aOverlays.IsEmpty()) return null;
		return m_aOverlays[m_iCurrentOverlay];
	}
	/*!
	Get index of current overlay.
	\return Index
	*/
	int GetCurrentOverlayIndex()
	{
		return m_iCurrentOverlay;
	}
	/*!
	Get widget representing the overlay.
	\return Widget
	*/
	Widget GetOverlayWidget()
	{
		if (!m_aOverlays || m_aOverlays.IsEmpty()) return null;
		return m_aOverlays[m_iCurrentOverlay].GetWidget();
	}
	void ActivateLayer()
	{
		if (m_CustomRoot == 0 || m_CustomRoot == m_Layer) return;
		
		SCR_MenuOverlaysEditorComponent overlayManager = SCR_MenuOverlaysEditorComponent.Cast(SCR_MenuOverlaysEditorComponent.GetInstance(SCR_MenuOverlaysEditorComponent));
		if (!overlayManager) return;
		
		SCR_EditorMenuOverlayLayer rootLayer = overlayManager.GetOverlayLayer(m_CustomRoot);
		if (!rootLayer) return;
		
		m_RootLayer = rootLayer;
		m_RootLayer.GetOnOverlayChanged().Insert(RefreshOverlay);
	}
	/*!
	Create widgets of all overlays within the layer.
	Inactive overlays will be hidden, and activated only when they become current.
	\param menu Menu in which the overlay is screated
	\param workspace GUI workspace
	*/
	void PostActivateLayer(SCR_MenuEditorComponent menu, WorkspaceWidget workspace)
	{
		if (!menu)
			return;
		
		m_RootWidgetDefault = menu.GetMenu().GetRootWidget();
		if (m_CustomRoot != 0)
		{
			if (m_bCanHide)
			{
				SCR_HideEditorUIComponent hideComponent = SCR_HideEditorUIComponent.Cast(menu.GetMenuComponent().GetHideableWidget().FindHandler(SCR_HideEditorUIComponent));
				if (hideComponent) hideComponent.GetOnOpacityChange().Insert(OnHide);
			}
		}
		else
		{			
			if (m_bCanHide)
			{
				//--- Hidden on key press, create in 'Hide' layer
				SCR_HideEditorUIComponent hideComponent = SCR_HideEditorUIComponent.Cast(menu.GetMenuComponent().GetHideableWidget().FindHandler(SCR_HideEditorUIComponent));
				if (hideComponent) m_RootWidget = hideComponent.GetWidget();
			}
			if (!m_RootWidget) m_RootWidget = m_RootWidgetDefault;
			SetCurrentOverlay(m_iCurrentOverlay, true);
		}
		//SetCurrentOverlay(m_iCurrentOverlay, true);
	}
	/*!
	Delete all overlays within the layer.
	*/
	void ExitLayer(SCR_MenuEditorComponent menu)
	{
		if (!m_aOverlays || m_aOverlays.IsEmpty()) return;
		
		m_aOverlays[m_iCurrentOverlay].DeleteWidget();
		
		SCR_HideEditorUIComponent hideComponent = SCR_HideEditorUIComponent.Cast(menu.GetMenuComponent().GetHideableWidget().FindHandler(SCR_HideEditorUIComponent));
		if (hideComponent) hideComponent.GetOnOpacityChange().Remove(OnHide);
	}
	/*!
	Get event invoker every time the overlay changes.
	\return Script invoker
	*/
	ScriptInvoker GetOnOverlayChanged()
	{
		return Event_OnOverlayChanged;
	}
	
	protected void OnHide(float opacity)
	{
		if (!m_aOverlays || m_aOverlays.IsEmpty()) return;
		m_aOverlays[m_iCurrentOverlay].SetWidgetOpacity(opacity);
	}
	protected void RefreshOverlay()
	{
		SetCurrentOverlay(m_iCurrentOverlay, true);
	}
	protected void UpdateCustomRoot()
	{
		if (m_RootLayer)
		{
			Widget rootWidget = m_RootLayer.GetOverlayWidget();
			if (rootWidget)
			{
				m_RootWidget = GetRootWidget(rootWidget, m_sRootFrameName);
				if (!m_RootWidget) m_RootWidget = GetRootWidget(rootWidget, DEFAULT_ROOT_FRAME);
				
				/*
				if (rootWidget.GetName() == "Frame")
					m_RootWidget = rootWidget;
				else
					m_RootWidget = rootWidget.FindAnyWidget("Frame");
				*/

				if (m_RootWidget)
				{
					if (m_RootWidget.GetTypeID() == WidgetType.FrameWidgetTypeID)
						return;
					else
						Print(string.Format("Custom overlay root of layer %1 is not a frame widget!", Type().EnumToString(EEditorMenuOverlayLayer, m_Layer)), LogLevel.ERROR);
				}
				else
				{
					Print(string.Format("Custom overlay root of layer %1 is missing 'Frame' widget!", Type().EnumToString(EEditorMenuOverlayLayer, m_Layer)), LogLevel.ERROR);
				}
			}
		}
		m_RootWidget = m_RootWidgetDefault;
	}
	protected Widget GetRootWidget(Widget rootWidget, string widgetName)
	{
		if (rootWidget.GetName() == widgetName)
			return rootWidget;
		else
			return rootWidget.FindAnyWidget(widgetName);
	}
	void SCR_EditorMenuOverlayLayer()
	{
		m_iDefaultOverlay = m_iCurrentOverlay;
	}
};

[BaseContainerProps(), BaseContainerCustomTitleField("m_sDisplayName")]
class SCR_EditorMenuOverlay
{
	[Attribute()]
	private LocalizedString m_sDisplayName;
	
	[Attribute(params: "layout")]
	private ResourceName m_Overlay;
	
	private Widget m_Widget;
	
	/*!
	Get name of the overlay.
	\return Name
	*/
	string GetDisplayName()
	{
		return m_sDisplayName;
	}
	
	/*!
	Create overlay widget.
	\param rootWidget Parent widget in which the overlay is created
	\param workspace GUI workspace
	\return Created widget
	*/
	void CreateWidget(Widget rootWidget, int zOrder, bool align)//, WorkspaceWidget workspace)
	{
		if (m_Widget || m_Overlay.IsEmpty() || !rootWidget) return;
		
		ArmaReforgerScripted game = GetGame();
		if (!game) return;
		
		WorkspaceWidget workspace = game.GetWorkspace();
		if (!workspace) return;
		
		m_Widget = workspace.CreateWidgets(m_Overlay, rootWidget);
		if (!m_Widget) return;

		m_Widget.SetZOrder(zOrder);
		if (align)
		{
			float left, top, right, bottom;
			FrameSlot.GetOffsets(rootWidget, left, top, right, bottom);
			vector anchorsMin = FrameSlot.GetAnchorMin(rootWidget);
			vector anchorsMax = FrameSlot.GetAnchorMin(rootWidget);
			vector alignment = FrameSlot.GetAlignment(rootWidget);
			vector size = FrameSlot.GetSize(m_Widget);
			
			FrameSlot.SetAlignment(m_Widget, alignment[0], alignment[1]);
			FrameSlot.SetAnchorMin(m_Widget, anchorsMin[0], anchorsMin[1]);
			FrameSlot.SetAnchorMax(m_Widget, anchorsMax[0], anchorsMax[1]);
			FrameSlot.SetOffsets(m_Widget, left, top, right, bottom);
			FrameSlot.SetSize(m_Widget, size[0], size[1]);
		}
		else
		{
			FrameSlot.SetAnchorMin(m_Widget, 0, 0);
			FrameSlot.SetAnchorMax(m_Widget, 1, 1);
			FrameSlot.SetOffsets(m_Widget, 0, 0, 0, 0);
		}
	}
	/*!
	Delete overlay widget.
	*/
	void DeleteWidget()
	{
		if (!m_Widget) return;
		m_Widget.RemoveFromHierarchy();
		m_Widget = null;
	}
	/*!
	Set opacity of the overlay widget.
	\param opacity
	*/
	void SetWidgetOpacity(float opacity)
	{
		if (m_Widget) m_Widget.SetOpacity(opacity)
	}
	/*!
	Get overlay widget.
	\return Widget
	*/
	Widget GetWidget()
	{
		return m_Widget;
	}
};