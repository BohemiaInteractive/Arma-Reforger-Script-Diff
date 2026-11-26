class SCR_AxisCurveGraphDescriptionComponent : SCR_WLibComponentBase
{
	[Attribute(defvalue: "{D94B3F054F9F4562}UI/layouts/Menus/SettingsMenu/CustomWidgets/SettingsInputCurveGraphDescription.layout", desc: "Layout", uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_sLayout;

	[Attribute(defvalue: "", desc: "Profile Name")]
	protected string m_sProfileName;

	protected Widget m_wDescriptionContainer;
	protected Widget m_wSpawnedLayout;
	protected ref array<ref CanvasWidgetCommand> m_aDrawCommands = {};

	//------------------------------------------------------------------------------------------------
	void Init(notnull Widget w)
	{
		Widget parent = w.GetParent();
		while (parent && !m_wDescriptionContainer)
		{
			m_wDescriptionContainer = VerticalLayoutWidget.Cast(parent.FindAnyWidget("Description"));
			parent = parent.GetParent();
		}

		if (!m_wDescriptionContainer)
			w.RemoveHandler(this);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);

		if (m_wSpawnedLayout)
			return false; // focus is called twice, once from m_wRoot, and then for the second time from slider, which is a child of root, so dont spawn twice

		m_wSpawnedLayout = GetGame().GetWorkspace().CreateWidgets(m_sLayout, m_wDescriptionContainer);
		if (m_wSpawnedLayout)
			UpdateGraph();

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);

		GetGame().GetCallqueue().Remove(UpdateGraph);
		if (!m_wSpawnedLayout)
			return false;

		m_wSpawnedLayout.RemoveFromHierarchy();
		m_wSpawnedLayout = null;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateGraph()
	{
		BaseContainer gamepadProfile = SCR_InputHelper.GetGamepadProfileContainer();
		if (!gamepadProfile)
			return;

		BaseContainer entry = gamepadProfile.GetObject(m_sProfileName);
		if (entry)
			RequestGraphUpdate(entry);
	}

	//------------------------------------------------------------------------------------------------
	void RequestGraphUpdate(notnull BaseContainer curveParentContainer)
	{
		if (!m_wSpawnedLayout)
			return;

		CanvasWidget canvas = CanvasWidget.Cast(m_wSpawnedLayout.FindAnyWidget("Canvas"));
		if (!canvas)
			return;

		float maxWidth;
		float maxHeight;
		canvas.GetScreenSize(maxWidth, maxHeight);
		if (maxWidth == 0 && maxHeight == 0)
		{
			GetGame().GetCallqueue().CallLater(UpdateGraph);
			return; // wait for the convas to be fully initialized, as its width and height determine the scale of the graph
		}

		SCR_InputHelper.DrawInputCurve(curveParentContainer, canvas, m_aDrawCommands, false, SCR_InputHelper.ENGINE_SETTINGS_INPUT_DEAD_ZONE_NAME);
	}
}