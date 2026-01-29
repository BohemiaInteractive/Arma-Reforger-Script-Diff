/*!
This class represents group's processing of a waypoint.
It is instantiated when a new waypoint is selected by group, and deleted when group switches to another waypoint.
*/

class SCR_AIWaypointState : Managed
{
	protected SCR_AIGroupUtilityComponent m_Utility;
	protected AIWaypoint m_Waypoint;
	
	void SCR_AIWaypointState(notnull SCR_AIGroupUtilityComponent utility, SCR_AIWaypoint waypoint)
	{
		m_Utility = utility;
		m_Waypoint = waypoint;
	}
	
	
	//! Called when group selects the waypoint as current.
	//! Can be overridden in inherited class. Don't forget to call OnSelected of base class.
	void OnSelected();
	
	//! Called when group deselects the waypoint as current, also including completion or removal.
	//! Can be overridden in inherited class. Don't forget to call OnDeselected of base class.
	void OnDeselected()
	{
		m_Utility.CancelActivitiesRelatedToWaypoint(m_Waypoint, doNotCompleteWaypoint: true);
	}
	
	//! Called when group switches from Moving towards waypoint to behavior ON waypoint
	//! Default behavior is to cancel all activities of this waypoint so far, override if not your case
	void OnExecuteWaypointTree()
	{
		m_Utility.CancelActivitiesRelatedToWaypoint(m_Waypoint, SCR_AIMoveActivity);
	}
	
	//! Getter for the related waypoint
	AIWaypoint GetWaypoint()
	{
		return m_Waypoint;
	}
}