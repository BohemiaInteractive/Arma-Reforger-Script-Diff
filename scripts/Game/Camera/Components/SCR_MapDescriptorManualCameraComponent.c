//! @ingroup ManualCamera

//! Camera representation in the map
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_MapDescriptorManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute("0", UIWidgets.SearchComboBox, "Descriptor type.", enums: ParamEnumArray.FromEnum(EMapDescriptorType))]
	protected EMapDescriptorType m_iBaseType;
	
	[Attribute(desc: "Quad name from imageset defined on MapWidget.")]
	protected string m_sImageDef;
	
	[Attribute("0 0 0 1", desc: "Color of camera icon.")]
	protected ref Color m_IconColor;
	
	[Attribute("0 0 0 0.2", desc: "Color of the view cone.")]
	protected ref Color m_ViewConeColor;
	
	[Attribute("0.375", uiwidget: UIWidgets.Slider, "Icon size coefficient.", params: "0 1 0.125")]
	protected float m_fIconScale;
	
	[Attribute("1000", desc: "Maximum length of the view cone (and also maximum distance of tracing method).")]
	protected float m_fMaxViewConeLength;
	
	[Attribute("ManualCamera", desc: "Name of canvas widget in which view cone will be rendered.")]
	protected string m_sMapCanvasWidgetName;
	
	protected SCR_MapEntity m_MapEntity;
	protected ref MapItem m_MapItem;
	protected CanvasWidget m_MapCanvas;
	protected ref PolygonDrawCommand m_DrawLine = new PolygonDrawCommand();
	protected ref array<ref CanvasWidgetCommand> m_MapDrawCommands = { m_DrawLine };
	
	protected WorkspaceWidget m_Workspace;
	protected BaseWorld m_World;
	protected ref TraceParam m_Trace = new TraceParam();
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] width
	//! \param[in] height
	//! \param[out] dir
	//! \param[out] traceCoef
	void TraceScreenPos(float width, float height, out vector dir, out float traceCoef)
	{
		vector pos = m_Workspace.ProjScreenToWorldNative(width, height, dir, m_World);
		dir *= m_fMaxViewConeLength;
		
		TraceParam trace = new TraceParam();
		m_Trace.Start = pos;
		m_Trace.End = pos + dir;
		
		traceCoef = Math.Max(m_World.TraceMove(m_Trace, null), traceCoef);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!m_MapEntity.IsOpen())
			return;
		
		//--- Update icon position
		m_MapItem.SetPos(param.transform[3][0], param.transform[3][2]);
		m_MapItem.SetAngle(param.rotOriginal[0] + param.rotDelta[0]);
		
		//--- Initialize view cone canvas (must be done after the map was opened)
		if (!m_MapCanvas && m_MapEntity.GetMapMenuRoot())
			m_MapCanvas = CanvasWidget.Cast(m_MapEntity.GetMapMenuRoot().FindWidget(m_sMapCanvasWidgetName));
		
		if (!m_MapCanvas)
			return;
		
		//--- Trace left and right side of the screen
		const int widthLeft = 0; // TODO: check for good const usage
		int widthRight = m_Workspace.GetWidth();
		float height = m_Workspace.GetHeight() / 2;
		float traceCoef;
		vector dirLeft, dirRight;
		TraceScreenPos(widthLeft, height, dirLeft, traceCoef);
		TraceScreenPos(widthRight, height, dirRight, traceCoef);
		
		//--- Convert positions to map space
		vector posCam = param.transform[3];
		vector posLeft = posCam + dirLeft * traceCoef;
		vector posRight = posCam + dirRight * traceCoef;
		
		int posCamX, posCamY, posLeftX, posLeftY, posRightX, posRightY;
		float mapZoom = m_MapEntity.GetCurrentZoom();
		m_MapEntity.WorldToScreenCustom(posCam[0], posCam[2], posCamX, posCamY, mapZoom, true);
		m_MapEntity.WorldToScreenCustom(posLeft[0], posLeft[2], posLeftX, posLeftY, mapZoom, true);
		m_MapEntity.WorldToScreenCustom(posRight[0], posRight[2], posRightX, posRightY, mapZoom, true);
		
		//--- Draw the view cone
		m_DrawLine.m_Vertices = {posCamX, posCamY, posLeftX, posLeftY, posRightX, posRightY};
		m_MapCanvas.SetDrawCommands(m_MapDrawCommands);
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		if (!m_MapEntity)
			return false;
		
		m_MapItem = m_MapEntity.CreateCustomMapItem();
		m_MapItem.SetBaseType(m_iBaseType);
		m_MapItem.SetImageDef(m_sImageDef);
		
		MapDescriptorProps props = m_MapItem.GetProps();
		props.SetFrontColor(m_IconColor);
		props.SetOutlineColor(Color.FromInt(Color.BLACK));
		props.SetIconSize(1, m_fIconScale, m_fIconScale);
		props.Activate(true);
		m_MapItem.SetProps(props);
		
		m_DrawLine.m_iColor = m_ViewConeColor.PackToInt();
			
		m_Workspace = GetGame().GetWorkspace();
		m_World = GetCameraEntity().GetWorld();
		m_Trace = new TraceParam();
		m_Trace.Flags = TraceFlags.WORLD | TraceFlags.OCEAN;
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		if (m_MapItem)
			m_MapItem.Recycle();
	}
}
