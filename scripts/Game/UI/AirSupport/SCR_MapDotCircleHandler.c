class SCR_MapDotCircleHandler : SCR_MapMarkerDynamicWComponent
{
	[Attribute(defvalue: "", desc: "Dot layout resource", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "layout")]
	protected ResourceName m_sDotResource;
	
	protected float m_fActualRadius;
	protected Widget m_wFrame;
	protected SCR_MapEntity m_MapEnt;
	protected ref array<Widget> m_aDots = {};
	protected SCR_MapMarkerDotCircle m_DotMarkerEnt;
	
	protected float m_fEnlargement;
	
	//------------------------------------------------------------------------------------------------
	void OnMapZoom(float ppu)
	{
		if (!m_DotMarkerEnt)
			return;
		
		m_fActualRadius = m_DotMarkerEnt.m_fRadius * ppu;
		
		CreateDots();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetImage(ResourceName icon, string quad, float aspectRatio = 1, int sizeFlag = 64)
	{
		if (!m_wMarkerIcon)
			return;
		
		super.SetImage(icon, quad, aspectRatio, sizeFlag);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] text
	override void SetText(string text)
	{
		if (!m_wMarkerText)
			return;
		
		super.SetText(text);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] marker
	override void SetMarkerEntity(notnull SCR_MapMarkerEntity marker)
	{
		m_DotMarkerEnt = SCR_MapMarkerDotCircle.Cast(marker);
		if (!m_DotMarkerEnt)
			return;
		
		OnMapZoom(m_MapEnt.GetCurrentZoom());
		m_MapEnt.GetOnMapZoom().Insert(OnMapZoom);
		
		CreateDots();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wFrame = w.FindAnyWidget("DotFrame");
		
		m_MapEnt = SCR_MapEntity.GetMapInstance();
		if (!m_MapEnt)
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateDots()
	{
		if (!m_wFrame || !m_DotMarkerEnt)
			return;
		
		foreach (Widget dot : m_aDots)
		{
			dot.RemoveFromHierarchy();
		}
		
		m_aDots.Clear();
		
		int dots = m_DotMarkerEnt.m_fRadius * m_DotMarkerEnt.m_fDotDensity;
		if (dots < 1)
			return;
		
		float angleIncrement = 2 * Math.PI / dots;
		float angle, x, y;
		
		ImageWidget dot;
		for (int i = 0; i < dots; i++)
		{
			angle = i * angleIncrement;
			x = m_fActualRadius * Math.Cos(angle);
			y = m_fActualRadius * Math.Sin(angle);
			
			dot = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidgets(m_sDotResource, m_wFrame));
			
			dot.SetColor(m_DotMarkerEnt.m_DotColor);
			
			FrameSlot.SetPos(dot, x, y);
			
			m_aDots.Insert(dot);
		}
	}
}