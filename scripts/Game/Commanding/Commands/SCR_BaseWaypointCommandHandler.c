[BaseContainerProps()]
class SCR_BaseWaypointCommandHandler
{
	//------------------------------------------------------------------------------------------------
	//! Method called when waypoint is created but havent been yet assigned to any group
	//! \param[in] waypoint
	void OnWaypointCreated(notnull SCR_AIWaypoint waypoint);

	//------------------------------------------------------------------------------------------------
	//! Method called when waypoint is assigned to the group
	//! \param[in] waypoint
	//! \param[in] group
	void OnWaypointIssued(notnull SCR_AIWaypoint waypoint, notnull SCR_AIGroup group);
}
