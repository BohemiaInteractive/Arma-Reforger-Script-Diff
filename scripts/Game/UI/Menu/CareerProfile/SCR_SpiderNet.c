//------------------------------------------------------------------------------------------------
class SCR_SpiderNet: ScriptedWidgetComponent
{
	protected CanvasWidget m_wCanvasWidget;
	
	protected vector m_vCenter;
	protected float m_fRadius;
	protected const float m_fLegendOffsetAsRadius = 0.2;
	
	protected ref array<ref CanvasWidgetCommand> m_aCanvasCommands;
	protected ref array<float> m_aVertices;
	protected ref array<float> m_aOldVertices;
	protected ref array<float> m_aLegendPositions;
	protected ImageWidget m_wbackgroundImage;	
		
	protected ref array<ref Widget> m_aLegends;
	
	protected Widget m_wSpiderNetFrame;
	
	protected SCR_CareerSpecializationsUI m_CareerSpecializationsHandler = null;
	
	protected int m_iNumberOfPoints;
	protected ref array<float> m_aSpPoints, m_aOldSpPoints;
	
	[Attribute(defvalue: "0.761 0.386 0.08 0.059", desc: "Outline Graph Color")]
	protected ref Color m_iPolygonColor;
	
	[Attribute(defvalue: "0.761 0.386 0.08 1", desc: "Outline Graph Color")]
	protected ref Color m_iOutlineColor;
	
	[Attribute(defvalue: "0.761 0.571 0.347 0.059", desc: "Outline Graph Color for OLD stats")]
	protected ref Color m_iOldStatsPolygonColor;
	
	[Attribute(defvalue: "0.761 0.484 0.199 1", desc: "Outline Graph Color for OLD stats")]
	protected ref Color m_iOldStatsOutlineColor;
	
	[Attribute("0.03", "auto", "Value to display when progress is 0")]
	protected float m_fMinRepresentativeValue;
	
	[Attribute(params: "Legend layout")]
	protected ResourceName m_LegendLayout;
	
	//------------------------------------------------------------------------------------------------
	protected override void HandlerAttached(Widget w)
	{
		m_wCanvasWidget = CanvasWidget.Cast(w.FindAnyWidget("Canvas"));		
		m_wbackgroundImage = ImageWidget.Cast(w.FindAnyWidget("Background"));		
		m_wSpiderNetFrame = w.FindAnyWidget("SpiderNetFrame");
	}
	
	void RegisterCareerProfileHandler(SCR_CareerSpecializationsUI instance)
	{
		m_CareerSpecializationsHandler = instance;
	}
	
	void SetSpPoints(array<float> specializations, bool newStats = true)
	{
		if (newStats)
		{
			m_aSpPoints = {};
		
			for (int i = 0, count = specializations.Count(); i < count; i++)
				m_aSpPoints.Insert(specializations[i] / 1000000);
			
			return;
		}
		
		m_aOldSpPoints = {};
		
		for (int i = 0, count = specializations.Count(); i < count; i++)
			m_aOldSpPoints.Insert(specializations[i] / 1000000);
	}
	
