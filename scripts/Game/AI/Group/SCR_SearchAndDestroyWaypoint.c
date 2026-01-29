class SCR_SearchAndDestroyWaypointClass : SCR_TimedWaypointClass
{
};


class SCR_SearchAndDestroyWaypoint : SCR_TimedWaypoint
{	
	//------------------------------------------------------------------------------------------------
	void SCR_SearchAndDestroyWaypoint(IEntitySource src, IEntity parent)
	{
		if (SCR_Global.IsEditMode())
			return;
	}
};
