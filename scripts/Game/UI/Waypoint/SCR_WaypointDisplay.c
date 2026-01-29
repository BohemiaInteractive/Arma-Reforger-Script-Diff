//------------------------------------------------------------------------------------------------
//! Waypoint UI 
//! Attached to SCR_HUDManagerComponent which is a component of SCR_PlayerController
class SCR_WaypointDisplay : SCR_InfoDisplayExtended
{
	//TODO: ATTRIBUTE
	protected static string DISTANCE_STRING_UNIT = "#AR-Tutorial_WaypointUnits_meters";
	protected static float WAYPOINT_FADE_THRESHOLD = 20;
	protected static float WAYPOINT_MINIMUM_OPACITY = 0.2;
	protected static float WAYPOINT_DISTANCE_INDICATOR_FADE_START = 100;
	protected static float WAYPOINT_DISTANCE_INDICATOR_FADE_END = 50;
	protected static float WAYPOINT_FADE_IN_DISTANCE = 3;
		
	//TODO: ATTRIBUTE
	protected ResourceName m_sWaypointLayout = "{825C6D728AC3E02A}UI/layouts/Waypoint/Waypoint.layout";
	protected ref array<ref SCR_Waypoint> m_aWaypoints;
	
	//------------------------------------------------------------------------------------------------
	SCR_Waypoint FindWaypointByEntity(IEntity entity)
	{
		if (!m_aWaypoints)
			return null;
		
		IEntity attachedEntity;
		foreach(SCR_Waypoint waypoint : m_aWaypoints)
		{
			attachedEntity = waypoint.GetAttachedEntity();
			if (!attachedEntity || (attachedEntity != entity))
				continue;
			
			return waypoint;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void GetWaypoints(out array<ref SCR_Waypoint> waypoints)
	{
		waypoints = m_aWaypoints;
	}
	
	//------------------------------------------------------------------------------------------------
	void DeleteWaypoint(SCR_Waypoint waypoint)
	{
		if (m_aWaypoints)
			m_aWaypoints.RemoveItem(waypoint);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_Waypoint CreateWaypoint(vector position)
	{
		SCR_Waypoint waypoint = new SCR_Waypoint(m_sWaypointLayout, m_wRoot);
		if (!waypoint)
			return null;
		
		waypoint.SetPositionStatic(position);
		
		if (!m_aWaypoints)
			m_aWaypoints = {};
		
		m_aWaypoints.Insert(waypoint);
		
		return waypoint;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_Waypoint CreateWaypoint(IEntity entity)
	{
		SCR_Waypoint waypoint = new SCR_Waypoint(m_sWaypointLayout, m_wRoot);
		if (!waypoint)
			return null;
		
		waypoint.AttachToEntity(entity);
		
		if (!m_aWaypoints)
			m_aWaypoints = {};
		
		m_aWaypoints.Insert(waypoint);
		
		return waypoint;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool HandleWidgetOpacities(notnull SCR_Waypoint waypoint, notnull IEntity player)
	{
		float distance = vector.Distance(player.GetOrigin(), waypoint.GetPosition());

		if ((waypoint.m_iMinimumDrawDistance > distance) || ((waypoint.m_iMaximumDrawDistance != -1) && (waypoint.m_iMaximumDrawDistance < distance)))
		{
			waypoint.SetVisible(false);
			return false;
		}
		else
		{
			waypoint.SetVisible(true);
		}
		
		RichTextWidget distanceWidget = waypoint.m_wDistance;
		int shownDistance;
		float distanceOpacity;

		if (distanceWidget)
		{	
			// Rounding
			int roundedDistance = Math.Round(distance);
			
			if (distance > 1000)
				shownDistance = roundedDistance - (roundedDistance % 1000);
			else if (distance > 100)
				shownDistance = Math.Round(distance / 100) * 100;
			else if (distance > 50)
				shownDistance = roundedDistance - (roundedDistance % 50);
			
			distanceWidget.SetTextFormat(DISTANCE_STRING_UNIT, shownDistance);
			
			//Distance opacity
			if (distance >= WAYPOINT_DISTANCE_INDICATOR_FADE_START)
				distanceOpacity = 1;
			else if (distance <= WAYPOINT_DISTANCE_INDICATOR_FADE_END)
				distanceOpacity = 0;
			else
				distanceOpacity = Math.InverseLerp(WAYPOINT_DISTANCE_INDICATOR_FADE_END, WAYPOINT_DISTANCE_INDICATOR_FADE_START, distance);
			
			distanceWidget.SetOpacity(distanceOpacity);
		}
		
		Widget iconWidget = waypoint.m_wIcon;
		float iconsOpacity;
		
		if (iconWidget)
		{
			//Icon Opacity
			if (waypoint.FadingEnabled())
			{
				if (waypoint.m_iMaximumDrawDistance != -1 && distance > (waypoint.m_iMaximumDrawDistance - WAYPOINT_FADE_IN_DISTANCE))	// Player is reaching the max drawing distance
					iconsOpacity = Math.Lerp(0, 1, (waypoint.m_iMaximumDrawDistance - distance) / WAYPOINT_FADE_IN_DISTANCE);
				else if (waypoint.m_iMinimumDrawDistance != 0 && distance < (waypoint.m_iMinimumDrawDistance + WAYPOINT_FADE_IN_DISTANCE))	// Player is reaching the min drawing distance
					iconsOpacity = Math.Lerp(1, 0, (distance - waypoint.m_iMinimumDrawDistance) / WAYPOINT_FADE_IN_DISTANCE);
				else if (distance < WAYPOINT_FADE_THRESHOLD)	// Player is within fade distance
					iconsOpacity = Math.Lerp(WAYPOINT_MINIMUM_OPACITY, 1, distance / WAYPOINT_FADE_THRESHOLD);
				else
					iconsOpacity = 1;
			}
			else
			{
				iconsOpacity = 1;
			}
			
			if (iconWidget)
				iconWidget.SetOpacity(iconsOpacity);
			
			if (distanceWidget && distanceOpacity > iconsOpacity)
				distanceWidget.SetOpacity(iconsOpacity);
		}
		
		RichTextWidget titleWidget = waypoint.m_wTitle;
		
		if (titleWidget)
			titleWidget.SetOpacity(iconsOpacity);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	// Overrides
	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		if (!m_wRoot || !m_aWaypoints || m_aWaypoints.IsEmpty())
			return;
		
		IEntity player = m_PlayerController.GetControlledEntity();
		if (!player)
			return;
		
		WorkspaceWidget workspace = m_wRoot.GetWorkspace();
		if (!workspace)
			return;
		
		int screenHeight = workspace.GetHeight();
		int screenWidth = workspace.GetWidth();
		int widgetPosX, widgetPosY;
		
		BaseWorld world = GetGame().GetWorld();
		vector position2D;

		foreach (SCR_Waypoint waypoint : m_aWaypoints)
		{
			//Don't update waypoints, that are not enabled
			if (!waypoint.IsEnabled())
				continue;
			
			if (!HandleWidgetOpacities(waypoint, player))
				continue;
			
			//positioning widget
			position2D = workspace.ProjWorldToScreen(waypoint.GetPosition(), world);
			widgetPosX = workspace.DPIScale(position2D[0]);
			widgetPosY = workspace.DPIScale(position2D[1]);
			
			if (widgetPosX < 0)
				position2D[0] = 0;
			else if (widgetPosX > screenWidth)
				position2D[0] = workspace.DPIUnscale(screenWidth);
			
			if (widgetPosY < 0)
				position2D[1] = 0;
			else if (widgetPosY > screenHeight || position2D[2] < 0)
				position2D[1] = workspace.DPIUnscale(screenHeight);
			
			FrameSlot.SetPos(waypoint.GetWidget(), position2D[0], position2D[1]);
		}
	}
}