	//------------------------------------------------------------------------------------------------
	void DrawSpiderNet()
	{
		if (!m_aSpPoints || m_aSpPoints.IsEmpty() || !m_wbackgroundImage || !m_wCanvasWidget)
			return;
		
		float x, y;
		m_wbackgroundImage.GetScreenSize(x, y);
		if (x <= 0 && y <=  0) //If the layout and widgets have not been initialized yet, do nothing
		{
			GetGame().GetCallqueue().CallLater(DrawSpiderNet,10);
			return;
		}
		
		m_fRadius = x/2;
		m_vCenter = {};
		m_vCenter[0] = x/2; //center: x
		m_vCenter[1] = y/2; //center: y
		
		CalculateVertices(m_aSpPoints, m_aOldSpPoints);
		DrawVertices();
		DrawLegends();
		if (m_CareerSpecializationsHandler)
			m_CareerSpecializationsHandler.UpdateSpecialization(0);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(Widget w)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! To navigate a CanvasWidget we do minus operation to move upwards on the vertical axis
	//! Because the CanvasWidget has the 0,0 coordinate at the top-left corner of the widget
	//! 
	//! That's why to calculate position we do x=center+h*cos(angle), y=center-h*sin(angle). Because the sin is inverted.
	//! So we need to invert the sum operation when working with sin if we want to invert the vertical direction
	//! and move upwards by decreasing on the y
	protected void CalculateVertices(array<float> points, array<float> oldPoints)
	{
		m_aVertices = {};
		m_aOldVertices = {};
		m_aLegendPositions = {};
		m_iNumberOfPoints = points.Count();
		
		float angle, lengthToPoint, lengthToOldPoint, lengthToLegend;
		
		for (int i=0; i<m_iNumberOfPoints;i++)
		{
			//Distribute the 2*PI angles equidistantly for all specializations
			angle = ( ( (2 * Math.PI) / m_iNumberOfPoints) *i) + (Math.PI / 2);
			
			//m_iMinRepresentativeValue of offset so 0 doesn't look like nothing at all
			if (points[i] < m_fMinRepresentativeValue)
				points[i] = m_fMinRepresentativeValue;
			
			if (oldPoints && oldPoints[i] < m_fMinRepresentativeValue)
				oldPoints[i] = m_fMinRepresentativeValue;
			
			//Length of the line = % of points on this Specialization.
			lengthToPoint = m_fRadius * points[i];
			
			if (oldPoints)
				lengthToOldPoint = m_fRadius * oldPoints[i];
			
			//Distance to Legend from Center
			lengthToLegend = lengthToPoint + m_fRadius * m_fLegendOffsetAsRadius;
			
			//Vertices so we know where to draw.
			m_aVertices.Insert(m_vCenter[0] + lengthToPoint * Math.Cos(angle)); //x
			m_aVertices.Insert(m_vCenter[1] - lengthToPoint * Math.Sin(angle)); //y
			
			if (oldPoints)
			{
				m_aOldVertices.Insert(m_vCenter[0] + lengthToOldPoint * Math.Cos(angle)); //x
				m_aOldVertices.Insert(m_vCenter[1] - lengthToOldPoint * Math.Sin(angle)); //y
			}
			
			//Legend position so we know where to put the legend
			m_aLegendPositions.Insert(m_vCenter[0] + lengthToLegend * Math.Cos(angle)); //x
			m_aLegendPositions.Insert(m_vCenter[1] - lengthToLegend * Math.Sin(angle)); //y
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create inner polygon
	protected void DrawVertices()
	{
		m_aCanvasCommands = {};
		
		m_aCanvasCommands.Insert(InnerArea());
		m_aCanvasCommands.Insert(OuterArea());
		m_aCanvasCommands.Insert(OldOuterArea());
		m_aCanvasCommands.Insert(OldInnerArea());
		
		m_wCanvasWidget.SetDrawCommands(m_aCanvasCommands);
	}
	
	//------------------------------------------------------------------------------------------------
	protected CanvasWidgetCommand InnerArea()
	{
		PolygonDrawCommand polygon = new PolygonDrawCommand();
		
		polygon.m_iColor = m_iPolygonColor.PackToInt();
		polygon.m_Vertices = m_aVertices;
		
		return polygon;
	}
	
	//------------------------------------------------------------------------------------------------
	protected CanvasWidgetCommand OldInnerArea()
	{
		PolygonDrawCommand polygon = new PolygonDrawCommand();
		
		polygon.m_iColor = m_iOldStatsPolygonColor.PackToInt();
		polygon.m_Vertices = m_aOldVertices;
		
		return polygon;
	}
	
	//------------------------------------------------------------------------------------------------
	protected CanvasWidgetCommand OuterArea()
	{
		LineDrawCommand surface = new LineDrawCommand();
		
		surface.m_Vertices = m_aVertices;
		surface.m_fWidth = 4;
		surface.m_fOutlineWidth = 6;
		surface.m_iOutlineColor = m_iOutlineColor.PackToInt();
		surface.m_bShouldEnclose = true;
		
		return surface;
	}
	
	//------------------------------------------------------------------------------------------------
	protected CanvasWidgetCommand OldOuterArea()
	{
		LineDrawCommand surface = new LineDrawCommand();
		
		surface.m_Vertices = m_aOldVertices;
		surface.m_fWidth = 4;
		surface.m_fOutlineWidth = 6;
		surface.m_iOutlineColor = m_iOldStatsOutlineColor.PackToInt();
		surface.m_bShouldEnclose = true;
		
		return surface;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DrawLegends()
	{
		m_aLegends = {};
		
		string name;
		
		for (int i = 0; i < m_iNumberOfPoints; i++)
		{
			switch (i)
			{
				case 0: name = "1"; break;
				case 1: name = "2"; break;
				case 2: name = "3"; break;
				default: name = "Undefined ID"; break;
			}
			
			CreateLegend(m_aLegendPositions[i * 2], m_aLegendPositions[i * 2 + 1], name, i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateLegend(float positionX, float positionY, string title, int id)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;
		
		Widget legendWidget = workspace.CreateWidgets(m_LegendLayout, m_wSpiderNetFrame);
		if (!legendWidget)
			return;
		
		ButtonWidget buttonWidget = ButtonWidget.Cast(legendWidget.FindAnyWidget("LegendButton"));
		if (!buttonWidget)
			return;
		
		SCR_ButtonLegendComponent handler = SCR_ButtonLegendComponent.Cast(buttonWidget.FindHandler(SCR_ButtonLegendComponent));
		if (!handler)
			return;
		
		handler.Hide();
		
		handler.GetOnClicked().Insert(OnButtonClick);
		handler.GetOnMouseEnter().Insert(OnMouseEnterButton);
		handler.GetOnMouseLeave().Insert(OnMouseLeaveButton);
		handler.SetButtonId(id);
		handler.SetText(title);
		
		if (m_CareerSpecializationsHandler)
			m_CareerSpecializationsHandler.AddLegendButtonHandler(handler);
		
		AdjustLegend(legendWidget, handler, positionX, positionY);
		m_aLegends.Insert(legendWidget);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AdjustLegend(Widget legendWidget, SCR_ButtonLegendComponent handler, float positionX, float positionY)
	{
		ImageWidget image = handler.GetCoreImageWidget();
		if (!image)
			return;
		
		float x, y;
		image.GetScreenSize(x, y);
		if (x == 0 && y == 0)
		{
			GetGame().GetCallqueue().CallLater(AdjustLegend, 10, false, legendWidget, handler, positionX, positionY);
			return;
		}
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;
		
		positionX = workspace.DPIUnscale(positionX - x/2);
		positionY = workspace.DPIUnscale(positionY - y/2);
		
		FrameSlot.SetPos(legendWidget, positionX, positionY);
		
		handler.Unhide();
		
		//if (handler.GetButtonId() == 0)
		//	handler.SetFocus();
	}
	
	//When a legend is selected, deselect other legends
	//------------------------------------------------------------------------------------------------
	void OnButtonClick(SCR_ButtonLegendComponent buttonHandler)
	{	
		if (!buttonHandler || !m_CareerSpecializationsHandler)
			return;
		
		int newSelection = buttonHandler.GetButtonId();
		m_CareerSpecializationsHandler.UpdateSpecialization(newSelection);
	}
	
	//When a legend is on hover
	//------------------------------------------------------------------------------------------------
	void OnMouseEnterButton(SCR_ButtonLegendComponent buttonHandler)
	{	
		if (!buttonHandler || !m_CareerSpecializationsHandler)
			return;
		
		int hoveredButton = buttonHandler.GetButtonId();
		m_CareerSpecializationsHandler.UpdateHoveredSpecialization(hoveredButton, true);
	}
	
	//When a legend is not hover anymore
	//------------------------------------------------------------------------------------------------
	void OnMouseLeaveButton(SCR_ButtonLegendComponent buttonHandler)
	{	
		if (!buttonHandler || !m_CareerSpecializationsHandler)
			return;
		
		int hoveredButton = buttonHandler.GetButtonId();
		m_CareerSpecializationsHandler.UpdateHoveredSpecialization(hoveredButton, false);
	}
	
	/*
	//------------------------------------------------------------------------------------------------
	protected void CreateLegend(float positionX, float positionY, float horizontalAlignment, float verticalAlignment, string title, int id)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;
		
		Widget legendWidget = Widget.Cast(workspace.CreateWidgets(m_LegendLayout, m_wSpiderNetFrame));
		if (!legendWidget)
			return;
		
		ButtonWidget buttonWidget = ButtonWidget.Cast(legendWidget.FindAnyWidget("LegendButton"));
		if (!buttonWidget)
			return;
		
		SCR_ButtonLegendComponent handler = SCR_ButtonLegendComponent.Cast(buttonWidget.FindHandler(SCR_ButtonLegendComponent));
		if (!handler)
			return;
		
		handler.GetOnClicked().Insert(OnButtonClick);
		handler.SetButtonId(id);
		handler.SetText(title);
		
		if (m_CareerSpecializationsHandler)
			m_CareerSpecializationsHandler.AddLegendButtonHandler(handler);
		
		positionX = m_workspace.DPIUnscale(positionX);
		positionY = m_workspace.DPIUnscale(positionY);
		
		FrameSlot.SetPos(legendWidget, positionX, positionY);
		
		TextWidget wText = handler.GetTextWidget();
		ImageWidget wImage = handler.GetImageWidget();
		
		if ((wText && wImage))
		{
			FrameSlot.SetAlignment(legendWidget, horizontalAlignment, verticalAlignment);
			if (horizontalAlignment < 0.6)
			{
				//Image left, Text right
				AlignableSlot.SetHorizontalAlign(wImage, LayoutHorizontalAlign.Left);
				wText.SetFlags(WidgetFlags.RALIGN);
			}
			else
			{
				//Image right, Text left which's set as default on editor rn
				AlignableSlot.SetHorizontalAlign(wImage, LayoutHorizontalAlign.Right);
			}
		}
		
		m_aLegends.Insert(legendWidget);
	}
	*/
};