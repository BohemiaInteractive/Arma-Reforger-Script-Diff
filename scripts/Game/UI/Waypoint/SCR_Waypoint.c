//------------------------------------------------------------------------------------------------
class SCR_Waypoint : Managed
{
	protected static string WIDGET_NAME_DISTANCE = "Distance";
	protected static string WIDGET_NAME_TITLE = "Title";
	protected static string WIDGET_NAME_ICON = "Icon_Extra";
	
	//TODO> MOVE TO DISPLAY
	protected string m_sImageSetResourceName = "{6EAF838A9284890D}UI/Imagesets/Tutorial-Waypoints/Tutorial-Waypoints.imageset";
	
	protected IEntity m_AttachedTo;
	protected Widget m_wWaypointWidget;
	protected vector m_vWaypointPos;
	
	int m_iMinimumDrawDistance;
	int m_iMaximumDrawDistance = -1;
	
	protected vector m_vOffset = vector.Zero;
	RichTextWidget m_wTitle, m_wDistance;
	ImageWidget m_wIcon;
	
	protected bool m_bEnabled = true;
	protected bool m_bFadingEnabled;
	
	//------------------------------------------------------------------------------------------------
	void ShowDistance(bool show)
	{
		m_wDistance.SetVisible(show);
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableFading(bool enable)
	{
		m_bFadingEnabled = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	bool FadingEnabled()
	{
		return m_bFadingEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		return m_bEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEnabled(bool enable)
	{
		m_bEnabled = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetOffsetVector(vector offset)
	{
		m_vOffset = offset;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get Waypoint Widget
	Widget GetWidget()
	{
		return m_wWaypointWidget;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsVisible()
	{
		if (m_wWaypointWidget)
			return m_wWaypointWidget.IsVisible();
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVisible(bool visible)
	{
		m_wWaypointWidget.SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetColor(notnull Color color)
	{
		if (m_wTitle)
			m_wTitle.SetColor(color);
		
		if (m_wDistance)
			m_wDistance.SetColor(color);
		
		if (m_wIcon)
			m_wIcon.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIconImage(string imageName, bool visible, int index = 0)
	{
		if (!m_wIcon)
			return;
		
		m_wIcon.LoadImageFromSet(index, m_sImageSetResourceName, imageName);
		m_wIcon.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetAttachedEntity()
	{
		return m_AttachedTo;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Attach waypoint to entity, copying its position
	void AttachToEntity(IEntity entity)
	{
		m_AttachedTo = entity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set only static position, not attached to entity
	void SetPositionStatic(vector pos)
	{
		m_vWaypointPos = pos;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get waypoint position. Attached entity is prioritized over static position
	vector GetPosition()
	{
		if (m_AttachedTo)
			return m_AttachedTo.GetOrigin() + m_vOffset;
		
		return m_vWaypointPos + m_vOffset;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_Waypoint(ResourceName resourceName, Widget rootW)
	{
		m_wWaypointWidget = GetGame().GetWorkspace().CreateWidgets(resourceName, rootW);
		
		m_wTitle = RichTextWidget.Cast(m_wWaypointWidget.FindAnyWidget(WIDGET_NAME_TITLE));
		m_wDistance = RichTextWidget.Cast(m_wWaypointWidget.FindAnyWidget(WIDGET_NAME_DISTANCE));
		m_wIcon = ImageWidget.Cast(m_wWaypointWidget.FindAnyWidget(WIDGET_NAME_ICON));
		
		SetColor(Color.Yellow);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Waypoint()
	{
		if (m_wWaypointWidget)
			m_wWaypointWidget.RemoveFromHierarchy();
	}
}